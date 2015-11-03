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
 * @file        popup-runner.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>

#include <log/alog.h>
#include "AskUINotificationBackend.h"
#include "popup-runner.h"

namespace { // anonymous

using namespace AskUser::Agent;

const char *POPUP_EXEC = "/usr/bin/askuser-popup"; // askuser-popup binary

static int timeout_child (int timeout, int popup_pid)
{
    int timeout_pid;
    if ((timeout_pid = fork()) == -1) {
        kill(popup_pid, SIGKILL);
        return -1;
    }
    if (timeout_pid == 0) {
        sleep(timeout);
        _exit(0);
    }

    return timeout_pid;
}

static std::string response_to_str (UIResponseType response)
{
    switch (response) {
    case UIResponseType::URT_NO_ONCE:
        return "URT_NO_ONCE";
    case UIResponseType::URT_NO_SESSION:
        return "URT_NO_SESSION";
    case UIResponseType::URT_NO_LIFE:
        return "URT_NO_LIFE";
    case UIResponseType::URT_YES_ONCE:
        return "URT_YES_ONCE";
    case UIResponseType::URT_YES_SESSION:
        return "URT_YES_SESSION";
    case UIResponseType::URT_YES_LIFE:
        return "URT_YES_LIFE";
    default:
        return "URT_ERROR";
    }
}

static int wait_for_popup (int popup_pid, int timeout_pid)
{
    (void) timeout_pid;

    int status;
    pid_t wpid;
    do {
        wpid = waitpid(0, &status, 0);

        if (wpid == timeout_pid) {
            ALOGE("POPUP TIMEOUT");
            kill(popup_pid, SIGKILL);
            return -1;
        }
        else
            if (wpid == popup_pid) {
            // timeout process isn't needed any longer - kill it
            kill(timeout_pid, SIGKILL);
            break;
        }
        ALOGE("Some child process has exited (pid: " << wpid << ")");
    } while (true);

    if (WIFEXITED(status))
        status = WEXITSTATUS(status);
    else {
        ALOGE("Popup terminated abnormally");
        return -1;
    }

    ALOGD("STATUS EXIT ON POPUP (CHILD: " << wpid << "): " << status);

    switch (static_cast <popup_status> (status)) {

    case popup_status::NO_ERROR:
        ALOGD("NO_ERROR");
        return 0;

    case popup_status::EXIT_ERROR:
        ALOGD("ERROR");
        return -1;

    default: // Unknown exit status
        ALOGD("UNKNOWN_ERROR");
        return -1;
    }

    return -1;
}

} // anonymous namespace

namespace AskUser {
namespace Agent {

// BinaryStream class implementation
void BinaryStream::Read(size_t num, void * bytes) {
    size_t max_size = m_data.size();
    for (size_t i = 0; i < num; ++i) {
        if( i + m_readPosition >= max_size){
            return;
        }
        static_cast <unsigned char*> (bytes)[i] = m_data[i + m_readPosition];
    }
    m_readPosition += num;
}

void BinaryStream::Write(size_t num, const void * bytes) {
    for (size_t i = 0; i < num; ++i) {
        m_data.push_back(static_cast <const unsigned char*> (bytes)[i]);
    }
}

BinaryStream::BinaryStream() :
    m_readPosition(0)
{}

BinaryStream::~BinaryStream() {}

const unsigned char* BinaryStream::char_pointer() const {
    return &m_data[0];
}

size_t BinaryStream::size() const {
    return m_data.size();
}
// BinaryStream

UIResponseType run_popup(const std::string &client,
        const std::string &user,
        const std::string &privilege,
        int timeout)
{
    // serialization
    BinaryStream stream;
    AskUser::Serialization::Serialize(stream, client);
    AskUser::Serialization::Serialize(stream, user);
    AskUser::Serialization::Serialize(stream, privilege);

    int   fd_send_to_child[2];
    int   fd_send_to_parent[2];
    pid_t childpid;

    if(0 != pipe(fd_send_to_child)){
        ALOGE("Cannot create pipes!");
        return UIResponseType::URT_ERROR;
    }
    if(0 != pipe(fd_send_to_parent)){
        ALOGE("Cannot create pipes!");
        close(fd_send_to_child[0]);
        close(fd_send_to_child[1]);
        return UIResponseType::URT_ERROR;
    }

    if ((childpid = fork()) == -1) {
        ALOGE("Fork() ERROR");
        close(fd_send_to_child[0]);
        close(fd_send_to_parent[1]);
        goto error;
    }

    if(childpid == 0) { // Child process
        ALOGD("Child");

        // read data from parent
        close(fd_send_to_child[1]);

        // send data to parent
        close(fd_send_to_parent[0]);

        std::stringstream pipe_in_buff;
        std::stringstream pipe_out_buff;
        pipe_in_buff  << fd_send_to_parent[1];
        pipe_out_buff << fd_send_to_child[0];
        std::string pipe_in  = pipe_in_buff.str();
        std::string pipe_out = pipe_out_buff.str();

        ALOGD("Passed file descriptors: " << fd_send_to_child[0] << ", "<< fd_send_to_parent[1]);

        if (execl(POPUP_EXEC, POPUP_EXEC, pipe_out.c_str(), pipe_in.c_str(), NULL) < 0){
            ALOGE("execl FAILED");
        }

        ALOGE("This should not happened!!!");
        _exit(UIResponseType::URT_ERROR);

    } // end of child process
    else { // Parent process
        ALOGD("Parent (child pid: " << childpid << ")");

        int  buff_size = 1024;
        char result[buff_size];

        // send data to child
        close(fd_send_to_child[0]);

        // read data from child
        close(fd_send_to_parent[1]);

        //writing to child
        ALOGD("Sending message to popup-bin process");
        int begin = 0, tmp;
        while (begin * sizeof(char) < stream.size()) {
            tmp = TEMP_FAILURE_RETRY(write(fd_send_to_child[1],
                        stream.char_pointer() + sizeof(char) * begin,
                        stream.size() - sizeof(char)*begin));
            if(-1 == tmp){
                ALOGE("Write to pipe failed!");
                goto error;
            }
            begin += tmp;
        }
        ALOGD("Message has been sent");

        (void) timeout;
        int timeout_pid;
        // TODO: implement popup timeout using sigtimedwait()
        if (timeout > 0 && (timeout_pid = timeout_child(timeout, childpid)) == -1) {
            ALOGE("Cannot setup timeout process. Popup process should be killed.");
            goto error;
        }

        if (wait_for_popup(childpid, timeout_pid) != 0 ) {
            goto error;
        }

        int count;
        count = TEMP_FAILURE_RETRY(read(fd_send_to_parent[0], result, buff_size));

        UIResponseType response;
        if (0 < count) {
            ALOGD("RESULT FROM POPUP PIPE (CHILD) : [ " << count << " ]");
            int response_int;
            BinaryStream stream_in;
            stream_in.Write(count, result);
            AskUser::Deserialization::Deserialize(stream_in, response_int);
            response = static_cast <UIResponseType> (response_int);
            ALOGD("response :" << response_to_str(response));

        }
        else {
            ALOGD("ERROR, count = " << count);;
            goto error;
        }

        ALOGD("popup-runner: EXIT SUCCESS");
        // cleanup
        close(fd_send_to_parent[0]);
        close(fd_send_to_child[1]);
        return response;
    }

    ALOGE("This should not happend!!!");
error:
    // cleanup
    ALOGD("popup-runner: EXIT ERROR");
    close(fd_send_to_parent[0]);
    close(fd_send_to_child[1]);
    return UIResponseType::URT_ERROR;
}

} // UI
} // CCHECKER
