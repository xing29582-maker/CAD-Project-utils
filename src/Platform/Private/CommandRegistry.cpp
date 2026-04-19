#include "CommandRegistry.h"

namespace cadutils
{
    CommandRegistry& CommandRegistry::Instance()
    {
        static CommandRegistry s_instance;
        return s_instance;
    }

    void CommandRegistry::Register(const std::string& id, FactoryFn factory)
    {
        m_factories[id] = factory;
    }

    std::unique_ptr<ICommand> CommandRegistry::Create(const std::string& id) const
    {
        auto it = m_factories.find(id);
        if (it == m_factories.end())
            return nullptr;
        return it->second();
    }

    std::vector<std::string> CommandRegistry::GetAllIds() const
    {
        std::vector<std::string> ids;
        ids.reserve(m_factories.size());
        for (const auto& [id, _] : m_factories)
            ids.push_back(id);
        return ids;
    }

    bool CommandRegistry::Contains(const std::string& id) const
    {
        return m_factories.find(id) != m_factories.end();
    }

} // namespace cadutils