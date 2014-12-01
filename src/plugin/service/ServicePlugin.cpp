/*
 *  Copyright (c) 2014 Samsung Electronics Co.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License
 */
/**
 * @file        ServicePlugin.cpp
 * @author      Zofia Abramowska <z.abramowska@samsung.com>
 * @version     1.0
 * @brief       Implementation of cynara server side AskUser plugin.
 */

#include <string>
#include <tuple>
#include <iostream>
#include <ostream>
#include <cynara-plugin.h>

#include <types/SupportedTypes.h>
#include <translator/Translator.h>
#include "CapacityCache.h"

using namespace Cynara;
typedef std::tuple<std::string, std::string, std::string> Key;
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

namespace AskUser {

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
                pluginData = Translator::Plugin::requestToData(client, user, privilege);
                requiredAgent = AgentType(SupportedTypes::Agent::AgentType);
                return PluginStatus::ANSWER_NOTREADY;
            }

            result = temp;
            return PluginStatus::ANSWER_READY;
        } catch (Translator::TranslateErrorException &e) {
            LOG("Error translating request to data : " << e.what());
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
            PolicyType resultType = Translator::Plugin::dataToAnswer(agentData);
            result = PolicyResult(resultType);
            m_cache.update(Key(client, user, privilege), result);
            return PluginStatus::SUCCESS;
        } catch (Translator::TranslateErrorException &e) {
            LOG("Error translating data to answer : " << e.what());
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

} // namespace AskUser
