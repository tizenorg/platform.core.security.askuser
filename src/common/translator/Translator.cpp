/*
 *  Copyright (c) 2014-2016 Samsung Electronics Co.
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
 * @file        src/common/Translator.cpp
 * @author      Zofia Abramowska <z.abramowska@samsung.com>
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Implementation of Translator methods
 */

#include "Translator.h"

#include <limits>
#include <sstream>

namespace AskUser {
namespace Translator {
namespace Agent {

RequestData dataToRequest(const Cynara::PluginData &data) {
    std::stringstream stream(data);
    std::size_t strSize;
    std::string members[3];

    for (auto &member : members) {
        stream >> strSize;
        std::vector<char> buffer(strSize, '\0');
        char separator;
        //Consume separator
        stream.read(&separator, 1);
        stream.read(buffer.data(), strSize);
        //read doesn't append null
        member.assign(buffer.begin(), buffer.end());
    }
    return RequestData{members[0], members[1], members[2]};
}

Cynara::PluginData answerToData(Cynara::PolicyType answer) {
    return std::to_string(answer);
}

std::string notificationRequestToData(cynara_agent_req_id id, const std::string &app, const std::string &privilege)
{
    const char separator = ' ';
    return std::to_string(id) + separator +
           std::to_string(app.length()) + separator + app + separator +
           std::to_string(privilege.length()) + separator + privilege + separator + separator;
}

} //namespace Agent

namespace Plugin {
Cynara::PolicyType dataToAnswer(const Cynara::PluginData &data) {
    long long policyType;
    try {
        policyType = std::stoll(data);
    } catch (const std::exception &e) {
        throw TranslateErrorException("Could not convert response to PolicyType : " +
                                      data);
    }
    auto maxPolicyType = std::numeric_limits<Cynara::PolicyType>::max();
    if (policyType > maxPolicyType) {
        throw TranslateErrorException("Value of response exceeds max value of PolicyType : "
                                      + std::to_string(policyType));
    }
    return static_cast<Cynara::PolicyType>(policyType);
}

Cynara::PluginData requestToData(const std::string &client,
                                 const std::string &user,
                                 const std::string &privilege)
{
    const char separator = ' ';
    return std::to_string(client.length()) + separator + client + separator
            + std::to_string(user.length()) + separator + user + separator
            + std::to_string(privilege.length()) + separator + privilege + separator;
}

} //namespace Plugin

namespace Gui {
std::string responseToString(GuiResponse response)
{
    switch (response) {
    case GuiResponse::Allow:
        return "Allow";
    case GuiResponse::Deny:
        return "Deny once";
    case GuiResponse::Never:
        return "Deny";
    case GuiResponse::Error:
        return "Error";
    default:
        return "None";
    }
}

NotificationRequest dataToNotificationRequest(char *data) {
    std::stringstream stream(data);
    std::size_t strSize;
    char separator;

    cynara_agent_req_id id;
    std::string members[2];

    stream >> id;
    stream.read(&separator, 1);

    for (auto &member : members) {
        stream >> strSize;
        std::vector<char> buffer(strSize, '\0');
        char separator;
        //Consume separator
        stream.read(&separator, 1);
        stream.read(buffer.data(), strSize);
        //read doesn't append null
        member.assign(buffer.begin(), buffer.end());
    }

    return NotificationRequest({id, std::move(members[0]), std::move(members[1])});
}

} //namespace Gui
} //namespace AskUser
