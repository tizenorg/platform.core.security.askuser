#pragma once

#include <types/PolicyType.h>
namespace AskUser {
namespace SupportedTypes {

namespace Agent {
const char* AgentType = "AskUser";
} //namespace Agent

namespace Service {
//service
const Cynara::PolicyType ASK_USER = 10;
} //namespace Service

namespace Client {
//client
const Cynara::PolicyType ALLOW_ONCE = 11;
const Cynara::PolicyType ALLOW_PER_SESSION = 12;
const Cynara::PolicyType ALLOW_PER_LIFE = 13;
} //namespace Client
} //namespace SupportedTypes
} //namespace AskUser
