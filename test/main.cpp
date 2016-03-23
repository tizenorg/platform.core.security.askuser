/*
 *  Copyright (c) 2014-2015 Samsung Electronics Co.
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
 * @file        main.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Main test file
 */

#include <iostream>
#include <cstring>

#include <cynara-admin.h>
#include <cynara-client.h>
#include <cynara-error.h>

#include <common/CynaraException.h>
#include <common/SupportedTypes.h>

#include <unistd.h>

cynara *cyn;
cynara_admin *admin;

void check_cynara_return(std::string func, int ret) {
  if (ret < 0) {
    throw AskUser::CynaraException(func, ret);
  }
}

int main(int argc, char **argv)
{
  int ret;

  constexpr size_t clen = 100;
  char *client = new char[clen];
  char *user = new char[clen];
  char *privilege = new char[clen];
  char *bucket = new char[clen];

  strcpy(client, "User::App::org.tizen.task-mgr");
  strcpy(user, "5001");

  if (argc > 1) {
    if (strlen(argv[1]) >= clen)
      throw AskUser::Exception("Passed privilege string is too long");

    strcpy(privilege, argv[1]);
  } else {
    strcpy(privilege, "http://tizen.org/privilege/appmanager.kill");
  }
  strcpy(bucket, "");

  try {

    ret = cynara_admin_initialize(&admin);
    check_cynara_return("cynara_admin_initialize", ret);

    cynara_admin_policy **policies = new cynara_admin_policy*[2];
    cynara_admin_policy policy = {bucket, client, user, privilege,
                                  AskUser::SupportedTypes::Service::ASK_USER , nullptr};

    policies[0] = &policy;
    policies[1] = nullptr;

    ret = cynara_admin_set_policies(admin, policies);
    check_cynara_return("cynara_admin_set_policies", ret);

    ret = cynara_admin_finish(admin);
    check_cynara_return("cynara_admin_finish", ret);

    ret = cynara_initialize(&cyn, nullptr);
    check_cynara_return("cynara_initialize", ret);

    for (int i = 0; i < 10; ++i) {

      ret = cynara_check(cyn, client, "session", user, privilege);
      check_cynara_return("cynara_check", ret);

      switch (ret) {
      case CYNARA_API_ACCESS_ALLOWED:
        std::cout << "Allowed" << std::endl;
        break;
      case CYNARA_API_ACCESS_DENIED:
        std::cout << "Denied" << std::endl;
        break;
      }

    }

    ret = cynara_finish(cyn);
    check_cynara_return("cynara_finish", ret);

  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl;
  }

  return 0;
}
