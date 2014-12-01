/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        CapacityCache.h
 * @author      Zofia Abramowska <z.abramowska@samsung.com>
 * @version     1.0
 * @brief       LRU cache container template declaration.
 */

#pragma once

#include <iostream>
#include <functional>
#include <list>
#include <string>
#include <tuple>
#include <unordered_map>

// TODO : redo when common logs support C++ streams
#define LOG(msg) std::cout << msg << std::endl;

namespace Plugin {

template<class Key, class Value>
class CapacityCache {
public:
    using KeyHasherFun = std::function<std::string(const Key &)>;
    static const std::size_t CACHE_DEFAULT_CAPACITY = 100;

    CapacityCache(const std::function<std::string(const Key&)> &func, std::size_t capacity = CACHE_DEFAULT_CAPACITY)
        : m_capacity(capacity),
          m_keyHasher(func)
    {}

    bool get(const Key &key, Value &value);
    bool update(const Key &key, const Value &value);
    void clear();

private:
    typedef std::list<std::string> KeyUsageList;
    typedef std::unordered_map<std::string,
        std::pair<Value, KeyUsageList::iterator>> KeyValueMap;

    void evict();

    std::size_t m_capacity;

    //KeyUsageList::type m_keyUsage;

    KeyHasherFun m_keyHasher;
    KeyUsageList m_keyUsage;
    KeyValueMap m_keyValue;

};

template<class Key, class Value>
bool CapacityCache<Key, Value>::get(const Key &key, Value &value) {
    auto resultIt = m_keyValue.find(m_keyHasher(key));
    //Do we have entry in cache?
    if (resultIt == m_keyValue.end()) {
        /*LOGD("No entry for client=%s user=%s privilege=%s.",
                key.client().toString().c_str(),
                key.user().toString().c_str(),
                key.privilege().toString().c_str());
        */
        return false;
    } else {
        /*LOGD("Entry available for client=%s user=%s privilege=%s",
                key.client().toString().c_str(),
                key.user().toString().c_str(),
                key.privilege().toString().c_str());
        */
        LOG( "Found: " << key << " with value:" << resultIt->second.first);
        value = resultIt->second.first;
        return true;
    }
}

template<class Key, class Value>
void CapacityCache<Key, Value>::clear(void) {
    m_keyUsage.clear();
    m_keyValue.clear();
}

template <class Key, class Value>
void CapacityCache<Key, Value>::evict(void) {

    auto lastUsedKey = m_keyUsage.back();
    m_keyUsage.pop_back();

    auto value_it = m_keyValue.find(lastUsedKey);
    m_keyValue.erase(value_it);
}

template <class Key, class Value>
bool CapacityCache<Key, Value>::update(const Key &key, const Value &value) {
    LOG("Key: " << key << ", Value: " << value);
    if (m_capacity < 0)
        return false;
    std::string cacheKey = m_keyHasher(key);
    if (m_keyValue.size() == m_capacity) {
        //LOGD("Capacity reached.");
        evict();
    }
    auto resultIt = m_keyValue.find(cacheKey);

    bool existed = false;;
    //Move value usage to front
    if (resultIt != m_keyValue.end()) {
        existed = true;
        auto usageIt = resultIt->second.second;
        m_keyUsage.splice(m_keyUsage.begin(), m_keyUsage, usageIt);

    } else {
        m_keyUsage.push_front(cacheKey);
    }

    m_keyValue[cacheKey] = std::make_pair(value, m_keyUsage.begin());
    return existed;
}

} //namespace Cynara


