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
 * @brief       Declaration of Socket methods
 */

#pragma once

#include <cstddef>
#include <string>

namespace AskUser {

namespace Socket {

int accept(int fd);
void close(int fd);
int connect(std::string path);
int listen(std::string path);
bool recv(int fd, void *buf, size_t size, int flags = 0);
void send(int fd, const void *buf, size_t size, int flags = 0);

} /* namespace Socket */

} /* namespace AskUser */
