/* gcks.cxx */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#include <libmikey/Mikey.h>
#include <libmikey/KeyAgreementPSK.h>
#include <bits/basic_string.h>
#include "MikeyTestConfig.h"

extern "C" {
#include "krb5.h"
#include "com_err.h"
}

typedef struct {
   char* client_name;
   char* client_addr;
   char* group_name;
   int client_port;
} client_conn_info_t;

#define PRINT_TIME(id) \
        print_time(id, __LINE__)
timespec prev_ts;

timespec 
diff(timespec start, timespec end)
{
   timespec temp;
   if ((end.tv_nsec < start.tv_nsec) ) { 
      temp.tv_sec = end.tv_sec-start.tv_sec-1;
      temp.tv_nsec = LONG_MAX+ (end.tv_nsec-start.tv_nsec);
   }   
   else{
      temp.tv_sec = end.tv_sec-start.tv_sec;
      temp.tv_nsec = end.tv_nsec-start.tv_nsec;
   }   
   return temp;
}

void print_time( clockid_t id, int line_num) {
    timespec tmp;

    if(clock_gettime(id, &tmp) < 0) {
      perror("clock_gettime()");
      return;
    }   

    tmp = diff(prev_ts, tmp);
    printf("PRINT_TIME(%d): %ds\t%4.2fms\n",  
            line_num, tmp.tv_sec, ((float)tmp.tv_nsec)/1000000);

    if(clock_gettime(id, &prev_ts) < 0) {
      perror("clock_gettime()");
      return;
    }   
}
void parse_conn_info(char* connInfo, client_conn_info_t* conn_info)// char** cname, char** caddr,
                           // int* port, char** sname) 
{
    char* pch;

    pch = strtok (connInfo,"|");
    conn_info->client_name = (char*)malloc(strlen(pch)+1);
    memset(conn_info->client_name, 0, strlen(pch)+1);
    strncpy(conn_info->client_name, pch, strlen(pch));
    //*cname = (char*)malloc(strlen(pch));
    //strncpy(*cname, pch, strlen(pch));

    pch = strtok (NULL, "|");
    conn_info->client_addr = (char*)malloc(strlen(pch)+1);
    memset(conn_info->client_addr, 0, strlen(pch)+1);
    strncpy(conn_info->client_addr, pch, strlen(pch));
    //*caddr = (char*)malloc(strlen(pch));
    //strncpy(*caddr, pch, strlen(pch));

    pch = strtok (NULL, "|");
    conn_info->client_port = atoi(pch);
    //*port = atoi(pch);

    pch = strtok (NULL, "|");
    conn_info->group_name = (char*)malloc(strlen(pch)+1);
    memset(conn_info->group_name, 0, strlen(pch)+1);
    strncpy(conn_info->group_name, pch, strlen(pch));
    //*sname = (char*)malloc(strlen(pch));
    //strncpy(*sname, pch, strlen(pch));


    printf("cname:[%s] caddr: [%s] port[%d] sname: [%s]\n",
             conn_info->client_name,
             conn_info->client_addr, 
             conn_info->client_port, 
             conn_info->group_name);
    //printf("cname:[%s] caddr: [%s] port[%d] sname: [%s]\n",
    //         *cname, *caddr, *port, *sname);
}

/*
 * Retrieves the K_service(gcks) from the local keytab file
 */
krb5_error_code obtain_service_key(krb5_context context,
                                    char* keytab_filename,
                                    const char* realm,
                                    const char* gcks_sname,
                                    krb5_keyblock** gcks_key)
{
    krb5_error_code retval;
    krb5_principal service;
    
    retval = krb5_sname_to_principal(context, realm, gcks_sname,
                                     KRB5_NT_SRV_HST, &service);
    if (retval) {
        com_err("GCKS", retval, "while getting service principal");
        return retval;
    }
    
    retval = krb5_kt_read_service_key(context, keytab_filename, service, 0, 0,
            gcks_key);
    if (retval) {
        com_err("GCKS", retval, "while getting service key from keytab");
        return retval;
    }
    return 0;
}

/*
 * Decrypts the mikey message to retrieve the TGK (session key)
 */
void process_mikey_message( /*IN*/
                            char *keyMgmtMessage,
                            char* peerUri,
                            krb5_keyblock* gcks_key,
                            /*OUT*/
                            krb5_keyblock* session_key,
                            client_conn_info_t* conn_info
                            /*char** cname,
                            char** caddr,
                            int* port,
                            char** sname*/)
{
printf("gcks_key %s, length %d\n",gcks_key->contents, gcks_key->length); 
    std::string psk((char*)gcks_key->contents,gcks_key->length);

    MikeyTestConfig *config = new MikeyTestConfig();
    peerUri = "KDC";
    config->psk = psk;

    // FIXME free config
    MRef<Mikey*> mikey = new Mikey( config );

    // We try to authenticate the initiator message back from KDC (TGS)
    if( !mikey->responderAuthenticate( keyMgmtMessage, peerUri ) ){
        std::cout<<"Incoming key management message could not be authenticated"<<std::endl;
        std::cout<<mikey->authError();
    }

    // Sets the initiator's message data into Mikey class
    // this includes setting TGK in the keyAgreement
    mikey->setMikeyOffer();

    // The decrypted TGK here is set as the K_client_service session key!
    unsigned int tgkLength = mikey->getTgkLength();
    session_key->contents = (unsigned char*)malloc(tgkLength);
    memcpy(session_key->contents, mikey->getTgk(), tgkLength);
    session_key->length = tgkLength;

    KeyAgreement* kaBase = *(mikey->getKeyAgreement());
    MRef<KeyAgreementPSK*> ka = dynamic_cast<KeyAgreementPSK*>(kaBase);

    // Parse the extracted client's principal name and address from
    // MikeyMessage's GeneralExtension payload
    parse_conn_info(ka->connInfo, conn_info); //cname, caddr, port, sname);
    //delete mikey;
    //delete config;
}

void send_mikey_message(char* client_name, char* c_addr, int client_port,
                        krb5_keyblock* session_key){
printf("gcks_key %s, length %d\n",session_key->contents, session_key->length); 
    int type = KEY_AGREEMENT_TYPE_PSK;
    std::string psk((char*)session_key->contents, session_key->length);
    std::string keyMgmtMessage;
    char tmpBuf[BUFSIZ];
    char peerUri[2048];
    char client_addr[30];

    strncpy(peerUri, client_name, 2048);
    strncpy(client_addr, c_addr, 30);

    MikeyTestConfig *config = new MikeyTestConfig();
    config->psk = psk; //Pre-Shared key is K_client_service(group) from Krb5

    // FIXME free config
    MRef<Mikey*> mikey = new Mikey( config );
    mikey->addSender(rand());
    
    // We construct the mikey message
    keyMgmtMessage = mikey->initiatorCreate( type, peerUri );
    strncpy(tmpBuf, keyMgmtMessage.c_str(),BUFSIZ);
    fflush(stdout);

    // Now we attempt to send the message to the client
    struct hostent *host;
    struct sockaddr_in client_saddr;
    int sock;

    printf("GCKS to client MikeyMsg:\n%s\n", keyMgmtMessage.c_str());
    host = gethostbyname(client_addr);
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Failed to open socket");
        return;
    }
    client_saddr.sin_family = AF_INET;
    client_saddr.sin_port = htons(9999);
    client_saddr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
    bzero(&(client_saddr.sin_zero),8);

    while ( connect(sock, (struct sockaddr *)&client_saddr,
                sizeof(struct sockaddr)) == -1 )
    {
        //close(sock); 
//printf("%s(%d)\n",__FILE__,__LINE__);
        usleep(500);
        //  FIXME Race condition: sclient2 may not be ready to receive
        //  our mikey message. Short term fix is to make counter
        //  for retries per second
        //perror("Failed to connect to client");
        //return;
    }

    send(sock,keyMgmtMessage.c_str(), strlen(keyMgmtMessage.c_str()), 0);
    //delete mikey;
    //delete config;

    close(sock);
    printf("MikeyMessage sent to client\n");
}

int main(int argc, char *argv[]) {
    int sock, kdc_sock, bytes_received, void_ptr;
    char recv_data[BUFSIZ];
    struct sockaddr_in gcks_addr;
    int sin_size;
    krb5_context context;
    krb5_error_code retval;
    krb5_keytab keytab = NULL; // Default keytab
    krb5_keyblock* gcks_key = NULL;
    char* keytab_filename = NULL;
    char* gcks_sname = "gcks1";
    char* gcks_realm = "LOCALHOST";

  struct timespec start_ts; 
    clockid_t id = CLOCK_REALTIME; 
     
    if(clock_getres(id, &start_ts) < 0) { 
      perror("clock_getres()"); 
      return 0; 
    } 
    if(clock_gettime(id, &prev_ts) < 0) { 
      perror("clock_gettime()");
      return 0;
    }
    retval = krb5_init_context(&context);
    if (retval) {
        com_err("GCKS", retval, "while initializing krb5");
        exit(1);
    }

    if( argc > 1) {
        gcks_sname = argv[1];
    }
    else {
        printf("Usage: %s <gcks_service_name> <keytab file>\n",argv[0]);
    }

    if( argc > 2 ){
        keytab_filename = argv[2];
        // Specified keytab, load reference to keytab first
        if ((retval = krb5_kt_resolve(context, keytab_filename, &keytab))) 
        {
            com_err("GCKS", retval,
                    "while resolving keytab file %s", keytab_filename);
            exit(1);
        }
    }
    
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Socket");
        exit(1);
    }

    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,
               &void_ptr,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }
    struct linger linger;
    linger.l_onoff = 1;
    linger.l_linger = 1;

    if (setsockopt(sock,SOL_SOCKET,SO_LINGER,
               &linger,sizeof(linger)) == -1) {
        perror("Setsockopt");
        exit(1);
    }
    gcks_addr.sin_family = AF_INET;
    gcks_addr.sin_port = htons(KDC_GCKS_PORT);
    gcks_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    bzero(&(gcks_addr.sin_zero),8);

    // Obtain the GCKS key to decrypt/authenticate the MikeyMessage
    obtain_service_key(context, keytab_filename, gcks_realm, gcks_sname,
                            &gcks_key);

    if (bind(sock, (struct sockaddr *)&gcks_addr,
                      sizeof(struct sockaddr))  == -1) 
    {
        perror("Unable to bind");
        exit(1);
    }

    if (listen(sock, 5) == -1) {
        perror("Listen");
        exit(1);
    }

    sin_size = sizeof(struct sockaddr_in);
    // This should be set by reading mikey payload
    //char* client_name = NULL;// = "jeffrey/admin@LOCALHOST";
    //char* client_addr = NULL;// = "127.0.0.1"
    //char* group_name = NULL;
    //int client_port;
    int service_count = 0;
    while (1)//service_count++ < 50)
    {
        printf("\nGCKS waiting for KDC on port %d", KDC_GCKS_PORT);
        fflush(stdout);

        kdc_sock = accept(sock, (struct sockaddr *)&gcks_addr,(socklen_t*)&sin_size);

    if(clock_gettime(id, &prev_ts) < 0) { 
      perror("clock_gettime()");
      return 0;
    }
        printf("\n I got a connection from (%s , %d)",
              inet_ntoa(gcks_addr.sin_addr),htons(gcks_addr.sin_port));

        char* peerUri = "KDC";
        krb5_keyblock session_key;

        client_conn_info_t conn_info;
        conn_info.client_name = NULL;      
        conn_info.client_addr = NULL;
        conn_info.group_name = NULL;
        conn_info.client_port = 0;

        memset(recv_data,0, BUFSIZ);
        
        printf("\n GCKS waiting for data\n");
        bytes_received = recv(kdc_sock,recv_data,BUFSIZ-1,0);
        printf("\n GCKS RECIEVED %d Bytes\n" , bytes_received);
        recv_data[bytes_received] = '\0';
        //printf("\n GCKS RECIEVED DATA = \n%s\n<END>" , recv_data);
        fflush(stdout);

        close(kdc_sock);
PRINT_TIME(id);
        
        process_mikey_message(  /*IN*/
                                recv_data,
                                peerUri,
                                gcks_key,
                                /*OUT*/
                                &session_key,
                                &conn_info       
                                /*&client_name,
                                &client_addr,
                                &client_port,
                                &group_name*/);
PRINT_TIME(id);
        
        // Now we should save this Group-Client(name& address) pair matchup
        // for the following purposes:
        //
        //  1) Rekey process
        //     -all the clients within the group will get a rekey update
        //      using all individual K_client_service(group) key
        //  2) Rejoin attempts
        //     -client sends service ticket to verify it's legit rejoin request
        //     -the GCKS needs to group name to retrieve the corresponding
        //      K_service(group) to verify the ticket.
        //
        // The rekey process for Mikey is not yet well defined so we leave
        // this functionality as future work.
        //
        // Note: the data structure should be done in a secure way!
        //
        // assign_client_group( group_name, client_name, client_addr, session_key)

        send_mikey_message( conn_info.client_name, 
                            conn_info.client_addr,
                            conn_info.client_port, 
                            &session_key);
PRINT_TIME(id);
printf("%s(%d)\n",__FILE__,__LINE__);
         printf("DEBUG: %s %s \n",conn_info.client_name, conn_info.client_addr);
        if(conn_info.client_name) free(conn_info.client_name);
        if(conn_info.client_addr) free(conn_info.client_addr);
        if(conn_info.group_name)  free(conn_info.group_name);
        if(session_key.contents) {
            free(session_key.contents); 
            session_key.contents = NULL;
            session_key.length = 0;
        }
    }
    close(sock);
    return 0;
} 
