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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
extern "C" {
#include <time.h>
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

    if (client) {
        retval = krb5_copy_principal(context, client,
                                     &creds.client);
// printf("%d \t %s(%d)\n", retval, __FILE__,__LINE__);
    }
    else {
        retval = krb5_cc_get_principal(context, ccache,
                                       &creds.client);
// printf("%d \t %s(%d)\n", retval, __FILE__,__LINE__);
    }

    if (retval) {
//printf("%d \t %s(%d)\n", retval, __FILE__,__LINE__);
        krb5_free_principal(context, creds.server);
        goto error_return;
    }
    /* creds.times.endtime = 0; -- memset 0 takes care of this
       zero means "as long as possible" */
    /* creds.keyblock.enctype = 0; -- as well as this.
       zero means no session enctype
       preference */

    if ((retval = krb5_get_credentials(context, flags,
                                       ccache, &creds, &credsp))) {
//printf("%d \t %s(%d)\n", retval, __FILE__,__LINE__);
        goto error_return;
    }
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
int
main(int argc, char *argv[])
{
    struct addrinfo *ap, aihints, *apstart;
    int aierr;
    int sock;
    krb5_context context;
    krb5_data recv_data;
    krb5_data cksum_data;
    krb5_error_code retval;
    krb5_ccache ccdef;
    krb5_creds *creds;   // Used to test retrieval of credentials
    krb5_principal client, server;
    krb5_error *err_ret;
    krb5_ap_rep_enc_part *rep_ret;
    krb5_auth_context auth_context = 0;
    short xmitlen;
    char *cache_name = NULL;
    char *portstr;
    char *service = SAMPLE_SERVICE;
    char *group_name = "group1";
    char* client_name;
    int i;

//------Used for timing performance
    FILE* timeLog = NULL;
    char* timeLog_filename = NULL;
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
    PRINT_TIME(id);
//-----------------------------------

    if (argc > 7 ) {
        fprintf(stderr, "usage: %s <hostname> [port] [service] [group name] [cache_name] [timing_log_file]\n",argv[0]);
        exit(1);
    }

    retval = krb5_init_context(&context);
    if (retval) {
        com_err(argv[0], retval, "while initializing krb5");
        exit(1);
    }

    (void) signal(SIGPIPE, SIG_IGN);

    if (argc > 2)
        portstr = argv[2];
    else
        portstr = SAMPLE_PORT;

    if (argc > 3) {
        service = argv[3];
    }
    if (argc > 4) {
        group_name = argv[4];
    }
    
    if (argc > 5) {
        cache_name = argv[5];
    }
   
   if (argc > 6 ) {
        timeLog_filename = argv[6];
   }

    PRINT_TIME(id);
    memset(&aihints, 0, sizeof(aihints));
    aihints.ai_socktype = SOCK_STREAM;
    aierr = getaddrinfo(argv[1], portstr, &aihints, &ap);
    
    printf("After getaddrinfo\n");
    PRINT_TIME(id);
    if(clock_gettime(id, &start_ts) < 0) {
      perror("clock_gettime()");
      return 0;
    }
     
    if (aierr) {
        fprintf(stderr, "%s: error looking up host '%s' port '%s'/tcp: %s\n",
                argv[0], argv[1], portstr, gai_strerror(aierr));
        exit(1);
    }
    if (ap == 0) {
        /* Should never happen.  */
        fprintf(stderr, "%s: error looking up host '%s' port '%s'/tcp: no addresses returned?\n",
                argv[0], argv[1], portstr);
        exit(1);
    }
    retval = krb5_sname_to_principal(context, argv[1], service,
                                     KRB5_NT_SRV_HST, &server);
    if (retval) {
        com_err(argv[0], retval, "while creating server name for host %s service %s",
                argv[1], service);
        exit(1);
    }
    PRINT_TIME(id);
    /* set up the address of the foreign socket for connect() */
    apstart = ap; /* For freeing later */
    for (sock = -1; ap && sock == -1; ap = ap->ai_next) {
        char abuf[NI_MAXHOST], pbuf[NI_MAXSERV];
        char mbuf[NI_MAXHOST + NI_MAXSERV + 64];
        int MAX_CONNECT_RETRY = 1;
        int RETRY_COUNT = 0;
/*
        if (getnameinfo(ap->ai_addr, ap->ai_addrlen, abuf, sizeof(abuf),
                        pbuf, sizeof(pbuf), NI_NUMERICHOST | NI_NUMERICSERV)) {
            memset(abuf, 0, sizeof(abuf));
            memset(pbuf, 0, sizeof(pbuf));
            strncpy(abuf, "[error, cannot print address?]",
                    sizeof(abuf)-1);
            strncpy(pbuf, "[?]", sizeof(pbuf)-1);
        }
        memset(mbuf, 0, sizeof(mbuf));
        strncpy(mbuf, "error contacting ", sizeof(mbuf)-1);
        strncat(mbuf, abuf, sizeof(mbuf) - strlen(mbuf) - 1);
        strncat(mbuf, " port ", sizeof(mbuf) - strlen(mbuf) - 1);
        strncat(mbuf, pbuf, sizeof(mbuf) - strlen(mbuf) - 1);*/
        sock = socket(ap->ai_family, SOCK_STREAM, 0);
        if (sock < 0) {
            fprintf(stderr, "%s: socket: %s\n", mbuf, strerror(errno));
            continue;
        }

    PRINT_TIME( id);
        if( connect(sock, ap->ai_addr, ap->ai_addrlen) < 0) {       
           fprintf(stderr, "Connecting");
//           while (connect(sock, ap->ai_addr, ap->ai_addrlen) < 0) {
               //fprintf(stderr, "%s: connect: %s\n", mbuf, strerror(errno));
               //close(sock);
               //sock = -1;
//               sleep(1);
//               fprintf(stderr, ".");
//               if( RETRY_COUNT++ >=  MAX_CONNECT_RETRY ) {
//                  printf("\nFailed to connect after %s retries\n",
//                           MAX_CONNECT_RETRY);
//                  exit(1);
//               }
//            }
        }
        /* connected */
    }
    
    if (sock == -1)
        /* Already printed error message above. */
        exit(1);
    
    printf("socket connection established\n");

    cksum_data.data = argv[3];
    cksum_data.length = strlen(argv[3]);

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
        com_err(argv[0], retval, "while getting client principal");
        exit(1);
    }
  
    PRINT_TIME(id);
    retval = krb5_unparse_name(context, client, &client_name);
    if (retval) {
        com_err(argv[0], retval, "while getting client principal name");
        exit(1);
    }

    krb5_flags req_flags = AP_OPTS_MUTUAL_REQUIRED;

    PRINT_TIME(id);
    // This will attempt to retrieve credentials from the cache,
    // If it's not found, it'll use the TGT in the cache to retrieve
    // a new ticket related to the service principal
    retval = obtain_credentials(context, client, server, req_flags,
                           ccdef, &err_ret,
                           &creds);// Test retrieval of creds.
    
    PRINT_TIME(id);
    // We need to forward the group name to the gcks server
    krb5_authdata **multicast_group_authdata;

    multicast_group_authdata = (krb5_authdata**) malloc(2*
				 sizeof(*multicast_group_authdata));
    if (multicast_group_authdata != NULL) {
        multicast_group_authdata[1] = NULL; // must end authdata chain
        multicast_group_authdata[0] = 
			(krb5_authdata*) malloc(sizeof(krb5_authdata));
        multicast_group_authdata[0]->magic = KV5M_AUTHDATA;
        multicast_group_authdata[0]->ad_type = KRB5_AUTHDATA_MULTICAST_GROUP;

        multicast_group_authdata[0]->contents = 
			(krb5_octet*) malloc(strlen(group_name)+1);

        memset(multicast_group_authdata[0]->contents, 0, strlen(group_name)+1);
        multicast_group_authdata[0]->length = strlen(group_name)+1;
        memcpy(multicast_group_authdata[0]->contents, 
				group_name, strlen(group_name));
    }
    creds->authdata = multicast_group_authdata;


    PRINT_TIME(id);
    retval = krb5_sendauth(context, &auth_context, (krb5_pointer) &sock,
                           SAMPLE_VERSION, client, server,
                           req_flags,
                           &cksum_data,
                           creds,           // no creds, use ccache instead
                           ccdef, &err_ret, &rep_ret,
                           0);// Test retrieval of creds.

    PRINT_TIME( id );
    krb5_free_principal(context, server);       /* finished using it */
    krb5_free_principal(context, client);
    krb5_cc_close(context, ccdef);
    PRINT_TIME(id);
    if (auth_context) krb5_auth_con_free(context, auth_context);

    if (retval && retval != KRB5_SENDAUTH_REJECTED) {
        com_err(argv[0], retval, "while using sendauth");
        exit(1);
    }
    if (retval == KRB5_SENDAUTH_REJECTED) {
        /* got an error */
        printf("sendauth rejected, error reply is:\n\t\"%*s\"\n",
               err_ret->text.length, err_ret->text.data);
    } else if (rep_ret) {
        /* got a reply */
        krb5_free_ap_rep_enc_part(context, rep_ret);

    PRINT_TIME(id);
        /* -------------- Model 1: GCKS as Kerberos Service ----------------- */
        // First message tells us the length of the contents in reply
        printf("sendauth succeeded, reply is:\n");
        if ((retval = net_read(sock, (char *)&xmitlen,
                               sizeof(xmitlen))) <= 0) {
            if (retval == 0)
                errno = ECONNABORTED;
            com_err(argv[0], errno, "while reading data from server");
            exit(1);
        }
        recv_data.length = ntohs(xmitlen);

    PRINT_TIME(id);
        // Allocate the buffer dynamically to copy reply contents
        if (!(recv_data.data = (char *)malloc((size_t) recv_data.length + 1))) {
            com_err(argv[0], ENOMEM,
                    "while allocating buffer to read from server");
            exit(1);
        }

    PRINT_TIME(id);
        // Second read receives actual content from the server
        if ((retval = net_read(sock, (char *)recv_data.data,
                               recv_data.length)) <= 0) {
            if (retval == 0)
                errno = ECONNABORTED;
            com_err(argv[0], errno, "while reading data from server");
            exit(1);
        }
    PRINT_TIME(id);
        if(sock) close(sock);
        sock = NULL;
        recv_data.data[recv_data.length] = '\0';

        // This should be our MIKEY message returned back from server
//        printf("reply len %d, contents:\n%s\n",
//               recv_data.length,recv_data.data);

//        printf("\n===========Retrieving local SESSION KEY=============\n");
        // Test section should work regardless of rep_ret, we are only doing
        // this here because we only want to retrieve the session key to send
        // the MIKEY_INIT Message

        krb5_octet * session_key; // Session key between Service Server & Client
        unsigned int session_key_length = creds->keyblock.length;

        session_key = (unsigned char*)malloc(creds->keyblock.length);
        memcpy(session_key,creds->keyblock.contents,creds->keyblock.length);
    PRINT_TIME(id);
/*
        printf("Client local cache ticket: %i bytes length\n",
            creds->keyblock.length);
        for(i=0; i < creds->keyblock.length; i++)
        {
            fprintf(stderr, "%X", *(session_key+i));
        }

        printf("\n====================================================\n");
*/
        // Attempt to RESP_AUTH MIKEY message
        
        std::string peerUri(client_name);//"jeffrey/admin@LOCALHOST";
        std::string keyMgmtMessage(recv_data.data);
        std::string psk((char*)session_key, session_key_length) ;
        MRef<Mikey *> mikey;

    PRINT_TIME(id);
        MikeyTestConfig *config = new MikeyTestConfig();
        config->psk = psk;
        // FIXME free config
        mikey = new Mikey( config );

    PRINT_TIME(id);
        std::cout<<client_name<< ": MIKEY Responder Authenticate:"<<std::endl;

        // We try to authenticate the initiator message back from server
        if( !mikey->responderAuthenticate( keyMgmtMessage, peerUri ) ){
            std::cout<<"Incoming key management message could not be authenticated"<<std::endl;
            std::cout<<mikey->authError();
        }
        mikey->setMikeyOffer();

    PRINT_TIME(id);
        std::cout<<"MIKEY INIT message authenticated!"<<std::endl;
        
 //       fprintf(stderr, "\n==============================================\n");
        /*------------------------------------------------------------------*/

        free(recv_data.data);

    } else {
        com_err(argv[0], 0, "no error or reply from sendauth!");
        exit(1);
    }
    freeaddrinfo(apstart);
    krb5_free_context(context);
    
    PRINT_TIME(id);
    long time_elapsed; // ns  
    struct timespec end_ts;
//   for(time_elapsed = 0; time_elapsed < LONG_MAX; time_elapsed++){}
    if(clock_gettime(id, &end_ts) < 0) {
      perror("clock_gettime()");
      return 0;
    }
    if( timeLog_filename ) {
       timeLog = fopen(timeLog_filename, "a");
/*       fprintf(timeLog,"[%d, %ld, %d, %ld]\n", 
         (int)end_ts.tv_sec,end_ts.tv_nsec,
         (int)start_ts.tv_sec, start_ts.tv_nsec);*/
      struct timespec diff_ts = diff(start_ts, end_ts);

      time_elapsed = 
         (int)(diff_ts.tv_sec) * 1000000000 +  diff_ts.tv_nsec;
      fprintf(timeLog, "%ld \n",time_elapsed / 1000000);
    }
    PRINT_TIME(id);
    exit(0);
}
