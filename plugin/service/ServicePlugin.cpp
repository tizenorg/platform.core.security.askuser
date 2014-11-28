#include <string>
#include <tuple>
#include <iostream>
#include <ostream>
#include <cynara-plugin.h>

#include "SupportedTypes.h"
#include "CapacityCache.h"

#define LOG(msg) std::cout << msg << std::endl

using namespace Cynara;
typedef std::tuple<std::string, std::string, std::string> Key;
std::function<std::string(const Key&)> hasher = [](const Key &key) {
    const char separator = '\1';
    auto client = std::get<0>(key);
    auto user = std::get<1>(key);
    auto privilege = std::get<2>(key);
    return client + user + privilege + separator +
            std::to_string(client.size()) + separator +
            std::to_string(user.size()) + separator +
            std::to_string(privilege.size());
};

std::ostream &operator<<(std::ostream &os, const Key &key) {
    os << "client: " << std::get<0>(key)
       << ", user: " << std::get<1>(key)
       << ", privilege: " << std::get<2>(key);
    return os;
}

std::ostream &operator<<(std::ostream &os, const PolicyResult &result) {
    os << "type: " << result.policyType()
       << ", metadata: " << result.metadata();
    return os;
}

const std::vector<PolicyType> serviceTypes = {SupportedTypes::Service::ASK_USER};

class AskUserPlugin : public ServicePluginInterface {
public:
    AskUserPlugin()
        : m_cache(hasher, 100)
    {}
    const std::vector<PolicyType> &getSupportedPolicyTypes() {
        return serviceTypes;
    }
    PluginStatus check(const std::string &client,
                       const std::string &user,
                       const std::string &privilege,
                       PolicyResult &result,
                       AgentType &requiredAgent,
                       PluginData &pluginData) noexcept
    {
        try {
            Key key = Key(client, user, privilege);
            PolicyResult temp;
            if (!m_cache.get(key, temp)) {
                pluginData = requestToData(client, user, privilege);
                requiredAgent = getAgentType();
                return PluginStatus::ANSWER_NOTREADY;
            }

            result = temp;
            return PluginStatus::ANSWER_READY;
        } catch (std::exception &e) {
            LOG("Failed with std exception: " << e.what());
        } catch (...) {
            LOG("Failed with unknown exception: ");
        }
        return PluginStatus::ERROR;
    }

    PluginStatus update(const std::string &client,
                        const std::string &user,
                        const std::string &privilege,
                        const PluginData &agentData,
                        PolicyResult &result) noexcept
    {

        try {
            result = dataToResponse(agentData);
            m_cache.update(Key(client, user, privilege), result);
            return PluginStatus::SUCCESS;
        } catch (std::exception &e) {
            LOG("Failed with std exception: " << e.what());
        } catch (...) {
            LOG("Failed with unknown exception: ");
        }
        return PluginStatus::ERROR;
    }

    void invalidate() {
        m_cache.clear();
    }
private:
    typedef std::tuple<std::string, std::string, std::string> Key;

    Plugin::CapacityCache<Key, PolicyResult> m_cache;

    PluginData requestToData(const std::string &client,
                             const std::string &user,
                             const std::string &privilege)
    {
        (void)client;
        (void)user;
        (void)privilege;
        return "";
    }
    PolicyResult dataToResponse(const PluginData &agentData) {
        (void)agentData;
        return PolicyResult();
    }

    AgentType getAgentType() {
        return AgentType("ASK_USER");
    }
};

extern "C" {
ExternalPluginInterface *create(void) {
    return new AskUserPlugin();
}

void destroy(ExternalPluginInterface *ptr) {
    delete ptr;
}

}
