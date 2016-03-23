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
 * @file        Translator.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of Translator functions
 */

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <cynara-plugin.h>
#include <string>

#include "Types.h"

void dataToRequest(const Cynara::PluginData &data, std::string &client, std::string &user,
                   std::string &privilege);

Cynara::PluginData requestToData(const std::string &client,
                                 const std::string &user,
                                 const std::string &privilege);

Cynara::PolicyType dataToAnswer(const Cynara::PluginData &data);
Cynara::PluginData answerToData(Cynara::PolicyType answer);

std::string GuiResponseToString(GuiResponse response);

NotificationRequest dataToRecieve(char *data);
std::string recieveToData(RequestId id, const std::string &app, const std::string &perm);

#endif // TRANSLATOR_H
