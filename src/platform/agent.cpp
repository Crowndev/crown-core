#include "agent.h"
#include "arith_uint256.h"
#include "governance.h"

#include <boost/foreach.hpp>
#include <memory>

namespace std // TODO: remove this when we switch to C++14
{
    template<typename T, typename... Args>
    unique_ptr<T> make_unique(Args &&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

namespace Platform
{
    class AgentRegistryIteratorImpl:
        public boost::iterator_facade<AgentRegistryIteratorImpl, Identity, boost::single_pass_traversal_tag>
    {
    public:
        using ClientIterator = std::map<uint256, Identity>::const_iterator;

        AgentRegistryIteratorImpl(ClientIterator begin, ClientIterator end)
            : m_current(begin)
            , m_end(end)
        {}

    public:
        void increment() { ++m_current; }
        bool equal(const AgentRegistryIteratorImpl& other) const { return m_current == other.m_current; }
        const Identity& dereference() const { return m_current->second; }

    private:
        ClientIterator m_current;
        ClientIterator m_end;
    };


    AgentRegistry::Iterator::Iterator(std::unique_ptr<AgentRegistryIteratorImpl> impl)
        : m_impl(std::move(impl))
    {
    }

    AgentRegistry::Iterator::Iterator(Iterator&&) = default;
    AgentRegistry::Iterator& AgentRegistry::Iterator::operator=(Iterator&&) = default;
    AgentRegistry::Iterator::~Iterator() = default;


    void AgentRegistry::Iterator::increment()
    {
        m_impl->increment();
    }

    bool AgentRegistry::Iterator::equal(const AgentRegistry::Iterator& other) const
    {
        return m_impl->equal(*other.m_impl);
    }

    const Identity& AgentRegistry::Iterator::dereference() const
    {
        return m_impl->dereference();
    }

    AgentRegistry::Iterator AgentRegistry::begin() const
    {
        return Iterator(std::make_unique<AgentRegistryIteratorImpl>(m_agents.begin(), m_agents.end()));
    }

    AgentRegistry::Iterator AgentRegistry::end() const
    {
        return Iterator(std::make_unique<AgentRegistryIteratorImpl>(m_agents.begin(), m_agents.end()));
    }


    AgentRegistry::AgentRegistry()
    {
        Identity a1(ArithToUint256(7));
        Add(a1);

        Identity a2(ArithToUint256(8));
        Add(a2);
    }

    void AgentRegistry::Add(const Identity& agent)
    {
        m_agents.insert(std::make_pair(agent.id, agent));
    }

    void AgentRegistry::Remove(const Identity& agent)
    {
        m_agents.erase(agent.id);
    }

    bool AgentRegistry::IsAgent(uint256 id) const
    {
        return m_agents.find(id) != m_agents.end();
    }

    AgentRegistry& GetAgentRegistry()
    {
        static AgentRegistry s_agent;

        for (const auto& id: AgentsVoting().CalculateResult())
        {
            s_agent.Add(Identity(id));
        }

        return s_agent;
    }
}