#pragma once

#include <types/RequestData.h>
#include <types/SupportedTypes.h>
#include <cynara-plugin.h>

#include <exception>
#include <string>

namespace AskUser {

namespace Translator {

class TranslateErrorException : std::exception {
public:
    TranslateErrorException(const std::string &msg) : m_what(msg) {};
    const char* what() throw() {
        return m_what.c_str();
    }
private:
    std::string m_what;
};

namespace Agent {
    RequestData dataToRequest(const Cynara::PluginData &data);
    Cynara::PluginData answerToData(Cynara::PolicyType answer);
} // namespace Agent

namespace Plugin {
    Cynara::PolicyType dataToAnswer(const Cynara::PluginData &data);
    Cynara::PluginData requestToData(const std::string &client,
                                     const std::string &user,
                                     const std::string &privilege);
} // namespace Plugin

};

} // namespace AskUser

