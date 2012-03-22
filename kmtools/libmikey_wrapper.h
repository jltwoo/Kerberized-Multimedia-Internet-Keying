/* Copyright (C) 2010-2011
 *
 * Authors: Jeffrey Woo <jltwoo@uwaterloo.ca>
*/

#ifndef LIBMIKEY_WRAPPER_H
#define LIBMIKEY_WRAPPER_H

/*
MRef<Mikey *> mikey;
		int type = KEY_AGREEMENT_TYPE_PSK;

		MikeyTestConfig *config = new MikeyTestConfig();
                config->psk = psk;
		// FIXME free config
		mikey = new Mikey( config );
                mikey->addSender(rand());
		keyMgmtMessage = mikey->initiatorCreate( type, peerUri );
*/

#ifndef KEY_AGREEMENT_TYPE_PSK
#define KEY_AGREEMENT_TYPE_PSK 	1
#endif

#ifdef __cplusplus
 extern "C" {
#endif

// incomplete declarations
struct Mikey;	   
struct MikeyTestConfig;

// Constructors
void call_Mikey_Construct( struct Mikey** mikey, struct MikeyTestConfig* config);
void call_MikeyTestConfig_Construct( struct MikeyTestConfig** config);

// MikeyTestConfig helper functions
char* call_MikeyTestConfig_getPSK ( struct MikeyTestConfig* config );
void call_MikeyTestConfig_setPSK ( struct MikeyTestConfig* config, char* newPsk, int length );
char* call_MikeyTestConfig_getTGK ( struct MikeyTestConfig* config );
void call_MikeyTestConfig_setTGK ( struct MikeyTestConfig* config, char* newtgk, int length );
char* call_MikeyTestConfig_setConnInfo( struct MikeyTestConfig* config, char* connInfo );

char* call_Mikey_getTGK ( struct Mikey* mikey );
unsigned int call_Mikey_getTGKLength ( struct Mikey* mikey );


void call_Mikey_addSender( struct Mikey* mikey, int ssrc );

char* call_Mikey_initiatorCreate( struct Mikey* mikey, int kaType, char* peerUri );
unsigned int call_Mikey_responderAuthenticate( struct Mikey* mikey, char* message, char* peerUri );

#ifdef __cplusplus
}
#endif

#endif
