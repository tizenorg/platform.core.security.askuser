/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Yunjin Lee <yunjin-.lee@samsung.com>
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 * @file        SafeFunction.cpp
 * @author      Yunjin Lee <yunjin-.lee@samsung.com>
 * @version     1.0
 * @brief       Util for safe function.
 */

#include "SafeFunction.h"
#include <string.h>
#include <errno.h>

namespace AskUser {
namespace Util {
#define ERROR_STRING_SIZE 256

std::string safeStrError(int error) {
    char buf[ERROR_STRING_SIZE];
    return strerror_r(error, buf, ERROR_STRING_SIZE);
}
}
}
