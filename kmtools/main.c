
#include "stdio.h"
#include "libmikey_wrapper.h"

/*
	MRef<Mikey *> mikey;
	int type = KEY_AGREEMENT_TYPE_PSK;
	MikeyTestConfig *config = new MikeyTestConfig();
        config->psk = psk;
	mikey = new Mikey( config );
        mikey->addSender(rand());

	keyMgmtMessage = mikey->initiatorCreate( type, peerUri );
*/

int main(){
	struct MikeyTestConfig* config;
	struct Mikey* mikey;

	// Constructors for C++ objects
	printf("DEBUG0" );
	call_MikeyTestConfig_Construct( &config );
	printf("DEBUG1" );
	call_MikeyTestConfig_setPSK( config, "MIKEY_PSK", sizeof("MIKEY_PSK") );
	printf("DEBUG2" );
	call_Mikey_Construct( &mikey, config );

	printf("Config PSK: %s\n", call_MikeyTestConfig_getPSK(config) );

	call_Mikey_addSender(mikey, rand());
	char* senderMessage = call_Mikey_initiatorCreate(mikey, KEY_AGREEMENT_TYPE_PSK, "jeffrey/admin@LOCALHOST");
	printf("Mikey Sender Message: %s\n", senderMessage );

	int errcode = call_Mikey_responderAuthenticate(mikey, senderMessage, "jeffrey/admin@LOCALHOST");
	printf("Mikey ResponderAuth: %s\n", (errcode>0)?"SUCCESS":"FAILED");
	return 0;
}
