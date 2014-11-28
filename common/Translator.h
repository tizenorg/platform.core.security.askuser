#pragma once

#include "Types.h"
#include "SupportedTypes.h"
#include <cynara-plugin.h>

#include <string>

namespace AskUser {

namespace Translator {

namespace Agent {
    RequestData dataToRequest(const Cynara::PluginData &data);
    Cynara::PluginData answerToData(Cynara::PolicyType answer);
}

namespace Plugin {
    Cynara::PolicyType dataToAnswer(const Cynara::PluginData &data);
    Cynara::PluginData requestToData(const std::string &client,
                                     const std::string &user,
                                     const std::string &privilege);
}

};

} /* namespace AskUser */

