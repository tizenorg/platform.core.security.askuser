/*
 *  Copyright (c) 2014-2016 Samsung Electronics Co.
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
 * @file        ClientPlugin.cpp
 * @author      Zofia Abramowska <z.abramowska@samsung.com>
 * @brief       Implementation of cynara client side AskUser plugin.
 */

#include <cynara-client-plugin.h>
#include <cynara-error.h>
#include <vector>

#include <SupportedTypes.h>
#include <log.h>

using namespace Cynara;

namespace AskUser {
const std::vector<PolicyDescription> clientDescriptions = {
  { SupportedTypes::Client::ALLOW_ONCE, "Allow once" },

  { SupportedTypes::Client::DENY_ONCE, "Deny once" },
};

class ClientPlugin : public ClientPluginInterface {
public:
  const std::vector<PolicyDescription> &getSupportedPolicyDescr() {
    return clientDescriptions;
  }

  bool isCacheable(const ClientSession &/*session*/, const PolicyResult &/*result*/) {
    return false;
  }

  bool isUsable(const ClientSession &/*session*/,
                const ClientSession &/*prevSession*/,
                bool &updateSession,
                PolicyResult &/*result*/)
  {
    updateSession = false;
    return false;
  }

  void invalidate() {}

  virtual int toResult(const ClientSession &/*session*/, PolicyResult &result) {
    switch (result.policyType()) {
    case SupportedTypes::Client::ALLOW_ONCE:
      return CYNARA_API_ACCESS_ALLOWED;
    default:
      return CYNARA_API_ACCESS_DENIED;
    }
  }
};

} // namespace AskUser

extern "C" {
ExternalPluginInterface *create(void) {
  return new AskUser::ClientPlugin();
}

void destroy(ExternalPluginInterface *ptr) {
  delete ptr;
}
} // extern "C"
