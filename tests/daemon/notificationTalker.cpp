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

#include <fcntl.h>
#include <memory>
#include <unistd.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <NotificationTalker.h>

using namespace AskUser::Agent;
using namespace AskUser;

using::testing::_;

namespace {

class FakeNotificationTalker : public NotificationTalker {
public:
    FakeNotificationTalker() {
      m_responseHandler = [](NotificationResponse){};
    }

    void addRequest(NotificationRequest &&request) {
      addRequest_(request);
    }

    MOCK_METHOD1(addRequest_, void(NotificationRequest));
    MOCK_METHOD1(removeRequest, void(RequestId));
    MOCK_METHOD0(stop, void());

    int queueSize() {
        int sum = 0;
        for (auto &pair : m_requests) {
            sum += std::get<1>(pair).size();
        }
        return sum;
    }

    void invokeAdd(NotificationRequest request) {
      NotificationTalker::addRequest(std::move(request));
    }

    void invokeRemove(RequestId id) {
      NotificationTalker::removeRequest(id);
    }

    int getSockFd() {
        return m_sockfd;
    }

    void clear() {
      NotificationTalker::clear();
    }
};

} /* namespace */

TEST(NotificationTalker, addRequest) {
    using testing::Invoke;

    FakeNotificationTalker notificationTalker;
    cynara_agent_req_id id = 1;
    std::string user = "user";
    std::string client = "client";
    std::string privilege = "privilege";

    NotificationRequest ptr(id, user, client, privilege);

    EXPECT_CALL(notificationTalker, addRequest_(_)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeAdd));

    int requestCountBegin = notificationTalker.queueSize();
    notificationTalker.parseRequest(RequestType::RT_Action, ptr);
    int requestCountEnd = notificationTalker.queueSize();

    ASSERT_EQ(requestCountBegin + 1, requestCountEnd);
}

TEST(NotificationTalker, addAndRemoveRequest) {
    using testing::Invoke;

    FakeNotificationTalker notificationTalker;
    cynara_agent_req_id id = 1;
    std::string user = "user";
    std::string client = "client";
    std::string privilege = "privilege";

    NotificationRequest ptr(id, user, client, privilege);
    NotificationRequest ptr2(id);

    EXPECT_CALL(notificationTalker, addRequest_(_)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeAdd));

    EXPECT_CALL(notificationTalker, removeRequest(id)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeRemove));

    int requestCountBegin = notificationTalker.queueSize();
    notificationTalker.parseRequest(RequestType::RT_Action, ptr);
    int requestCountAdd = notificationTalker.queueSize();
    notificationTalker.parseRequest(RequestType::RT_Cancel, ptr2);
    int requestCountEnd = notificationTalker.queueSize();

    ASSERT_EQ(requestCountBegin + 1, requestCountAdd);
    ASSERT_EQ(requestCountBegin, requestCountEnd);
}

TEST(NotificationTalker, addAndRemoveNonExistingRequest) {
    using testing::Invoke;

    FakeNotificationTalker notificationTalker;
    cynara_agent_req_id id = 1;
    cynara_agent_req_id id2 = 2;
    std::string user = "user";
    std::string client = "client";
    std::string privilege = "privilege";

    NotificationRequest ptr(id, user, client, privilege);
    NotificationRequest ptr2(id2);

    EXPECT_CALL(notificationTalker, addRequest_(_)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeAdd));

    EXPECT_CALL(notificationTalker, removeRequest(id2)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeRemove));

    int requestCountBegin = notificationTalker.queueSize();
    notificationTalker.parseRequest(RequestType::RT_Action, ptr);
    int requestCountAdd = notificationTalker.queueSize();
    notificationTalker.parseRequest(RequestType::RT_Cancel, ptr2);
    int requestCountEnd = notificationTalker.queueSize();

    ASSERT_EQ(requestCountBegin + 1, requestCountAdd);
    ASSERT_EQ(requestCountAdd, requestCountEnd);
}

TEST(NotificationTalker, removeNonExistingRequest) {
    using testing::Invoke;

    FakeNotificationTalker notificationTalker;
    cynara_agent_req_id id = 1;

    NotificationRequest ptr(id);

    EXPECT_CALL(notificationTalker, removeRequest(id)).
            WillOnce(Invoke(&notificationTalker, &FakeNotificationTalker::invokeRemove));

    int requestCountBegin = notificationTalker.queueSize();
    notificationTalker.parseRequest(RequestType::RT_Cancel, ptr);
    int requestCountEnd = notificationTalker.queueSize();

    ASSERT_EQ(requestCountBegin, requestCountEnd);
}


TEST(NotificationTalker, closeRequest) {
    FakeNotificationTalker notificationTalker;

    cynara_agent_req_id id = 1;
    NotificationRequest ptr(id);

    EXPECT_CALL(notificationTalker, stop());
    notificationTalker.parseRequest(RequestType::RT_Close, ptr);
}

TEST(NotificationTalker, closeSocket) {
    int fd;

    {
        FakeNotificationTalker notificationTalker;
        fd = notificationTalker.getSockFd();
        ASSERT_NE(-1, fcntl(fd, F_GETFL));
    }

    ASSERT_EQ(-1, fcntl(fd, F_GETFL));
}
