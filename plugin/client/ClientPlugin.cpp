#include <time.h>

#include <cynara-client-plugin.h>
#include <cynara-error.h>

#include <SupportedTypes.h>

using namespace Cynara;

const std::vector<PolicyType> clientTypes = {SupportedTypes::Client::ALLOW_ONCE,
        SupportedTypes::Client::ALLOW_PER_SESSION,
        SupportedTypes::Client::ALLOW_PER_LIFE};

class ClientDummyPlugin : public ClientPluginInterface {
public:
    const std::vector<PolicyType> &getSupportedPolicyTypes() {
        return clientTypes;
    }

    bool isCacheable(const ClientSession &session, const PolicyResult &result) {
        (void)session;
        if (result.policyType() == SupportedTypes::Client::ALLOW_PER_LIFE
                || result.policyType() == SupportedTypes::Client::ALLOW_PER_SESSION)
            return true;
        return false;
    }

    bool isUsable(const ClientSession &session, const ClientSession &prevSession,
                              bool &updateSession, PolicyResult &result) {
        if (result.policyType() == SupportedTypes::Client::ALLOW_PER_LIFE) {
            updateSession = false;
            return true;
        }

        if (result.policyType() == SupportedTypes::Client::ALLOW_PER_SESSION) {
            if (session == prevSession) {
                updateSession = true;
                return true;
            }
            return false;
        }

        return false;
    }

    void invalidate() {

    }

    virtual int toResult(const ClientSession &session, PolicyResult &result) {
        (void) session;
        (void) result;
        return CYNARA_API_ACCESS_ALLOWED;
    }
};


extern "C" {
ExternalPluginInterface *create(void) {
    return new ClientDummyPlugin();
}

void destroy(ExternalPluginInterface *ptr) {
    delete ptr;
}

}
