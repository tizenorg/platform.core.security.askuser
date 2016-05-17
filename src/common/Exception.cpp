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
 * @file        Exception.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of Exception and CynaraException classes
 */

#include "Exception.h"

#include <cynara-error.h>
#include <string.h>

namespace AskUser {

Exception::Exception(std::string msg) : m_msg(msg) {}

Exception::Exception(std::string msg, int err)
{
  char *error = strerror_r(err, nullptr, 0);
  m_msg = msg + ": " + error;
}

const char *Exception::what() const noexcept
{
  return m_msg.c_str();
}

CynaraException::CynaraException(std::string msg, int err)
{
  static const char *time2Die = "cynara_strerror error ;)";
  static char strerror[BUFSIZ];

  std::string error = cynara_strerror(err, strerror, sizeof(strerror)) == CYNARA_API_SUCCESS ?
        strerror : time2Die;

  m_msg = msg + ": " + error;
}

}
