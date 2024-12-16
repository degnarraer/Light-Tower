/*
    Light Tower by Rob Shockency
    Copyright (C) 2020 Rob Shockency degnarraer@yahoo.com

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
class SoundPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    SoundPowerModel( std::string Title
                   , unsigned int depth
                   , StatisticalEngineModelInterface &StatisticalEngineModelInterface )
      : DataModelWithNewValueNotification<float>(Title, StatisticalEngineModelInterface)
      , m_Depth(depth)
    {
      if (true == debugMemory) Serial << "New: SoundPowerModel\n";
    }
    virtual ~SoundPowerModel()
    {
      if (true == debugMemory) Serial << "Delete: SoundPowerModel\n";
    }

    //Model
    void UpdateValue()
    {
      SetCurrentValue(m_Result);
    }

  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() {
      return false;
    }
  private:
    float m_Result = 0.0;
    unsigned int m_Depth = 0;
    unsigned int m_CircularBufferIndex = 0;
    float m_RunningAverageCircularBuffer[POWER_SAVE_LENGTH] = {0};
    //Model
    void SetupModel() {}
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask()
    {
      int bufferIndex = m_CircularBufferIndex % POWER_SAVE_LENGTH;
      m_RunningAverageCircularBuffer[bufferIndex] = m_StatisticalEngineModelInterface.GetNormalizedSoundPower();
      float total = 0.0;
      int count = 0;
      int depth = m_Depth;
      if (depth > POWER_SAVE_LENGTH - 1) depth = POWER_SAVE_LENGTH - 1;
      for (int i = 0; i <= depth; ++i)
      {
        int index = bufferIndex + i;
        if (index <= POWER_SAVE_LENGTH - 1)
        {
          total += m_RunningAverageCircularBuffer[index];
        }
        else
        {
          total += m_RunningAverageCircularBuffer[index - POWER_SAVE_LENGTH];
        }
        ++count;
      }
      m_Result = total / count;
      if(m_Result >= 1.0) m_Result = 1.0;
      ++m_CircularBufferIndex;
    }
};

class CyclingPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    CyclingPowerModel( std::string Title
                     , unsigned int MilliSeconds
                     , unsigned int MaxCount
                     , StatisticalEngineModelInterface &StatisticalEngineModelInterface )
                     : DataModelWithNewValueNotification<float>(Title, StatisticalEngineModelInterface)
                     , m_MaxCount(MaxCount)
                     , m_MilliSeconds(MilliSeconds) { if (true == debugMemory) Serial << "New: CyclingPowerModel\n"; }
    virtual ~CyclingPowerModel() { if (true == debugMemory) Serial << "Delete: CyclingPowerModel\n"; }
    
    //Model
    void UpdateValue() { SetCurrentValue(m_Result); }

  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() { return false; }
  private:
    uint32_t m_Count = 0;
    const uint32_t m_MaxCount = 1000;
    float m_Result = 0.0;
    unsigned int m_MilliSeconds = 0;
    unsigned long m_CurrentTime;
    unsigned long m_PreviousTime;
    float m_RunningAverageCircularBuffer[POWER_SAVE_LENGTH] = {0};
    //Model
    void SetupModel() {}
    bool CanRunModelTask() {
      m_CurrentTime = millis();
      if(m_CurrentTime - m_PreviousTime >= m_MilliSeconds)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    void RunModelTask()
    {
      ++m_Count;
      m_Result = (float)(m_Count%m_MaxCount)/(float)m_MaxCount;
      m_PreviousTime = m_CurrentTime;
    }
};
