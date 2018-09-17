// Copyright (c) 2014-2018 Crown Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CROWN_PLATFORM_NF_TOKEN_REGISTRATION_TX_H
#define CROWN_PLATFORM_NF_TOKEN_REGISTRATION_TX_H

#include "key.h"
#include "serialize.h"
#include "nf-token.h"

class CTransaction;
class CMutableTransaction;
class CBlockIndex;
class CValidationState;

namespace Platform
{
    class NfTokenRegistrationTx
    {
    public:
        NfTokenRegistrationTx(const NfToken & nfToken)
            : m_version(CURRENT_VERSION)
            , m_nfToken(nfToken)
        {}

        bool Sign(CKey & privKey, CPubKey & pubKey);

        ADD_SERIALIZE_METHODS

        template<typename Stream, typename Operation>
        inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
        {
            READWRITE(m_version);
            READWRITE(m_nfToken.tokenTypeId);
            READWRITE(m_nfToken.tokenId);
            READWRITE(m_nfToken.tokenOwnerKeyId);
            READWRITE(m_nfToken.metadataAdminKeyId);
            READWRITE(m_nfToken.metadata);
            READWRITE(signature);
        }

        std::string ToString() const;

    public:
        static const int CURRENT_VERSION = 1;
        std::vector<unsigned char> signature; // TODO: temp public to conform the template signing function

    private:
        uint16_t m_version;
        NfToken m_nfToken;
        // TODO: std::vector<unsigned char> m_signature;
    };

    bool CheckNfTokenRegistrationTx(const CTransaction& tx, const CBlockIndex* pIndex, CValidationState& state);
}


#endif // CROWN_PLATFORM_NF_TOKEN_REGISTRATION_TX_H
