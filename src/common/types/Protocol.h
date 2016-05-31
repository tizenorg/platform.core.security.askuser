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
 * @file        src/common/types/Protocol.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of common types and consts
 */

#pragma once

#include <cstdint>

namespace AskUser {
namespace Protocol {

constexpr uint8_t dissmisCode = 0xDE;
constexpr uint8_t ackCode = 0xAC;

} // namespace Protocol
} // namespace AskUser
