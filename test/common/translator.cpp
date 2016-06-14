/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 * @file        translator.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Tests fortranslator funcions
 */

#include <cstring>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <translator/Translator.h>

using namespace AskUser;

TEST(TranslatorTest, PluginData_Strings) {
    std::string client = "TranslatorTest-client";
    std::string user = "TranslatorTest-user";
    std::string privilege = "http://example.com/permissions/example";

    auto data = Translator::Plugin::requestToData(client, user, privilege);
    auto request = Translator::Agent::dataToRequest(data);

    ASSERT_EQ(client, request.client);
    ASSERT_EQ(user, request.user);
    ASSERT_EQ(privilege, request.privilege);
}

TEST(TranslatorTest, NotificationRequest) {
    cynara_agent_req_id id = 1234;
    std::string app = "lorem ipsum dolor est amet";
    std::string privilege = "http://example.com/permissions/example";

    auto data = Translator::Gui::notificationRequestToData(id, app, privilege);
    auto request = Translator::Gui::dataToNotificationRequest(data);

    ASSERT_EQ(id, request.id);
    ASSERT_EQ(app, request.data.client);
    ASSERT_EQ(privilege, request.data.privilege);
}
