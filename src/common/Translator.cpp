/*
 *  Copyright (c) 2016 Samsung Electronics Co.
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
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of Translator functions
 */

#include "Translator.h"

#include <sstream>

namespace AskUser {

void dataToRequest(const Cynara::PluginData &data, std::string &client, std::string &user,
                   std::string &privilege) {
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

  client = std::move(members[0]);
  user = std::move(members[1]);
  privilege = std::move(members[2]);
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

Cynara::PolicyType dataToAnswer(const Cynara::PluginData &data) {
  long long policyType;
  policyType = std::stoll(data);
  return static_cast<Cynara::PolicyType>(policyType);
}

Cynara::PluginData answerToData(Cynara::PolicyType answer)
{
  return std::to_string(answer);
}

std::string GuiResponseToString(GuiResponse response)
{
    switch (response)
    {
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

  RequestId id;
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

std::string notificationRequestToData(RequestId id, const std::string &app, const std::string &privilege)
{
  const char separator = ' ';
  return std::to_string(id) + separator +
         std::to_string(app.length()) + separator + app + separator +
         std::to_string(privilege.length()) + separator + privilege + separator + separator;
}

} /* namespace AskUser */
