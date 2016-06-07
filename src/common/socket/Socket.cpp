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
 * @file        Socket.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Implementation of Socket methods
 */

#include "Socket.h"

#include <exception/ErrnoException.h>
#include <log/alog.h>

#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace AskUser {

namespace Socket {

int accept(int fd) {
    int retFd = TEMP_FAILURE_RETRY(::accept(fd, nullptr, nullptr));
    if (retFd < 0)
        throw ErrnoException("Accept socket error");

    ALOGD("Accepted socket <" << retFd << ">");

    return retFd;
}

void close(int fd) {
    int result = TEMP_FAILURE_RETRY(::close(fd));
    if (result < 0) {
        ALOGE("Close socket <" << fd << "> failed");
        return;
    }

    ALOGD("Closed socket <" << fd << ">");
}

int connect(const std::string &path) {
    int fd = -1;
    int result = 0;
    size_t lenght = 0;

    sockaddr_un remote;
    remote.sun_family = AF_UNIX;

    if (path.size() >= sizeof(remote.sun_path))
        throw std::invalid_argument("Path lenght is too big");
    strcpy(remote.sun_path, path.c_str());

    lenght = strlen(remote.sun_path) + sizeof(remote.sun_family);

    fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
        throw ErrnoException("Socket creation failed");

    result = TEMP_FAILURE_RETRY(::connect(fd, (struct sockaddr *)&remote, lenght));
    if (result == -1)
        throw ErrnoException("Connecting to <" + path + "> socket failed");

    ALOGD("Connected to <" << path << "> socket");

    return fd;
}

int listen(const std::string &path) {
    int fd = -1;
    int result = 0;
    size_t lenght = 0;

    sockaddr_un local;
    local.sun_family = AF_UNIX;

    if (path.size() >= sizeof(local.sun_path))
        throw std::invalid_argument("Path lenght is too big");
    strcpy(local.sun_path, path.c_str());

    lenght = strlen(local.sun_path) + sizeof(local.sun_family);

    result = unlink(path.c_str());
    if (result == -1 && errno != ENOENT)
        throw ErrnoException("Unlink " + path + " failed");

    fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
        throw ErrnoException("Socket creation failed");

    result = ::bind(fd, (struct sockaddr *)&local, lenght);
    if (result == -1)
        throw ErrnoException("Binding to <" + path + "> failed");

    result = ::listen(fd, 10);
    if (result == -1)
        throw ErrnoException("Listen on socked failed");

    ALOGD("Listening on <" << path << "> socket");

    return fd;
}

bool recv(int fd, void *buf, size_t size, int flags) {
    int result = 0;
    size_t bytesRead = 0;

    while (bytesRead < size) {
        result = TEMP_FAILURE_RETRY(::recv(fd, buf, size, flags));

        if (result < 0 && errno != ECONNRESET)
            throw ErrnoException("Error receiving data from socket");
        else if (result <= 0)
            return false;

        bytesRead += result;

        ALOGD("Recieved " << bytesRead << "/" << size << " byte(s)");
    }

    return true;
}

bool send(int fd, const void *buf, size_t size, int flags) {
    int result = 0;
    size_t bytesSend = 0;

    while (bytesSend < size) {

        result = TEMP_FAILURE_RETRY(::send(fd, buf, size, flags | MSG_NOSIGNAL));

        if (result < 0) {
            if (errno == EPIPE)
                return false;
            else
                throw ErrnoException("Error sending data to socket");
        }

        bytesSend += result;

        ALOGD("Send " << result << "/" << size << " byte(s)");
    }

    return true;
}

} /* namespace Socket */

} /* namespace AskUser */
