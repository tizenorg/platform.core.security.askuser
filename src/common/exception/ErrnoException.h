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
 * @file        src/common/ErrnoException.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of ErrnoException
 */

#ifndef _SRC_COMMON_ERRNOEXCEPTION_H
#define _SRC_COMMON_ERRNOEXCEPTION_H

#include <cstring>
#include <exception>
#include <string>

#include "Exception.h"

namespace AskUser {

class ErrnoException : public Exception
{
public:
  ErrnoException(std::string msg, int err) {
    constexpr int bufsize = 1024;
    char buf[bufsize];
    char *err_str = strerror_r(err, buf, bufsize);
    m_msg = msg + ": " + err_str;
  }
};

} /* namespace AskUser */

#endif // _SRC_COMMON_ERRNOEXCEPTION_H
