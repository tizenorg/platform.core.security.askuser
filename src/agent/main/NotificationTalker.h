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
 * @file        src/daemon/NotificationTalker.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of NotificationTalker class
 */

#ifndef __NOTIFICATION_TALKER__
#define __NOTIFICATION_TALKER__

#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <string>

#include <socket/SelectRead.h>
#include <types/RequestId.h>
#include <types/NotificationResponse.h>
#include <types/NotificationRequest.h>

#include <main/Request.h>

namespace AskUser {

namespace Agent {

typedef std::pair<std::string, int> UserToFdPair;
typedef std::map<std::string, int> UserToFdMap;
typedef std::map<int, std::string> FdToUserMap;
typedef std::map<int, bool> FdStatus;

typedef std::map<std::string, std::deque<NotificationRequest>> RequestsQueue;

typedef std::function<void(NotificationResponse)> ResponseHandler;

class NotificationTalker
{
public:
    NotificationTalker();

    void parseRequest(RequestType type, NotificationRequest request);
    void run();
    void setResponseHandler(ResponseHandler responseHandler);
    virtual void stop();

    ~NotificationTalker();

protected:
    ResponseHandler m_responseHandler;

    UserToFdMap m_userToFd;
    FdToUserMap m_fdToUser;
    FdStatus m_fdStatus;
    Socket::SelectRead m_select;
    int m_sockfd = 0;

    RequestsQueue m_requests;
    std::mutex m_mutex;

    bool m_stopflag;

    void parseResponse(NotificationResponse response, int fd);
    void recvResponses();

    void newConnection();
    void remove(int fd);

    virtual void addRequest(NotificationRequest &&request);
    virtual void removeRequest(RequestId id);
    virtual void sendRequest(int fd, const NotificationRequest &request);
    virtual void sendDismiss(int fd);
};

} /* namespace Agent */

} /* namespace AskUser */

#endif /* __NOTIFICATION_TALKER__ */

