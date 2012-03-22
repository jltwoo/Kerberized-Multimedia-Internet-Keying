#include "MikeyTestConfig.h"

MikeyTestConfig::MikeyTestConfig( /*MRef<SipIdentity*> aIdentity*/ )/*:
                identity(aIdentity)*/ {
    tgkPtr = NULL;
    tgkLength = 0;
    connInfo = NULL;
}

const std::string MikeyTestConfig::getUri() const{
        return "jeffrey/admin@LOCALHOST"; //identity->getSipUri().getProtocolId() + ":" +
                //identity->getSipUri().getUserIpString();
}
MRef<SipSim*> MikeyTestConfig::getSim() const{
     MRef<SipSim*> sim = new SipSimSoft( NULL, NULL );

    return sim;
/* SipSim, defined in libmcrypto - SipSim.h
        return identity->getSim();
*
*/
}

MRef<CertificateChain*> MikeyTestConfig::getPeerCertificate() const{
        return NULL;
}
size_t MikeyTestConfig::getPskLength() const{
        return psk.size();//identity->getPsk().size();
}

const byte_t* MikeyTestConfig::getPsk() const{
        return (byte_t*)psk.c_str();//identity->getPsk().c_str();
}

bool MikeyTestConfig::isMethodEnabled( int kaType ) const{
        switch( kaType ){
                case KEY_AGREEMENT_TYPE_DH:
                case KEY_AGREEMENT_TYPE_PK:
                case KEY_AGREEMENT_TYPE_RSA_R:
                        return false;//identity->dhEnabled;
                case KEY_AGREEMENT_TYPE_PSK: return true;
                case KEY_AGREEMENT_TYPE_DHHMAC:
                        //return identity->pskEnabled;
                default:
                        return false;
        }
}

bool MikeyTestConfig::isCertCheckEnabled() const{
        return false; //identity->checkCert;
}
