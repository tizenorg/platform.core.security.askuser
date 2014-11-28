#pragma once

#include <string>

namespace AskUser {


struct RequestData {
	std::string client;
	std::string user;
	std::string privilege;
};

} // namespace AskUser
