#include "Translator.h"

#include <limits>
#include <stdexcept>
#include <sstream>

namespace AskUser {
namespace Translator {
namespace Agent {
    RequestData dataToRequest(const Cynara::PluginData &data) {
        std::stringstream stream(data);
        std::size_t strSize;
        std::string members[3];

        for (int i = 0; i < 3; i++) {
            strSize << stream;
            char *buffer = new char[strSize + 1];
            char separator;
            //Consume separator
            stream.read(&separator, 1);
            stream.read(buffer, strSize);
            //read doesn't append null
            buffer[strSize] = '\0';
            members[i] = buffer;
        }
        return RequestData{members[0], members[1], members[2]};
    }
    Cynara::PluginData answerToData(Cynara::PolicyType answer) {
        return std::to_string(answer);
    }
}

namespace Plugin {
    Cynara::PolicyType dataToAnswer(const Cynara::PluginData &data) {
        long long policyType;
        try {
            policyType = std::stoll(data);
        } catch (std::exception &e) {
            throw std::runtime_error("Make me a class");
        }
        auto maxPolicyType = std::numeric_limits<Cynara::PolicyType>::max();
        if (policyType > maxPolicyType) {
            throw std::runtime_error("Me! Me! Me! Me too want class!");
        }
        return reinterpret_cast<Cynara::PolicyType>(policyType);
    }
    Cynara::PluginData requestToData(const std::string &client,
                                     const std::string &user,
                                     const std::string &privilege)
    {
        const char separator = ' ';
        return std::to_string(client.length()) + separator + client + separator
                + std::to_string(user.length()) + separator + user + separator
                + std::to_string(privilege.length()) + separator + privilege + separator;
    }
}
} //namespace Translator
} //namespace AskUser
