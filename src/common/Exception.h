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
 * @file        Exception.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of Exception and CynaraException classes
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

namespace AskUser {

class Exception : public std::exception
{
public:
  Exception(std::string msg);
  Exception(std::string msg, int err);
  virtual const char* what() const noexcept;

protected:
  Exception() = default;

private:
  std::string m_msg;
};

class CynaraException : public Exception
{
public:
  CynaraException(std::string msg, int err);

private:
  std::string m_msg;
};

#endif // EXCEPTION_H

} /* namespace AskUser */
