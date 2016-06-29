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
 * @file        SelectRead.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of SelectRead class
 */

#include "SelectRead.h"

#include <exception/ErrnoException.h>

namespace AskUser {

namespace Socket {

SelectRead::SelectRead() : m_exec(true), m_timeout({0, 0}) {}

void SelectRead::add(int fd) {
    if (m_exec) {
        FD_ZERO(&m_set);
        m_nfds = -1;
        m_exec = false;
    }

    FD_SET(fd, &m_set);
    m_nfds = m_nfds > fd ? m_nfds : fd;
}

int SelectRead::exec() {
    int result = 0;

    m_exec = true;

    result = select(m_nfds + 1, &m_set, nullptr, nullptr, &m_timeout);
    if (result == -1)
        throw ErrnoException("Select failed");

    return result;
}

bool SelectRead::isSet(int fd) {
    return FD_ISSET(fd, &m_set);
}

void SelectRead::setTimeout(int ms) {
    m_timeout.tv_usec = ms * 1000;
}

} /* namespace Socket */

} /* namespace AskUser */
