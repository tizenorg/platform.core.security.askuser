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
 * @brief       Definition of NotificationTalker class
 */

#include "NotificationTalker.h"

#include <algorithm>
#include <cstring>
#include <cynara-creds-socket.h>

#include <exception/ErrnoException.h>
#include <log/alog.h>
#include <socket/Socket.h>
#include <translator/Translator.h>
#include <config/Path.h>
#include <types/Protocol.h>

namespace AskUser {

namespace Agent {

NotificationTalker::NotificationTalker()
{
    m_stopflag = false;
    m_initialized = false;
    m_failed = true;
    m_select.setTimeout(100);
    try {
        m_sockfd = Socket::listen(Path::getSocketPath());
        m_thread = std::thread(&NotificationTalker::run, this);
        m_initialized = true;
    } catch (const Exception &e) {
        setErrorMsg(e.what());
    } catch (const std::exception &e) {
        setErrorMsg(std::string("caught std::exception: ") + e.what());
    } catch (...) {
        setErrorMsg("caught unknown exception");
    }
    m_failed = false;
}

void NotificationTalker::setErrorMsg(std::string s)
{
    m_errorMsg = std::move(s);
}

void NotificationTalker::parseRequest(RequestType type, NotificationRequest request)
{
    if (!m_responseHandler) {
        ALOGE("Response handler not set!");
        return;
    }

    switch (type) {
    case RequestType::RT_Close:
        ALOGD("Close service");
        stop();
        return;
    case RequestType::RT_Action:
        ALOGD("Add request: " << request.id);
        addRequest(std::move(request));
        return;
    case RequestType::RT_Cancel:
        ALOGD("Cancel request: " << request.id);
        removeRequest(request.id);
        return;
    default:
        return;
    }
}

void NotificationTalker::addRequest(NotificationRequest &&request)
{
    auto &queue = m_requests[request.data.user];
    auto it = std::find_if(queue.begin(), queue.end(),
            [&request](const NotificationRequest &req){return req.id == request.id;}
        );

    if (it == queue.end()) {
        queue.emplace_back(std::move(request));
    } else {
        ALOGD("Cynara request already exists");
    }
}

void NotificationTalker::removeRequest(RequestId id)
{
    for (auto &pair : m_requests) {
        auto &queue = std::get<1>(pair);
        auto it = std::find_if(queue.begin(), queue.end(),
                [&id](const NotificationRequest &req){return req.id == id;}
            );
        if (it == queue.end()) {
            ALOGW("Removing non-existent request");
            return;
        }
        if (it == queue.begin()) {
            auto user = std::get<0>(pair);
            auto it2 = m_userToFd.find(user);
            if (it2 != m_userToFd.end())
                sendDismiss(std::get<1>(*it2));
        }

        queue.erase(it);
    }
}

void NotificationTalker::stop()
{
    m_stopflag = true;
}

void NotificationTalker::clear()
{
    for (auto& pair : m_fdStatus) {
        int fd = std::get<0>(pair);
        Socket::close(fd);
    }

    m_fdStatus.clear();
    m_fdToUser.clear();
    m_userToFd.clear();

    Socket::close(m_sockfd);
    m_sockfd = 0;
}

NotificationTalker::~NotificationTalker()
{
    stop();
    clear();
    m_thread.join();
}

void NotificationTalker::sendRequest(int fd, const NotificationRequest &request)
{
    m_fdStatus[fd] = false;

    std::string data = Translator::Gui::notificationRequestToData(request.id,
                                                                  request.data.client,
                                                                  request.data.privilege);
    auto size = data.size();

    Socket::send(fd, &size, sizeof(size));
    Socket::send(fd, data.c_str(), size);
}

void NotificationTalker::sendDismiss(int fd)
{
    if (!m_fdStatus[fd]) {
        Socket::send(fd, &Protocol::dissmisCode, sizeof(Protocol::dissmisCode));
        m_fdStatus[fd] = true;
    }
}

void NotificationTalker::parseResponse(NotificationResponse response, int fd)
{
    auto &queue = m_requests[m_fdToUser[fd]];
    if (queue.empty()) {
        ALOGD("Request canceled");
        m_fdStatus[fd] = true;
        return;
    }

    NotificationRequest request = queue.front();
    if (request.id != response.id) {
        ALOGD("Request canceled");
        m_fdStatus[fd] = true;
        return;
    }

    queue.pop_front();
    ALOGD("For user: <" << request.data.user
          << "> client: <" << request.data.client
          << "> privilege: <" << request.data.privilege
          << "> received: <" << Translator::Gui::responseToString(response.response) << ">");

    m_responseHandler(response);

    Socket::send(fd, &Protocol::ackCode, sizeof(Protocol::ackCode));

    m_fdStatus[fd] = true;
}

void NotificationTalker::recvResponses()
{
    for (auto pair : m_userToFd) {
        int fd = std::get<1>(pair);

        if (m_select.isSet(fd)) {
            NotificationResponse response;
            if (Socket::recv(fd, &response, sizeof(response))) {
                parseResponse(response, fd);
            } else {
                remove(fd);
            }
        }
    }
}

void NotificationTalker::newConnection()
{
    if (m_select.isSet(m_sockfd)) {
        int fd = Socket::accept(m_sockfd);

        char *user_c = nullptr;

        cynara_creds_socket_get_user(fd, USER_METHOD_DEFAULT,&user_c);
        std::string user = user_c ? user_c : "0";

        auto it = m_userToFd.find(user);
        if (it != m_userToFd.end())
            remove(std::get<1>(*it));

        m_userToFd[user] = fd;
        m_fdToUser[fd] = user;
        m_fdStatus[fd] = true;

        free(user_c);

        ALOGD("Accepted new conection for user: " << user);
    }
}

void NotificationTalker::remove(int fd)
{
    Socket::close(fd);
    auto user = m_fdToUser[fd];
    m_fdToUser.erase(fd);
    m_userToFd.erase(user);
    m_fdStatus.erase(fd);
}

void NotificationTalker::run()
{
    try {
        ALOGD("Notification loop started");
        while (!m_stopflag) {
            std::lock_guard<std::mutex> lock(m_bfLock);

            m_select.add(m_sockfd);

            for (auto pair : m_userToFd)
                m_select.add(std::get<1>(pair));

            int rv = m_select.exec();

            if (m_stopflag) {
                clear();
                break;
            }

            if (rv) {
                recvResponses();
                newConnection();
            }

            for (auto pair : m_fdStatus ) {
                int fd = std::get<0>(pair);
                bool b = std::get<1>(pair);
                auto &queue = m_requests[m_fdToUser[fd]];
                if (b && !queue.empty()) {
                    NotificationRequest request = queue.front();
                    sendRequest(fd, request);
                }
            }
        }
        ALOGD("NotificationTalker loop ended");
    } catch (const std::exception &e) {
        setErrorMsg(e.what());
        m_failed = true;
    } catch (...) {
        setErrorMsg("unknown error");
        m_failed = true;
    }
}

} /* namespace Agent */

} /* namespace AskUser */
