/* 
 *
 * Sample Kerberos v5 client from MITKerberos modified for research purposes.
 *
 * Usage: sclient hostname
 */

extern "C" {
#include "krb5.h"
#include "com_err.h"
}

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
extern "C" {
#include "fake-addrinfo.h" /* not everyone implements getaddrinfo yet */
}

#include <signal.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>

#include "sample.h"
//#include "windows/identity/kcreddb/credential.h"

#ifndef GETSOCKNAME_ARG3_TYPE
#define GETSOCKNAME_ARG3_TYPE int
#endif

#ifndef DEFAULT_TEST
#define DEFAULT_TEST 0
#endif

#ifndef MIKEY_INTEGRATION_H
#define MIKEY_INTEGRATION_H
#include <libmikey/Mikey.h>
#include <libmikey/KeyAgreement.h>
#include "MikeyTestConfig.h"
#endif

#define PRINT_TIME(id) \
     print_time(id, __LINE__)
    

timespec prev_ts;

char* progname;
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


static int
net_read(int fd, char *buf, int len)
{
    int cc, len2 = 0;

    do {
        cc = SOCKET_READ((SOCKET)fd, buf, len);
        if (cc < 0) {
            if (SOCKET_ERRNO == SOCKET_EINTR)
                continue;

            /* XXX this interface sucks! */
            errno = SOCKET_ERRNO;

            return(cc);          /* errno is already set */
        }
        else if (cc == 0) {
            return(len2);
        } else {
            buf += cc;
            len2 += cc;
            len -= cc;
        }
    } while (len > 0);
    return(len2);
}

krb5_error_code KRB5_CALLCONV
obtain_credentials(krb5_context context, krb5_principal client,
              krb5_principal server, krb5_flags flags, krb5_ccache ccache,
              krb5_error **error,
              krb5_creds **out_creds)
{
    krb5_creds          creds;
    krb5_creds           * credsp = NULL;
    krb5_creds           * credspout = NULL;
    krb5_error_code     retval = 0;

    if (error)
        *error = 0;

    /*
     * Try getting it from the
     * credentials cache.
     */
    memset(&creds, 0, sizeof(krb5_creds));

    if ((retval = krb5_copy_principal(context, server,
                                      &creds.server)))
        goto error_return;
    if (client)
        retval = krb5_copy_principal(context, client,
                                     &creds.client);
    else
        retval = krb5_cc_get_principal(context, ccache,
                                       &creds.client);
    if (retval) {
        krb5_free_principal(context, creds.server);
        goto error_return;
    }
    /* creds.times.endtime = 0; -- memset 0 takes care of this
       zero means "as long as possible" */
    /* creds.keyblock.enctype = 0; -- as well as this.
       zero means no session enctype
       preference */

    if ((retval = krb5_get_credentials(context, flags,
                                       ccache, &creds, &credsp)))
        goto error_return;
    credspout = credsp;

    retval = 0;         /* Normal return */
    if (out_creds) {
        *out_creds = credsp;
        credspout = NULL;
        return(retval);
    }

error_return:
    krb5_free_cred_contents(context, &creds);
    if (credspout != NULL)
        krb5_free_creds(context, credspout);
    return(retval);
}

int
main(int argc, char *argv[])
{
    krb5_context context;
    krb5_error_code retval;
    krb5_ccache ccdef;
    krb5_creds *creds;   // Used to test retrieval of credentials
    krb5_principal client, server;
    krb5_error *err_ret;
    krb5_auth_context auth_context = 0;
    char* cache_name = NULL;
    char *service = SAMPLE_SERVICE;

//------Used for timing performance
    FILE* timeLog = NULL;
    char* timeLog_filename = NULL;
    struct timespec start_ts;
    struct timespec end_ts;
    clockid_t id = CLOCK_REALTIME;
    
    if(clock_getres(id, &start_ts) < 0) {
      perror("clock_getres()");
      return 0;
    }   
    
    if(clock_gettime(id, &start_ts) < 0) {
      perror("clock_gettime()");
      return 0;
    }
    if(clock_gettime(id, &prev_ts) < 0) {
      perror("clock_gettime()");
      return 0;
    }
    
    PRINT_TIME(id);
//-----------------------------------

    if (argc < 2 && argc > 5) {
        fprintf(stderr, 
            "usage: %s <hostname> <service/group> [cache_name] [timing_log_file] \n",argv[0]);
        exit(1);
    }

    retval = krb5_init_context(&context);
    if (retval) {
        com_err(argv[0], retval, "while initializing krb5");
        exit(1);
    }

    (void) signal(SIGPIPE, SIG_IGN);

    if (argc > 2) {
        service = argv[2];
    }
    
    if (argc >3 ) {
        cache_name = argv[3];
    } 

    if (argc >4 ) {
        timeLog_filename = argv[4];
    } 
    PRINT_TIME(id);
    retval = krb5_sname_to_principal(context, argv[1], service,
                                     KRB5_NT_SRV_HST, &server);
    if (retval) {
        com_err(argv[0], retval, "while creating server name for host %s service %s",
                argv[1], service);
        exit(1);
    }
    
    PRINT_TIME(id);
    retval = krb5_cc_resolve(context,cache_name,
          &ccdef);

    if(retval) {
        com_err(argv[0], retval, "while getting ticket ccache");
        exit(1);
    }
/*
    retval = krb5_cc_default(context, &ccdef);
    if (retval) {
        com_err(argv[0], retval, "while getting default ccache");
        exit(1);
    }
*/
    PRINT_TIME(id);
    retval = krb5_cc_get_principal(context, ccdef, &client);
    if (retval) {
        com_err(argv[0], retval, "while getting client cache");
        exit(1);
    }
    krb5_flags req_flags = AP_OPTS_MUTUAL_REQUIRED | KDC_OPT_MIKEY;
    PRINT_TIME(id);

    // This will attempt to retrieve credentials from the cache,
    // If it's not found, it'll use the TGT in the cache to retrieve
    // a new ticket related to the service principal
    retval = obtain_credentials(context, client, server, req_flags,
                           ccdef, &err_ret,
                           &creds);// Test retrieval of creds.

    PRINT_TIME(id);

    // Now we listen for incoming MIKEY message from GCKS
    int sock, connected, bytes_recieved, void_ptr;
    char recv_data[1024];
    struct sockaddr_in gcks_addr;
    int sin_size;
    char* peerUri;

    retval = krb5_unparse_name(context, client, &peerUri);
    if (retval) {
        com_err(argv[0], retval, "while getting client name");
        exit(1);
    }
    
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Socket");
        exit(1);
    }

    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&void_ptr,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }

    gcks_addr.sin_family = AF_INET;
    gcks_addr.sin_port = htons(9999);
    gcks_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(gcks_addr.sin_zero),8);

    if (bind(sock, (struct sockaddr *)&gcks_addr, sizeof(struct sockaddr))
                                                                   == -1) {
        perror("Unable to bind");
        exit(1);
    }

    if (listen(sock, 5) == -1) {
        perror("Listen");
        exit(1);
    }
    PRINT_TIME(id);

    printf("\nClient waiting for GCKS on port 9999\n");
    fflush(stdout);
    
    sin_size = sizeof(struct sockaddr_in);

    fd_set rfds;
    struct timeval t;
    int select_ret_val;
    
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    
    t.tv_sec = 3;
    t.tv_usec = 0;

//    select_ret_val = select(sock+1, &rfds, NULL, NULL, &t);
    PRINT_TIME(id);

    connected = accept(sock, (struct sockaddr *)&gcks_addr, (socklen_t*)&sin_size);
/*
    if(select_ret_val == -1) {  
        printf("Error on select call\n");
    }       
    else if( select_ret_val ){
        if(  (connected = accept(sock, 
                                (struct sockaddr *)&gcks_addr,
                                (socklen_t*)&sin_size)) < -1) 
        {
            close(sock);
            exit(1);
        }
    }
    else {
        // Time out...
     
        long time_elapsed; // ns  
        if(clock_gettime(id, &end_ts) < 0) {
          perror("clock_gettime()");
          return 0;
        }
        struct timespec diff_ts = diff(start_ts, end_ts); 
        time_elapsed = 
             (int)(diff_ts.tv_sec) * 1000000000 +  diff_ts.tv_nsec;
        printf("TIMEOUT on connect\n");
        if( timeLog_filename ) {
           timeLog = fopen(timeLog_filename, "a");
        
           fprintf(timeLog, "%ld\n",
                time_elapsed / 1000000); // in ms
            close(sock);
            exit(0);
        }
    }
   */ 
 /*   while( (connected = accept4(sock, 
                            (struct sockaddr *)&gcks_addr,
                            (socklen_t*)&sin_size,
                             SOCK_NONBLOCK | SOCK_CLOEXEC)) < -1) 
    {
//        if( errno == EAGAIN || errno == EWOULDBLOCK ) {
            long time_elapsed; // ns  
            if(clock_gettime(id, &end_ts) < 0) {
              perror("clock_gettime()");
              return 0;
            }
            struct timespec diff_ts = diff(start_ts, end_ts); 
            time_elapsed = 
                 (int)(diff_ts.tv_sec) * 1000000000 +  diff_ts.tv_nsec;
            if( time_elapsed / 1000000 > 100 ) {
                printf("TIMEOUT on connect\n");
                if( timeLog_filename ) {
                   timeLog = fopen(timeLog_filename, "a");
            
                   fprintf(timeLog, "%ld\n",
                        time_elapsed / 1000000); // in ms
                    close(sock);
                    exit(0);
                }
            }
  //      }
    }
*/  
    PRINT_TIME(id);

    printf("\n Got connection from (%s , %d)\n",
                inet_ntoa(gcks_addr.sin_addr),ntohs(gcks_addr.sin_port));
    bytes_recieved = recv(connected,recv_data,2048,0);
    PRINT_TIME(id);

    // Retrieved mikey message
    recv_data[bytes_recieved] = '\0';
    fflush(stdout);
    close(connected);
    close(sock);

    // Need to obtain session key to decrypt mikey message
    // creds->keyblock is K_client_service(group) session key
    std::string mikeyMessage(recv_data);
if(creds) printf("%s(%d)\n",__FILE__, __LINE__);
if(creds->keyblock.contents) printf("%s(%d): %s\n",
    __FILE__, __LINE__, creds->keyblock.contents);

    std::string psk((char*)creds->keyblock.contents,creds->keyblock.length);
 printf("%s(%d)\n",__FILE__, __LINE__);
    MikeyTestConfig *config = new MikeyTestConfig();
    config->psk = psk;
    
    // FIXME free config
    MRef<Mikey*> mikey = new Mikey( config );
    PRINT_TIME(id);

    std::cout<<peerUri<<std::endl;
    //std::cout<<"Received: "<<mikeyMessage<<std::endl;
    std::cout<<"MIKEY Responder Authenticate:"<<std::endl;
    // We try to authenticate the initiator message back from GCKS
    if( !mikey->responderAuthenticate( mikeyMessage, peerUri ) ){
        std::cout<<"Incoming key management message could not be authenticated"<<std::endl;
        std::cout<<mikey->authError();
    }
    PRINT_TIME(id);

    // Sets the initiator's message data into Mikey class
    // this includes setting TGK in the keyAgreement
    mikey->setMikeyOffer();    

    std::cout<<"Mikey offer set!"<<std::endl;
    fflush(stdout);

    PRINT_TIME(id);
    krb5_free_principal(context, server);       /* finished using it */
    krb5_free_principal(context, client);
    krb5_cc_close(context, ccdef);
    if (auth_context) krb5_auth_con_free(context, auth_context);

    krb5_free_creds(context, creds);
    krb5_free_context(context);

    PRINT_TIME(id);
    long time_elapsed; // ns  
    if(clock_gettime(id, &end_ts) < 0) {
      perror("clock_gettime()");
      return 0;
    }
    else {
      time_elapsed = 
         ((int)(end_ts.tv_sec -start_ts.tv_sec)) * 1000000000 +
       end_ts.tv_nsec - start_ts.tv_nsec;
    }
    if( timeLog_filename ) {
       timeLog = fopen(timeLog_filename, "a");
      struct timespec diff_ts = diff(start_ts, end_ts);

      time_elapsed = 
         (int)(diff_ts.tv_sec) * 1000000000 +  diff_ts.tv_nsec;

       fprintf(timeLog, "%ld\n",time_elapsed / 1000000); // in ms
    }

    PRINT_TIME(id);
    exit(0);
}
