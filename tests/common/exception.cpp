/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
/**
 * @file        exception.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Tests for exceptions classes
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <common/Exception.h>

using namespace AskUser;

/**
 * @brief   Exception should publicly inherit from std::exception
 *
 * Exception class must publicly inherit from another class, if child class is ought to be catched
 * by parent class catch block.
 *
 * @test    Scenario:
 * - throw Exception
 * - check if std::exception could be catched
 */
TEST(Exception, inherit_publicly_from_std_exception) {
    EXPECT_THROW(throw Exception(""), std::exception);
}

/**
 * @brief   CynaraException should publicly inherit from std::exception
 *
 * Exception class must publicly inherit from another class, if child class is ought to be catched
 * by parent class catch block.
 *
 * @test    Scenario:
 * - throw CynaraException
 * - check if std::exception could be catched
 */
TEST(CynaraException, inherit_publicly_from_std_exception) {
    EXPECT_THROW(throw CynaraException("", 0), std::exception);
}
