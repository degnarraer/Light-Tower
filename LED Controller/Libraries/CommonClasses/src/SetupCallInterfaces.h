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

#include <vector>
#include <mutex>
#include "Streaming.h"

class SetupCalleeInterface
{
    public:
        SetupCalleeInterface()
        {
            ESP_LOGD("SetupCalleeInterface", "Constructing SetupCalleeInterface");
        }

        virtual ~SetupCalleeInterface()
        {
            ESP_LOGD("SetupCalleeInterface", "Deleting SetupCalleeInterface");
        }
        virtual void Setup() = 0;
        virtual String GetName() const = 0;
};

class SetupCallerInterface
{
    public:
        SetupCallerInterface()
        {
            ESP_LOGD("SetupCallerInterface", "Constructing SetupCallerInterface");
        }

        virtual ~SetupCallerInterface()
        {
            std::lock_guard<std::recursive_mutex> lock(m_SetupMutex);
            ESP_LOGD("SetupCallerInterface", "Deleting SetupCallerInterface");
            for (SetupCalleeInterface* callee : m_SetupCallees)
            {
                DeRegisterForSetupCall(callee);
            }
            m_SetupCallees.clear();
            ESP_LOGD("SetupCallerInterface", "SetupCallerInterface Deleted");
        }

        virtual void RegisterForSetupCall(SetupCalleeInterface* newCallee)
		{
			std::lock_guard<std::recursive_mutex> lock(m_SetupMutex);
			ESP_LOGD("SetupCallerInterface", "Try Registering Callee");
			bool isFound = false;
			for (SetupCalleeInterface* callee : m_SetupCallees)
			{
				if(newCallee == callee)
				{
					ESP_LOGW("SetupCallerInterface", "WARNING! Setup Callee already added!");
					isFound = true;
					break;
				}
			}
			if(!isFound)
			{
				ESP_LOGD("SetupCallerInterface", "Callee Registered");
				m_SetupCallees.push_back(newCallee);
			}
		}

        virtual void DeRegisterForSetupCall(SetupCalleeInterface* callee)
		{
			std::lock_guard<std::recursive_mutex> lock(m_SetupMutex);
			auto it = std::find(m_SetupCallees.begin(), m_SetupCallees.end(), callee);
			if (it != m_SetupCallees.end())
			{
				m_SetupCallees.erase(it);
				ESP_LOGD("SetupCallerInterface", "Callee Deregistered");
			}
		}

        virtual void SetupAllSetupCallees()
		{
			std::lock_guard<std::recursive_mutex> lock(m_SetupMutex);
			ESP_LOGI("SetupAllSetupCallees", "Setup All Setup Callees");
			for (SetupCalleeInterface* callee : m_SetupCallees)
			{
				if (callee) 
				{
					callee->Setup();
					ESP_LOGI("SetupAllSetupCallees", "Setting up: \"%s\".", callee->GetName().c_str());
				}
				else
				{
					ESP_LOGW("SetupAllSetupCallees", "WARNING! Null callee encountered in list");
				}
			}
		}

    private:
        std::vector<SetupCalleeInterface*> m_SetupCallees;
        std::recursive_mutex m_SetupMutex;
};