/* Copyright (C) 2010-2011
 *
 * Authors: Jeffrey Woo <jltwoo@uwaterloo.ca>
*/
#include "libmikey_wrapper.h"
#include <cstring>

// Libraries to include from C++ Code
#include <libmikey/Mikey.h>
#include <libmikey/KeyAgreement.h>
#include <libmikey/KeyAgreementPSK.h>
#include "MikeyTestConfig.h"

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

// Constructor for Mikey 
extern "C" void call_Mikey_Construct( Mikey** mikey, MikeyTestConfig* config) {
	*mikey = new Mikey(config);
	(*mikey)->setConfig(config);
	(*mikey)->setStateByInt(0); // enum for STATE_START
}
// Constructor for MikeyTestConfig
extern "C" void call_MikeyTestConfig_Construct( MikeyTestConfig** config) {
	*config = new MikeyTestConfig();
}

extern "C" char* call_MikeyTestConfig_getPSK( MikeyTestConfig* config ) {
	return (char*)config->psk.c_str();
}
extern "C" void call_MikeyTestConfig_setPSK( MikeyTestConfig* config, char* newPsk, int length ) {
        std::string tmp(newPsk, length);
	config->psk = tmp;
}
extern "C" char* call_MikeyTestConfig_getTGK ( struct MikeyTestConfig* config ){
	return config->tgkPtr;
}
extern "C" void call_MikeyTestConfig_setTGK ( struct MikeyTestConfig* config, char* newtgk, int tgkLength ){
	config->tgkPtr = new char[tgkLength];
        memcpy(config->tgkPtr, newtgk, tgkLength);
	config->tgkLength = tgkLength;
}

extern "C" char* call_Mikey_getTGK ( struct Mikey* mikey ){
	return (char*)mikey->getTgk();
}
extern "C" unsigned int call_Mikey_getTgkLength ( struct Mikey* mikey ){
	return mikey->getTgkLength();
}
extern "C" char* call_MikeyTestConfig_setConnInfo( struct MikeyTestConfig* config, char* connInfo ){
        config->connInfo = connInfo;
        return config->connInfo;
}
extern "C" void call_Mikey_addSender( Mikey* mikey, int ssrc ) {
	mikey->addSender(ssrc);
}

extern "C" char* call_Mikey_initiatorCreate( struct Mikey* mikey, int kaType, char* peerUri ){
	char * cstr;
	std::string msg;
	
	msg = mikey->initiatorCreate(kaType, peerUri);	
	cstr = new char [msg.size()+1];
	strcpy (cstr, msg.c_str());
        cstr[msg.size()] = '\0';
	return cstr;
}

extern "C" unsigned int call_Mikey_responderAuthenticate( struct Mikey* mikey, char* message,
					    char* peerUri ){
	// bool not supported in C
	return (mikey->responderAuthenticate(message, peerUri))? 1:0;
}

