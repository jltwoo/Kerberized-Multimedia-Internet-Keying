/*
 * File:   MikeyTestConfig.h
 * Author: jeffrey
 *
 * Created on August 11, 2010, 11:01 PM
 */

#ifndef _MIKEYTESTCONFIG_H
#define	_MIKEYTESTCONFIG_H
#include <libmikey/Mikey.h>
#include <libmsip/SipDialogConfig.h>
#include <libmcrypto/SipSimSoft.h>

class MikeyTestConfig: public IMikeyConfig{
	public:
		MikeyTestConfig();

		const std::string getUri()const;
		MRef<SipSim*> getSim()const;

		MRef<CertificateChain*> getPeerCertificate() const;
		size_t getPskLength() const;

		const byte_t* getPsk() const;

		bool isMethodEnabled( int kaType ) const;

		bool isCertCheckEnabled() const;
                std::string psk;
	private:
		MRef<SipIdentity*> identity;
};


#endif	/* _MIKEYTESTCONFIG_H */

