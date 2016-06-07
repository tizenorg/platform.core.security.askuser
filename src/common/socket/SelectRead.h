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
 * @file        SelectRead.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of SelectRead class
 */

#pragma once

#include <sys/select.h>

namespace AskUser {

namespace Socket {

class SelectRead {
public:
    SelectRead();

    void add(int fd);
    int exec();
    bool isSet(int fd);
    void setTimeout(int ms);
private:
    bool m_exec;

    fd_set m_set;
    int m_nfds;

    timeval m_timeout;
};

} /* namespace Socket */

} /* namespace AskUser */
