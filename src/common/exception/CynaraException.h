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
 * @file        src/common/CynaraException.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of CynaraException
 */

#pragma once

#include <exception>
#include <string>

#include <cynara-error.h>

#include "Exception.h"

namespace AskUser {

class CynaraException : public Exception
{
public:
    CynaraException(const std::string &msg, int err) {
        static const char *time2Die = "cynara_strerror error ;)";
        static char strerror[BUFSIZ];

        std::string error = cynara_strerror(err, strerror, sizeof(strerror)) == CYNARA_API_SUCCESS ?
                                                                                strerror : time2Die;

        m_msg = msg + ": " + error;
    }
};

} /* namespace AskUser */
