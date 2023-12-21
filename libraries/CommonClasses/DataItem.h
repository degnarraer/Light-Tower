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
class SerialPortMessageManager
{
	public:
		SerialPortMessageManager(String taskName, HardwareSerial &serial)
						 : m_Serial(serial)
						 , m_TaskName(taskName)
		{
			xTaskCreatePinnedToCore( StaticSerialMonitor_Loop, m_TaskName.c_str(), 5000, this,  configMAX_PRIORITIES - 1,  &m_TaskHandle,  0 );
		}
		virtual ~SerialPortMessageManager()
		{
			vTaskDelete(m_TaskHandle);
		}
	private:
		HardwareSerial &m_Serial;
		String m_TaskName = "";
		TaskHandle_t m_TaskHandle;
	
		static void StaticSerialMonitor_Loop(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialMonitor_Loop();
		}	
		void SerialMonitor_Loop()
		{
			const TickType_t xFrequency = 20;
			TickType_t xLastWakeTime = xTaskGetTickCount();
			String message = "";
			char character;
			while(true)
			{
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
				while (m_Serial.available())
				{
					character = m_Serial.read();
					if(character == '\n')
					{
						message = m_TaskName + " Debug: " + message;
						Serial.println(message.c_str());
						message = "";
					}
					else
					{
						message.concat(character);
					}
				}
			}
		}
};

template <typename T>
class DataItem
{
	public:
		DataItem( T initialValue, SerialPortMessageManager &portManager )
			    : m_Value(initialValue)
				, m_PortManager(portManager)
		{
		}
		virtual ~DataItem()
		{
		}
	private:
		T m_Value;
		SerialPortMessageManager &m_PortManager;
};