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

#pragma once

#include <cerrno>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <thread>

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
    bool isInitialized() { return m_initialized; }
    std::string getInitErrorMsg() { return m_initErrorMsg; }
    void setResponseHandler(ResponseHandler responseHandler)
    {
        m_responseHandler = responseHandler;
    }
    void parseRequest(RequestType type, NotificationRequest request);
    virtual void stop();

    virtual ~NotificationTalker();

protected:
    void setErrnoMsg(const std::string &s, int err = errno);
    void setErrorMsg(std::string s);
    void run();
    void parseResponse(NotificationResponse response, int fd);
    void recvResponses();

    void newConnection();
    void remove(int fd);

    void clear();

    virtual void addRequest(NotificationRequest &&request);
    virtual void removeRequest(RequestId id);
    virtual void sendRequest(int fd, const NotificationRequest &request);
    virtual void sendDismiss(int fd);

    ResponseHandler m_responseHandler;

    UserToFdMap m_userToFd;
    FdToUserMap m_fdToUser;
    FdStatus m_fdStatus;
    fd_set m_fdSet;
    int m_sockfd;
    bool m_initialized;
    std::string m_initErrorMsg;

    RequestsQueue m_requests;
    std::mutex m_bfLock;

    std::thread m_thread;
    bool m_stopflag = false;
};

} /* namespace Agent */

} /* namespace AskUser */
