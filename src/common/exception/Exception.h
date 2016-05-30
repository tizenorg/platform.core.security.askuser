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
 * @file        src/common/Exception.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of Exception and CynaraException classes
 */

#ifndef _SRC_COMMON_EXCEPTION_H
#define _SRC_COMMON_EXCEPTION_H

#include <exception>
#include <string>

namespace AskUser {

class Exception : public std::exception
{
public:
  Exception(std::string msg) {
      m_msg = msg;
  }

  virtual ~Exception() {};

  virtual const char* what() const noexcept {
    return m_msg.c_str();
  }

protected:
  Exception() = default;

  std::string m_msg;
};

} /* namespace AskUser */

#endif // _SRC_COMMON_EXCEPTION_H
