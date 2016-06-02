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
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <exception/ErrnoException.h>
#include <log/alog.h>
#include <translator/Translator.h>
#include <config/Path.h>
#include <types/Protocol.h>

namespace AskUser {

namespace Agent {

NotificationTalker::NotificationTalker()
{
    m_initialized = false;
    try {
        int ret, len;
        sockaddr_un local;

        m_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (m_sockfd == -1) {
            setErrnoMsg("socket creation failed");
            return;
        }

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, Path::getSocketPath().c_str());
        unlink(Path::getSocketPath().c_str());
        len = strlen(local.sun_path) + sizeof(local.sun_family);

        ret = bind(m_sockfd, (struct sockaddr *)&local, len);
        if (ret == -1) {
            setErrnoMsg("binding to " + Path::getSocketPath() + " failed");
            return;
        }

        ret = listen(m_sockfd, 10);
        if (ret == -1) {
            setErrnoMsg("listen failed");
            return;
        }
        m_thread = std::thread(&NotificationTalker::run, this);
        m_initialized = true;
    } catch (const std::exception &e) {
        setErrorMsg(std::string("caught std::exception: ") + e.what());
    } catch (...) {
        setErrorMsg("caught unknown exception");
    }
}

void NotificationTalker::setErrnoMsg(const std::string &s, int err)
{
    m_initErrorMsg = s + " : "  + strerror(err);
}

void NotificationTalker::setErrorMsg(std::string s)
{
    m_initErrorMsg = std::move(s);
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
        close(fd);
    }

    m_fdStatus.clear();
    m_fdToUser.clear();
    m_userToFd.clear();

    close(m_sockfd);
    m_sockfd = 0;
}

NotificationTalker::~NotificationTalker()
{
    stop();
    m_thread.join();
}

void NotificationTalker::sendRequest(int fd, const NotificationRequest &request)
{
    m_fdStatus[fd] = false;

    std::string data = Translator::Gui::notificationRequestToData(request.id,
                                                                  request.data.client,
                                                                  request.data.privilege);
    auto size = data.size();

    int len = send(fd, &size, sizeof(size), 0);
    if (len <= 0)
        throw ErrnoException("Error sending data to socket");

    len = send(fd, data.c_str(), size, 0);
    if (len <= 0)
        throw ErrnoException("Error sending data to socket");
}

void NotificationTalker::sendDismiss(int fd)
{
    if (!m_fdStatus[fd]) {
        send(fd, &Protocol::dissmisCode, sizeof(Protocol::dissmisCode), 0);
        m_fdStatus[fd] = true;
    }
}

void NotificationTalker::parseResponse(NotificationResponse response, int fd)
{
    auto &queue = m_requests[m_fdToUser[fd]];
    if (queue.empty()) {
        //ALOGD("Request canceled");
        m_fdStatus[fd] = true;
        return;
    }

    NotificationRequest request = queue.front();
    if (request.id != response.id) {
        //ALOGD("Request canceled");
        m_fdStatus[fd] = true;
        return;
    }

    queue.pop_front();
    ALOGD("For user: <" << request.data.user
          << "> client: <" << request.data.client
          << "> privilege: <" << request.data.privilege
          << "> received: <" << Translator::Gui::responseToString(response.response) << ">");

    m_responseHandler(response);

    send(fd, &Protocol::ackCode, sizeof(Protocol::ackCode), 0);

    m_fdStatus[fd] = true;
}

void NotificationTalker::recvResponses()
{
    for (auto pair : m_userToFd) {
        int fd = std::get<1>(pair);

        if (FD_ISSET(fd, &m_fdSet)) {
            NotificationResponse response;
            int len = recv(fd, &response, sizeof(response), 0);
            if (len < 0) {
                throw ErrnoException("Error receiving data from socket");
            } else if (len) {
                parseResponse(response, fd);
            } else {
                remove(fd);
            }
        }
    }
}

void NotificationTalker::newConnection()
{
    if (FD_ISSET(m_sockfd, &m_fdSet)) {
        int fd;
        sockaddr_un remote;
        socklen_t t = sizeof(remote);

        fd = accept(m_sockfd, (sockaddr*)&remote, &t);
        if (fd < 0)
            throw ErrnoException("Accepting socket error");

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
    ALOGE("Close socket " << fd);
    close(fd);
    auto user = m_fdToUser[fd];
    m_fdToUser.erase(fd);
    m_userToFd.erase(user);
    m_fdStatus.erase(fd);
}

void NotificationTalker::run()
{
    ALOGD("Notification loop started");
    while (!m_stopflag) {
        std::lock_guard<std::mutex> lock(m_bfLock);
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000*100;

        int nfds = m_sockfd;

        FD_ZERO(&m_fdSet);
        FD_SET(m_sockfd, &m_fdSet);

        for (auto pair : m_userToFd) {
            int fd = std::get<1>(pair);
            FD_SET(fd ,&m_fdSet);
            nfds = fd > nfds ? fd : nfds;
        }

        int rv = select(nfds + 1, &m_fdSet, nullptr, nullptr, &timeout);

        if (m_stopflag)
            clear();
            break;

        if (rv < 0) {
            throw ErrnoException("error on select");
        } else if (rv) {
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
}

} /* namespace Agent */

} /* namespace AskUser */
