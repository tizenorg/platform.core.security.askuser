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
 * @file        src/common/config/Limits.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of limits methods
 */


#include "Limits.h"

#include <string>
#include <exception/Exception.h>

namespace AskUser {
namespace Limits {

namespace {

constexpr size_t sizeLimit = 8192;

}

void checkSizeLimit(size_t size) {
    if (size > sizeLimit)
        throw Exception("Size exceeds limits; limit: " +
                        std::to_string(sizeLimit) +
                        " size: " + std::to_string(size));
}

} // namespace Limits
} // namespace AskUser
