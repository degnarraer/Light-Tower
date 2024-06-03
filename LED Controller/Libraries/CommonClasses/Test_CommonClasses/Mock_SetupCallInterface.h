#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "SetupCallInterfaces.h"

class MockSetupCalleeInterface : public SetupCalleeInterface
{
public:
    MOCK_METHOD(void, Setup, (), (override));
};

class MockSetupCallerInterface : public SetupCallerInterface
{
public:
    MOCK_METHOD(void, RegisterForSetupCall, (SetupCalleeInterface* callee), (override));
    MOCK_METHOD(void, DeRegisterForSetupCall, (SetupCalleeInterface* callee), (override));
    MOCK_METHOD(void, SetupAllSetupCallees, (), (override));
};