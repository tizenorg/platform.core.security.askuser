/*
 *  Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Lukasz Wojciechowski <l.wojciechow@partner.samsung.com>
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
/*
 * @file        main.c
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @brief       Main client file
 */

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cynara-client.h>

cynara *cynar;

void user_handler(int sig) {
    (void) sig;
    int ret = cynara_finish(cynar);
    printf("finish ret [%d]\n", ret);
    cynar = NULL;
}

static const char *cystrerr(int errNum) {
    static const char *time2Die = "cynara_strerror error ;)";
    static char strerror[BUFSIZ];

    return cynara_strerror(errNum, strerror, sizeof(strerror)) == CYNARA_API_SUCCESS ? strerror : time2Die;
}

int main(int argc, char **argv) {
    struct sigaction act;
    int ret, repeats = 1;
    int result = 0;

    if (argc > 1) {
        if (sscanf(argv[1], "%d", &repeats) != 1) {
            printf("Wrong repeat count format!\n");
            return 0;
        }
    }

    char *client = argc > 2 ? argv[2] : "__test_client";
    char *session = argc > 3 ? argv[3] : "__test_session";
    char *user = argc > 4 ? argv[4] : "__test_user";
    char *privilege = argc > 5 ? argv[5] : "http://tizen.org/privilege/account.read";

    memset(&act, 0, sizeof(act));
    act.sa_handler = &user_handler;
    if ((ret = sigaction(SIGUSR1, &act, NULL)) < 0) {
        printf("sigaction failed [%d]", ret);
        return 0;
    }

    ret = cynara_initialize(&cynar, NULL);
    printf("init ret [%d]: %s\n", ret, cystrerr(ret));

    while (repeats--) {
        ret = cynara_check(cynar, client, session, user, privilege);
        printf("get ret [%d]: %s\n", ret, cystrerr(ret));
        result += ret == CYNARA_API_ACCESS_ALLOWED ? 1 : -1;
    }

    ret = cynara_finish(cynar);
    printf("finish ret [%d]: %s\n", ret, cystrerr(ret));

    return result;
}
