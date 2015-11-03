/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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
/*
 * @file        popup-runner.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#include <vector>

#include <ui/AskUIInterface.h>
#include <ui/serialization.h>

namespace AskUser {

namespace Agent {

enum popup_status : int {
    NO_ERROR   = 0,
    EXIT_ERROR = 1
};

class BinaryStream : public AskUser::IStream {
  public:
    void Read (size_t num,       void * bytes);
    void Write(size_t num, const void * bytes);

    BinaryStream();
    ~BinaryStream();

    const unsigned char* char_pointer() const;
    size_t size() const;

  private:
    std::vector<unsigned char> m_data;
    size_t m_readPosition;
};

UIResponseType run_popup(const std::string &client,
        const std::string &user,
        const std::string &privilege,
        int timeout); // zero or negative timeout means infinity

} // Agent
} // AskUser
