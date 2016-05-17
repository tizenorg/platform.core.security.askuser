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
 * @file        notificationTalker.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Tests for NotificationTalker class
 */

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <daemon/NotificationTalker.h>

using namespace AskUser::Daemon;
using namespace AskUser;

namespace {

class FakeNotificationTalker : public NotificationTalker {
public:
    MOCK_METHOD1(addRequest, void(CynaraRequestPtr request));
    MOCK_METHOD1(removeRequest, void(CynaraRequestPtr request));
    MOCK_METHOD0(stop, void());

    int queueSize() {
        int sum = 0;
        for (auto &pair : requests) {
            sum += std::get<1>(pair).size();
        }
        return sum;
    }

    void invokeAdd(CynaraRequestPtr request) { NotificationTalker::addRequest(request); }
    void invokeRemove(CynaraRequestPtr request) { NotificationTalker::removeRequest(request); }
};

} /* namespace */

TEST(NotificationTalker, addRequest) {
    using testing::Invoke;

    FakeNotificationTalker notificationTalker;
    RequestId id = 1;
    std::string user = "user";
    std::string client = "client";
    std::string privilege = "privilege";

    CynaraRequestPtr ptr = std::make_shared<CynaraRequest>(RequestType::RT_Action, id, user, client,
                                                           privilege);

    EXPECT_CALL(notificationTalker, addRequest(ptr)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeAdd));

    int requestCountBegin = notificationTalker.queueSize();
    notificationTalker.parseRequest(ptr);
    int requestCountEnd = notificationTalker.queueSize();

    ASSERT_EQ(requestCountBegin + 1, requestCountEnd);
}

TEST(NotificationTalker, addAndRemoveRequest) {
    using testing::Invoke;

    FakeNotificationTalker notificationTalker;
    RequestId id = 1;
    std::string user = "user";
    std::string client = "client";
    std::string privilege = "privilege";

    CynaraRequestPtr ptr = std::make_shared<CynaraRequest>(RequestType::RT_Action, id, user, client,
                                                           privilege);
    CynaraRequestPtr ptr2 = std::make_shared<CynaraRequest>(RequestType::RT_Cancel, id);

    EXPECT_CALL(notificationTalker, addRequest(ptr)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeAdd));

    EXPECT_CALL(notificationTalker, removeRequest(ptr2)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeRemove));

    int requestCountBegin = notificationTalker.queueSize();
    notificationTalker.parseRequest(ptr);
    int requestCountAdd = notificationTalker.queueSize();
    notificationTalker.parseRequest(ptr2);
    int requestCountEnd = notificationTalker.queueSize();

    ASSERT_EQ(requestCountBegin + 1, requestCountAdd);
    ASSERT_EQ(requestCountBegin, requestCountEnd);
}

TEST(NotificationTalker, addAndRemoveNonExistingRequest) {
    using testing::Invoke;

    FakeNotificationTalker notificationTalker;
    RequestId id = 1;
    RequestId id2 = 2;
    std::string user = "user";
    std::string client = "client";
    std::string privilege = "privilege";

    CynaraRequestPtr ptr = std::make_shared<CynaraRequest>(RequestType::RT_Action, id, user, client,
                                                           privilege);
    CynaraRequestPtr ptr2 = std::make_shared<CynaraRequest>(RequestType::RT_Cancel, id2);

    EXPECT_CALL(notificationTalker, addRequest(ptr)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeAdd));

    EXPECT_CALL(notificationTalker, removeRequest(ptr2)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeRemove));

    int requestCountBegin = notificationTalker.queueSize();
    notificationTalker.parseRequest(ptr);
    int requestCountAdd = notificationTalker.queueSize();
    notificationTalker.parseRequest(ptr2);
    int requestCountEnd = notificationTalker.queueSize();

    ASSERT_EQ(requestCountBegin + 1, requestCountAdd);
    ASSERT_EQ(requestCountAdd, requestCountEnd);
}

TEST(NotificationTalker, removeNonExistingRequest) {
    using testing::Invoke;

    FakeNotificationTalker notificationTalker;
    RequestId id = 1;
    std::string user = "user";
    std::string client = "client";
    std::string privilege = "privilege";

    CynaraRequestPtr ptr = std::make_shared<CynaraRequest>(RequestType::RT_Cancel, id);

    EXPECT_CALL(notificationTalker, removeRequest(ptr)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeRemove));

    int requestCountBegin = notificationTalker.queueSize();
    notificationTalker.parseRequest(ptr);
    int requestCountEnd = notificationTalker.queueSize();

    ASSERT_EQ(requestCountBegin, requestCountEnd);
}


TEST(NotificationTalker, closeRequest) {
    FakeNotificationTalker notificationTalker;

    CynaraRequestPtr ptr = std::make_shared<CynaraRequest>(RequestType::RT_Close);

    EXPECT_CALL(notificationTalker, stop());
    notificationTalker.parseRequest(ptr);
}
