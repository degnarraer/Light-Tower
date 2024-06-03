#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "DataItem/LocalDataItem.h"
#include "Test_CommonClasses/Mock_SetupCallInterface.h"

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
        delete mockCallee;
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



TEST(LocalDataItemTest, Registered_With_Setup_Caller)
{
    MockSetupCallerInterface* mockSetupCaller = new MockSetupCallerInterface();
    EXPECT_CALL(*mockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
    const int32_t initialValue = 10;
    LocalDataItem<int32_t, 1> *dataItem = new LocalDataItem<int32_t, 1>( "Test Name" 
                                                                       , initialValue
                                                                       , NULL
                                                                       , mockSetupCaller );
    delete dataItem;
    delete mockSetupCaller;
}


TEST(LocalDataItemTest, DeRegistered_With_Setup_Caller_On_Deletion)
{
    MockSetupCallerInterface* mockSetupCaller = new MockSetupCallerInterface();
    EXPECT_CALL(*mockSetupCaller, DeRegisterForSetupCall(NotNull())).Times(1);
    const int32_t initialValue = 10;
    LocalDataItem<int32_t, 1> *dataItem = new LocalDataItem<int32_t, 1>( "Test Name" 
                                                                       , initialValue
                                                                       , NULL
                                                                       , mockSetupCaller );
    delete dataItem;
    delete mockSetupCaller;
}