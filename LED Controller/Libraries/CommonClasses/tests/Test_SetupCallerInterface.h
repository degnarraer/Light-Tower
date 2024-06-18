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
#include "tests/Mock_SetupCallInterface.h"

using ::testing::_;
using ::testing::NotNull;
using namespace testing;

// Test Fixture for SetupCallerInterface
class SetupCallerInterfaceTest : public Test
                               , public SetupCallerInterface
{
protected:
    SetupCallerInterface* setupCaller;
    MockSetupCalleeInterface* mockCallee;

    void SetUp() override
    {
        mockCallee = new MockSetupCalleeInterface();
    }

    void TearDown() override
    {
        free(mockCallee);
    }
};

// Test Registration
TEST_F(SetupCallerInterfaceTest, RegisterCallee)
{
    // Expect the mock callee to be registered
    EXPECT_CALL(*mockCallee, Setup()).Times(0);

    RegisterForSetupCall(mockCallee);

    // Call SetupAllSetupCallees and expect Setup to be called on the mock callee
    EXPECT_CALL(*mockCallee, Setup()).Times(1);
    SetupAllSetupCallees();
}

// Test Deregistration
TEST_F(SetupCallerInterfaceTest, DeRegisterCallee)
{
    // Register and then deregister the mock callee
    RegisterForSetupCall(mockCallee);
    DeRegisterForSetupCall(mockCallee);

    // Call SetupAllSetupCallees and expect Setup not to be called on the mock callee
    EXPECT_CALL(*mockCallee, Setup()).Times(0);
    SetupAllSetupCallees();
}