#include "SetupCallInterfaces.h"

void SetupCallerInterface::RegisterForSetupCall(SetupCalleeInterface* newCallee)
{
	ESP_LOGD("RegisterForSetupCall", "Try Registering Callee");
	std::lock_guard<std::mutex> lock(m_Mutex);
	bool isFound = false;
	for (SetupCalleeInterface* callee : m_SetupCallees)
	{
		if(newCallee == callee)
		{
			ESP_LOGE("RegisterForSetupCall", "A callee with this name already exists!");
			isFound = true;
			break;
		}
	}
	if(false == isFound)
	{
		ESP_LOGI("RegisterForSetupCall", "Callee Registered");
		m_SetupCallees.push_back(newCallee);
	}
}
void SetupCallerInterface::DeRegisterForSetupCall(SetupCalleeInterface* callee)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	auto it = std::find(m_SetupCallees.begin(), m_SetupCallees.end(), callee);
	if (it != m_SetupCallees.end())
	{
		m_SetupCallees.erase(it);
	}
}

void SetupCallerInterface::SetupAllSetupCallees()
{
	ESP_LOGD("SetupCallerInterface", "Setup All Setup Callees");
	std::lock_guard<std::mutex> lock(m_Mutex);
	for (SetupCalleeInterface* callee : m_SetupCallees)
	{
		if (callee) 
		{
			callee->Setup();
		}
	}
}
