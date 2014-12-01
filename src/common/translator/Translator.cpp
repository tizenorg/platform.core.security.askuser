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
 * @file        Translator.cpp
 * @author      Zofia Abramowska <z.abramowska@samsung.com>
 * @version     1.0
 * @brief       Implementation of Translator methods
 */

#include "Translator.h"

#include <limits>
#include <stdexcept>
#include <sstream>

namespace AskUser {
namespace Translator {
namespace Agent {
    RequestData dataToRequest(const Cynara::PluginData &data) {
        std::stringstream stream(data);
        std::size_t strSize;
        std::string members[3];

        for (int i = 0; i < 3; i++) {
            stream >> strSize;
            char *buffer = new char[strSize + 1];
            char separator;
            //Consume separator
            stream.read(&separator, 1);
            stream.read(buffer, strSize);
            //read doesn't append null
            buffer[strSize] = '\0';
            members[i] = buffer;
        }
        return RequestData{members[0], members[1], members[2]};
    }
    Cynara::PluginData answerToData(Cynara::PolicyType answer) {
        return std::to_string(answer);
    }
}

namespace Plugin {
    Cynara::PolicyType dataToAnswer(const Cynara::PluginData &data) {
        long long policyType;
        try {
            policyType = std::stoll(data);
        } catch (std::exception &e) {
            throw TranslateErrorException("Could not converse response to PolicyType : " +
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
}
} //namespace Translator
} //namespace AskUser
