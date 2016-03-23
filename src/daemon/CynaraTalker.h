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
 * @file        CynaraTalker.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of CynaraTalker class
 */

#ifndef CYNARATALKER_H
#define CYNARATALKER_H

#include <cstdint> // after merging patch to cynara not needed
#include <cynara-agent.h>
#include <cynara-plugin.h>
#include <functional>
#include <memory>
#include <thread>

#include <common/Types.h>

typedef std::function<void(CynaraRequestPtr)> RequestHandler;

class CynaraTalker
{
public:
    CynaraTalker() = default;
    ~CynaraTalker();

    void addResponse(Response response);
    void setRequestHandler(RequestHandler requestHandler);
    void start();
    void stop();

private:
    RequestHandler m_requestHandler;
    cynara_agent *m_cynara;

    std::thread m_thread;
    bool m_stop_thread = false;

    void run();
};

typedef std::unique_ptr<CynaraTalker> CynaraTalkerPtr;

#endif // CYNARATALKER_H
