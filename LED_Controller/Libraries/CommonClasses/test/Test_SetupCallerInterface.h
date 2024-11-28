/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mock_SetupCallInterface.h"

using ::testing::_;
using ::testing::NotNull;
using namespace testing;

// Test Fixture for SetupCallerInterface
class SetupCallerInterfaceTest : public Test
{
protected:
    SetupCallerInterface *setupCaller;
    MockSetupCalleeInterface *mockCallee1;
    MockSetupCalleeInterface *mockCallee2;

    void SetUp() override
    {
        setupCaller = new SetupCallerInterface();
        mockCallee1 = new MockSetupCalleeInterface();
        mockCallee2 = new MockSetupCalleeInterface();
    }

    void TearDown() override
    {
        delete mockCallee1;
        delete mockCallee2;
        delete setupCaller;
    }
};

// Test Registration
TEST_F(SetupCallerInterfaceTest, RegisterCallee)
{
    // Expect the mock callee to be registered
    EXPECT_CALL(*mockCallee1, Setup()).Times(0);
    EXPECT_CALL(*mockCallee2, Setup()).Times(0);

    setupCaller->RegisterForSetupCall(mockCallee1);
    setupCaller->RegisterForSetupCall(mockCallee2);

    // Call SetupAllSetupCallees and expect Setup to be called on the mock callee
    EXPECT_CALL(*mockCallee1, Setup()).Times(1);
    EXPECT_CALL(*mockCallee2, Setup()).Times(1);
    setupCaller->SetupAllSetupCallees();
}

// Test Deregistration
TEST_F(SetupCallerInterfaceTest, DeRegisterCallee)
{
    // Register and then deregister the mock callee
    setupCaller->RegisterForSetupCall(mockCallee1);
    setupCaller->DeRegisterForSetupCall(mockCallee1);

    // Call SetupAllSetupCallees and expect Setup not to be called on the mock callee
    EXPECT_CALL(*mockCallee1, Setup()).Times(0);
    setupCaller->SetupAllSetupCallees();
}