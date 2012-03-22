/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * appl/sample/sserver/sserver.c
 *
 * Copyright 1990,1991 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 *
 * Sample Kerberos v5 server.
 *
 * sample_server:
 * A sample Kerberos server, which reads an AP_REQ from a TCP socket,
 * decodes it, and writes back the results (in ASCII) to the client.
 *
 * Usage:
 * sample_server servername
 *
 * file descriptor 0 (zero) should be a socket connected to the requesting
 * client (this will be correct if this server is started by inetd).
 */

extern "C" {
#include "k5-int.h"
#include "com_err.h"
#include <krb5.h>
}

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "sample.h"

extern krb5_deltat krb5_clockskew;

#ifndef GETPEERNAME_ARG3_TYPE
#define GETPEERNAME_ARG3_TYPE int
#endif

#ifndef MIKEY_KERBEROS
#define MIKEY_KERBEROS
#include <libmikey/Mikey.h>
#include <libmikey/KeyAgreement.h>
#include "MikeyTestConfig.h"
#endif

#define DEBUG

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


static void
usage(char *name)
{
    fprintf(stderr, "usage: %s [-p port] [-s service] [-S keytab]\n",
            name);
}

int
main(int argc, char *argv[])
{
  krb5_context context;
  krb5_auth_context auth_context;
  krb5_ticket * ticket;
  struct sockaddr_in peername;
  GETPEERNAME_ARG3_TYPE  namelen = sizeof(peername);
  int sock = -1;                      /* incoming connection fd */
  krb5_data recv_data;
  short xmitlen;
  krb5_error_code retval;
  krb5_principal server;
  char repbuf[BUFSIZ];
  char *cname;
  char *service = SAMPLE_SERVICE;
  short port = 0;             /* If user specifies port */
  extern int opterr, optind;
  extern char * optarg;
  int ch;
  krb5_keytab keytab = NULL;  /* Allow specification on command line */
  char *progname;
  int on = 1;
  unsigned int i;

  progname = *argv;

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
  
  retval = krb5_init_context(&context);
  if (retval) {
      com_err(argv[0], retval, "while initializing krb5");
      exit(1);
  }

  /* open a log connection */
  openlog("gcks1", 0, LOG_DAEMON);

  /*
   * Parse command line arguments
   *
   */
  opterr = 0;
  while ((ch = getopt(argc, argv, "p:S:s:")) != -1) {
      switch (ch) {
      case 'p':
          port = atoi(optarg);
          break;
      case 's':
          service = optarg;
          break;
      case 'S':
          if ((retval = krb5_kt_resolve(context, optarg, &keytab))) {
              com_err(progname, retval,
                      "while resolving keytab file %s", optarg);
              exit(2);
          }
          break;

      case '?':
      default:
          usage(progname);
          exit(1);
          break;
      }
  }

  argc -= optind;
  argv += optind;

  /* Backwards compatibility, allow port to be specified at end */
  if (argc > 1) {
      port = atoi(argv[1]);
  }

  retval = krb5_sname_to_principal(context, "LOCALHOST", service,
                                   KRB5_NT_SRV_HST, &server);
  if (retval) {
      syslog(LOG_ERR, "while generating service name (%s): %s",
             service, error_message(retval));
      exit(1);
  }

  /*
   * If user specified a port, then listen on that port; otherwise,
   * assume we've been started out of inetd.
   */

   int acc;
   struct sockaddr_in sockin;
   if (port) {
   //    int acc;    
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
          syslog(LOG_ERR, "socket: %m");
          exit(3);
      }
      /* Let the socket be reused right away */
      (void) setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on,
                         sizeof(on));
   
      sockin.sin_family = AF_INET;
      sockin.sin_addr.s_addr = 0;
      sockin.sin_port = htons(port);
   
      if (bind(sock, (struct sockaddr *) &sockin, sizeof(sockin))) {
          syslog(LOG_ERR, "bind: %m");
          exit(3);
      }
   } 
   else {
     printf("No port specified\n");
     /*
      * To verify authenticity, we need to know the address of the
      * client.
      */
     if (getpeername(0, (struct sockaddr *)&peername,
                            &namelen) < 0) {
         printf("Please specify a port\n");
         usage(progname);
         syslog(LOG_ERR, "getpeername: %m");
         exit(1);
     }
   } // if(port)

   // Start waiting for client communication
   while(1) {
         printf("GCKS1 Listening on port %d...\n",port );
         if (listen(sock, 2) == -1) {
             syslog(LOG_ERR, "listen: %m");
             exit(3);
         }
         if (
             (acc = accept(sock, (struct sockaddr *)&peername,
                             &namelen)
             ) == -1)
         {
             syslog(LOG_ERR, "accept: %m");
             exit(3);
         }
         //dup2(acc, 0);
         //close(sock);
         //sock = 0;
      //sock = 0;
    
      //close(sock);

    if(clock_gettime(id, &prev_ts) < 0) { 
      perror("clock_gettime()");
      return 0;
    }
      krb5_data group_name;

      printf("%s(%d): Connection established from [%X]\n", 
                  __FILE__, __LINE__, acc);

      auth_context = NULL;
    PRINT_TIME(id);
      retval = krb5_recvauth_gcks(context, &auth_context, 
                           &acc,//(krb5_pointer)&sock,
                           SAMPLE_VERSION, server,
                           0,   /* no flags */
                           keytab,      /* default keytab is NULL */
                           &ticket, &group_name);

    PRINT_TIME(id);
      if (retval) {
         printf("%s(%d): krb5_recvauth failed: %s\n", __FILE__,__LINE__,
                  error_message(retval));
         syslog(LOG_ERR, "recvauth failed--%s", error_message(retval));
         exit(1);
      }
      printf("%s\n",group_name.data);
      repbuf[sizeof(repbuf) - 1] = '\0';
    
    /* Get client name */
      retval = krb5_unparse_name(context, 
                                 ticket->enc_part2->client,
                                 &cname);
      if ( retval ){
         syslog(LOG_ERR, "unparse failed: %s", error_message(retval));
         strncpy(repbuf, "You are <unparse error>\n", 
                        sizeof(repbuf) - 1);
      } 
      else { // MODEL 1 GCKS 
    PRINT_TIME(id);
         /*--- Authentication successful ---*/
         fprintf(stderr, 
                "\n==============================================\n");
         fprintf(stderr, "Multicast group name received %s from %s \n",
                    group_name.data, cname);
         krb5_octet * session_key;
         unsigned int session_key_length =
                       ticket->enc_part2->session->length;
         session_key = (unsigned char*)malloc(session_key_length);
         memcpy(session_key,ticket->enc_part2->session->contents,
                 session_key_length);
/*
         fprintf(stderr, 
                 "\n==============SESSION KEY TEST================\n");
         fprintf(stderr, 
                 "Server Ticket Session Contents: %i bytes length\n", 
                 session_key_length);
         for(i=0; i < session_key_length; i++)
         {
             fprintf(stderr, "%X", *(session_key+i));
         }
         fprintf(stderr, "\n");
         fprintf(stderr, 
              "\n==============================================\n");
*/    
         /*--------- Attempt to create MIKEY message ----------------*/
         // We are replying back with the encrypted
         // MIKEY_INITIATOR message created using the libmikey
         std::string peerUri(cname); // "jeffrey/admin@LOCALHOST";
         std::string keyMgmtMessage;
         std::string psk((char*)session_key, session_key_length) ;
         MRef<Mikey *> mikey;
        	int type = KEY_AGREEMENT_TYPE_PSK;
  
        	MikeyTestConfig *config = new MikeyTestConfig();
                config->psk = psk;
        	// FIXME free config
        	mikey = new Mikey( config );
                mikey->addSender(rand());
        	keyMgmtMessage = mikey->initiatorCreate( 
                                            type, peerUri );
    PRINT_TIME(id);
  /*
         std::cout<<"\nMIKEY keyMgmtMessage:\n"
                     <<keyMgmtMessage<<std::endl;
         fprintf(stderr, 
               "\n==============================================\n");
  */
         strncpy(repbuf, keyMgmtMessage.c_str(), sizeof(repbuf) - 1);
        /*------------------------------------------------------------*/
         free(cname);
         free(session_key);
         krb5_auth_con_free(context, auth_context);

    } // If failed to unparse ticket  
  
    xmitlen = htons(strlen(repbuf));
    recv_data.length = strlen(repbuf);
    recv_data.data = repbuf;

    if ((retval = krb5_net_write(context, acc, (char *)&xmitlen,
                                 sizeof(xmitlen))) < 0) {
        syslog(LOG_ERR, "%m: while writing len to client");
        exit(1);
    }
    if ((retval = krb5_net_write(context, acc, (char *)recv_data.data,
                                 recv_data.length)) < 0) {
        syslog(LOG_ERR, "%m: while writing data to client");
        exit(1);
    }
  // close(sock);
      close(acc);
      krb5_free_ticket(context, ticket);
    PRINT_TIME(id);
   } //while(1)

   if(keytab)
      krb5_kt_close(context, keytab);
   krb5_free_principal(context, server);
   krb5_free_context(context);
   exit(0);
}
