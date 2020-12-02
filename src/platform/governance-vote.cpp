// Copyright (c) 2014-2018 Crown developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "governance-vote.h"
#include "agent.h"
#include "specialtx.h"
#include "consensus/validation.h"
#include "key_io.h"

#include "sync.h"

namespace Platform
{
    bool CheckVoteTx(const CTransaction& tx, const CBlockIndex* pindex, CValidationState& state)
    {
        AssertLockHeld(cs_main);

        VoteTx vtx;
        if (!GetNftTxPayload(tx, vtx))
            return state.DoS(100, false, REJECT_INVALID, "bad-tx-payload");

        if (!CheckInputsHashAndSig(tx, vtx, vtx.keyId, state))
            return state.DoS(50, false, REJECT_INVALID, "bad-vote-tx-invalid-signature");

        return true;
    }

    bool ProcessVoteTx(const CTransaction& tx, const CBlockIndex* pindex, CValidationState& state)
    {
        try
        {
            VoteTx vtx;
            GetNftTxPayload(tx, vtx);

            CMasternode* pmn = mnodeman.Find(vtx.voterId);
            if(pmn == nullptr)
                return state.DoS(10, false, REJECT_INVALID, "vote-tx-process-fail-no-masternode");

            if (pmn->pubkey2.GetID() != vtx.keyId)
                return state.DoS(10, false, REJECT_INVALID, "vote-tx-process-fail-signed-by-wrong-key");

            AgentsVoting().AcceptVote(vtx.GetVote());

            return true;
        }
        catch (const std::exception& )
        {
            return state.DoS(1, false, REJECT_INVALID, "vote-tx-process-fail");
        }

    }

    int64_t VoteTx::ConvertVote(VoteValue vote)
    {
        // Replace with more explicit serialization
        return static_cast<int64_t>(vote);
    }

    VoteValue VoteTx::ConvertVote(int64_t vote)
    {
        // Replace with more explicit serialization
        return static_cast<VoteValue>(vote);
    }

    Vote VoteTx::GetVote() const
    {
        return { candidate, ConvertVote(vote), time, voterId };
    }

    void VoteTx::ToJson(UniValue& result) const
    {
        result.push_back(Pair("voterId", voterId.ToString()));
        result.push_back(Pair("electionCode", electionCode));
        result.push_back(Pair("vote", vote));
        result.push_back(Pair("time", time));
        result.push_back(Pair("candidate", candidate.ToString()));
        result.push_back(Pair("keyId", EncodeDestination(keyId)));
    }

    std::string VoteTx::ToString() const
    {
        return strprintf(
            "VoteTx(candidate=%s,vote=%d,time=%d,voter=%s,election=%d)",
            candidate.ToString(),vote, time, voterId.ToString(),electionCode
        );

    }
}
