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
            std::lock_guard<std::recursive_mutex> lock(m_Mutex);
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
			ESP_LOGD("SetupCallerInterface", "Try Registering Callee");
			std::lock_guard<std::recursive_mutex> lock(m_Mutex);
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
			std::lock_guard<std::recursive_mutex> lock(m_Mutex);
			auto it = std::find(m_SetupCallees.begin(), m_SetupCallees.end(), callee);
			if (it != m_SetupCallees.end())
			{
				m_SetupCallees.erase(it);
				ESP_LOGI("SetupCallerInterface", "Callee Deregistered");
			}
		}

        virtual void SetupAllSetupCallees()
		{
			ESP_LOGD("SetupCallerInterface", "Setup All Setup Callees");
			std::lock_guard<std::recursive_mutex> lock(m_Mutex);
			for (SetupCalleeInterface* callee : m_SetupCallees)
			{
				if (callee) 
				{
					callee->Setup();
				}
				else
				{
					ESP_LOGW("SetupCallerInterface", "WARNING! Null callee encountered in list");
				}
			}
		}

    private:
        std::vector<SetupCalleeInterface*> m_SetupCallees;
        std::recursive_mutex m_Mutex;
};