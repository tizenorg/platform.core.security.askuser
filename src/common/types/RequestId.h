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
 * @file        RequestId.h
 * @author      Zofia Abramowska <z.abramowska@samsung.com>
 * @brief       RequestId typedef
 */

#pragma once

#include <cstdint> // required before fix to cynara-agent.h is submited

#include <cynara-agent.h>

namespace AskUser {

typedef cynara_agent_req_id RequestId;

} // namespace AskUser
