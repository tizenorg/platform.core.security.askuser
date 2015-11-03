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

#include <errno.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <log/alog.h>
#include <ui/popup-runner.h>
#include <ui/AskUIPopupBackend.h>

namespace { // anonymous

using namespace AskUser::Agent;

std::string response_to_str(UIResponseType response) {
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

bool wait_for_popup(int popup_pid, int timeout) {
    int status;
    int ret;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);
    siginfo_t info;
    struct timespec time = {timeout, 0L};

    if (popup_pid < 0)
        return false;
    else if (timeout > 0)
        ret = TEMP_FAILURE_RETRY(sigtimedwait(&set, &info, &time));
    else
        ret = TEMP_FAILURE_RETRY(sigwaitinfo(&set, &info));

    sigprocmask(SIG_UNBLOCK, &set, NULL);

    if  (ret == -1 && errno == EAGAIN) {
        ALOGE("POPUP TIMEOUT");
        goto err;
    } else if (ret == SIGCHLD && info.si_pid == popup_pid) {
        // call waitpid on the child process to get rid of zombie process
        waitpid(popup_pid, NULL, 0);

        // The proper signal has been caught and its pid matches to popup_pid
        // Now check the popup exit status
        status = WEXITSTATUS(info.si_status);
        ALOGD("STATUS EXIT ON POPUP (CHILD: " << info.si_pid << "): " << status);

        switch (static_cast<popup_status>(status)) {

        case popup_status::NO_ERROR:
            ALOGD("NO_ERROR");
            return true;

        case popup_status::EXIT_ERROR:
            ALOGD("ERROR");
            return false;

        default: // Unknown exit status
           ALOGD("UNKNOWN_ERROR");
           return false;
        }
    }
    else {
        ALOGE("Some other signal has been caught (pid: " << info.si_pid << ", signal: " << info.si_signo << ")");
        goto err;
    }

err:
    // kill popup process and return error
    kill(popup_pid, SIGKILL);

    // call waitpid on the child process to get rid of zombie process
    waitpid(popup_pid, NULL, 0);
    return false;
}

void child_process(int fd_send_to_child[2], int fd_send_to_parent[2]) {
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

    if (execl(POPUP_EXEC_PATH, POPUP_EXEC_PATH, pipe_out.c_str(), pipe_in.c_str(), NULL) < 0) {
        ALOGE("execl FAILED");
    }

    ALOGE("This should not happen!!!");
    _exit(UIResponseType::URT_ERROR);
}

bool send_message_to_child(const BinaryStream &stream, int fd_send_to_child) {
    ALOGD("Sending message to popup-bin process");
    unsigned int begin = 0;
    int tmp;
    while (begin < stream.size()) {
        tmp = TEMP_FAILURE_RETRY(write(fd_send_to_child,
                                       stream.char_pointer() + begin,
                                       stream.size() - begin));
        if (tmp == -1) {
            ALOGE("Write to pipe failed!");
            return false;
        }
        begin += tmp;
    }
    ALOGD("Message has been sent");
    return true;
}

UIResponseType parse_response(int count, char *data) {
    ALOGD("RESULT FROM POPUP PIPE (CHILD) : [ " << count << " ]");
    int response_int;
    BinaryStream stream_in;
    stream_in.Write(count, data);
    try {
        AskUser::Deserialization::Deserialize(stream_in, response_int);
    } catch (const std::runtime_error &e) {
        ALOGE("Cannot deserialize data. " << e.what());
        return UIResponseType::URT_ERROR;
    }
    UIResponseType response = static_cast <UIResponseType> (response_int);
    ALOGD("response :" << response_to_str(response));
    return response;
}

} // anonymous namespace

namespace AskUser {
namespace Agent {

// BinaryStream class implementation
void BinaryStream::Read(size_t num, void * bytes) {
    size_t max_size = m_data.size();
    if (m_readPosition + num > max_size)
        throw std::runtime_error("Not enough data to read!");
    for (size_t i = 0; i < num; ++i)
        static_cast <unsigned char*> (bytes)[i] = m_data[i + m_readPosition];
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

Popup_runner::Popup_runner() :
    m_popup_pid(-1),
    m_timeout(60),
    m_fd_send_to_parent(-1)
{};

Popup_runner::~Popup_runner() {
    dissmiss();
}

bool Popup_runner::run_popup(const std::string &client,
        const std::string &user,
        const std::string &privilege,
        int timeout) {
    // serialization
    BinaryStream stream;
    AskUser::Serialization::Serialize(stream, client);
    AskUser::Serialization::Serialize(stream, user);
    AskUser::Serialization::Serialize(stream, privilege);

    m_timeout = timeout;

    int   fd_send_to_child[2];
    int   fd_send_to_parent[2];

    if (pipe(fd_send_to_child) != 0) {
        ALOGE("Cannot create pipes!");
        return false;
    }
    if (pipe(fd_send_to_parent) != 0) {
        ALOGE("Cannot create pipes!");
        close(fd_send_to_child[0]);
        close(fd_send_to_child[1]);
        return false;
    }

    // Block SIGCHLD - it's needed to catch the signal in sigtimedwait/sigwaitinfo
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);

    if ((m_popup_pid = fork()) == -1) {
        ALOGE("Fork() ERROR");
        close(fd_send_to_child[0]);
        close(fd_send_to_parent[1]);
        goto error;
    }

    m_fd_send_to_parent = fd_send_to_parent[0];

    if(m_popup_pid == 0) { // Child process
        child_process (fd_send_to_child, fd_send_to_parent);
    } else { // Parent process
        ALOGD("Parent (child pid: " << m_popup_pid << ")");

        // send data to child
        close(fd_send_to_child[0]);

        // read data from child
        close(fd_send_to_parent[1]);

        // writing to child
        if (!send_message_to_child(stream, fd_send_to_child[1])) {
            kill(m_popup_pid, SIGKILL);
            // call waitpid on the child process to get rid of zombie process
            waitpid(m_popup_pid, NULL, 0);
            m_popup_pid = -1;
            goto error;
        }
        close(fd_send_to_child[1]); /* Reader will see EOF */
        fd_send_to_child[1] = -1;

        return true;
    }

    ALOGE("This should not happen!!!");
error:
    // cleanup
    ALOGD("popup-runner: EXIT ERROR");
    close(fd_send_to_parent[0]);
    close(fd_send_to_child[1]);
    return false;
}

UIResponseType Popup_runner::wait_for_response() {
    UIResponseType response = UIResponseType::URT_ERROR;

    int buff_size = 1024;
    char result[buff_size];
    size_t count = 0;

    // wait for child
    if (!wait_for_popup(m_popup_pid, m_timeout))
        goto error;
    m_popup_pid = -1;

    // Read message from popup (child)
    int tmp;
    do {
        tmp = TEMP_FAILURE_RETRY(read(m_fd_send_to_parent, result + count, buff_size - count));
        if (tmp < 0) {
            ALOGE("Error while reading popup response, read returned: " << tmp);
            ALOGE("errno: " << strerror(errno));
            goto error;
        }
        if (tmp > 0) {
            count += tmp;
        }
    } while (tmp != 0);

    // Parsing response from child
    if (count >= sizeof(UIResponseType))
        response = parse_response(count, result);
    else {
        ALOGD("ERROR, count = " << count);;
        goto error;
    }

    ALOGD("popup-runner: EXIT SUCCESS");
    // cleanup
error:
    close(m_fd_send_to_parent);
    m_fd_send_to_parent = -1;
    m_popup_pid = -1;
    return response;
}

bool Popup_runner::dissmiss() {

    if (m_popup_pid > 0) {
        kill(m_popup_pid, SIGKILL);
        // wait_for_response() calls wait() internally
        m_popup_pid = -1;
    }

    if (m_fd_send_to_parent >= 0) {
        close(m_fd_send_to_parent);
        m_fd_send_to_parent = -1;
    }
    return true;
}

} // UI
} // CCHECKER
