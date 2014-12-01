/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        ConcurrentQueue.h
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @brief       This file declares class representing thread-safe queue with non-blocking pop()
 */

#pragma once

#include <mutex>
#include <queue>
#include <thread>

namespace AskUser {

template<typename T>
class ConcurrentQueue {
public:

    bool pop(T &item) {
        std::unique_lock<std::mutex> mlock(m_mutex);
        if (m_queue.empty())
            return false;

        item = m_queue.front();
        m_queue.pop();

        return true;
    }

    void push(const T &item) {
        std::unique_lock<std::mutex> mlock(m_mutex);
        m_queue.push(item);
        mlock.unlock();
    }

    void push(T &&item) {
        std::unique_lock<std::mutex> mlock(m_mutex);
        m_queue.push(std::move(item));
        mlock.unlock();
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};

} // namespace AskUser
