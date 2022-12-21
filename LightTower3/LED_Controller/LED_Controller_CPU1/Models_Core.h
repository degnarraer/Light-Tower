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

#ifndef Models_Core_H
#define Models_Core_H

#include "TaskInterface.h"
#include <LinkedList.h>
#include "Streaming.h"
#include "Tunes.h"
#include "Statistical_Engine.h"
#include "LEDControllerInterface.h"

struct Power
{
  float NormalizedPower;
};

struct BandData
{
  float Power;
  unsigned int Band;
  CRGB Color;
  bool operator==(const BandData& a);
  bool operator!=(const BandData& a);
  friend Print& operator<<(Print& os, const BandData& bd);
};

struct Position
{
  int X;
  int Y;
  bool operator==(const Position& a);
  bool operator!=(const Position& a);
  friend Print& operator<<(Print& os, const Position& pos);
};

struct Size
{
  unsigned int W;
  unsigned int H;
  bool operator==(const Size& a);
  bool operator!=(const Size& a);
};

struct Coordinates
{
  struct Position Position;
  struct Size Size;
  bool operator==(const Coordinates& a);
  bool operator!=(const Coordinates& a);
};


class StatisticalEngineModelInterface;
class Model: public NamedItem
           , public Task
{
  public:
    Model(String Title): NamedItem(Title)
                       , Task(GetTitle()) 
    { 
      if (true == debugMemory) Serial << "New Model: " << GetTitle() << "\n";
    }
    virtual ~Model()
    {
      if (true == debugMemory) Serial << "Delete Model: " << GetTitle() << "\n";
    }

    //ModelEventNotificationCaller
    virtual void UpdateValue() = 0;

  protected:
    CRGB GetRainbowColor(unsigned int numerator, unsigned int denominator);
    CRGB GetRandomNonGrayColor();
    CRGB DimColor(CRGB color, float scalar);
  private:
    virtual void SetupModel() = 0;
    virtual bool CanRunModelTask() = 0;
    virtual void RunModelTask() = 0;

    //Task Interface
    void Setup();
    void RunMyPreTask(){}
    bool CanRunMyScheduledTask();
    void RunMyScheduledTask();
    void RunMyPostTask(){}
};

template <class T> class ModelEventNotificationCaller;
template <class T>
class ModelEventNotificationCallee
{
  public:
    virtual void NewValueNotification(T Value, String context) = 0;
};

template <class T>
class ModelEventNotificationCaller
{
  public:
    ModelEventNotificationCaller<T>() {}
    virtual ~ModelEventNotificationCaller<T>()
    {
      if (true == debugMemory) Serial << "Delete: ModelEventNotificationCaller\n";
    }
    void RegisterForNotification(ModelEventNotificationCallee<T> &callee, String context)
    {
      CallerInterfaceData<T> cid;
      cid.Callee = &callee;
      cid.Context = context;
      if (true == debugModelNotifications) Serial << "ModelEventNotificationCaller: Added\n";
      m_MyCalleesWithContext.add(cid);
      callee.NewValueNotification(GetCurrentValue(), context);
    }
    void DeRegisterForNotification(ModelEventNotificationCallee<T> &callee, String context)
    {
      CallerInterfaceData<T> cid;
      cid.Callee = &callee;
      cid.Context = context;
      for (int i = 0; i < m_MyCalleesWithContext.size(); ++i)
      {
        if (m_MyCalleesWithContext.get(i) == cid)
        {
          m_MyCalleesWithContext.remove(i);
          break;
        }
      }
    }
    bool HasUser()
    {
      return (m_MyCalleesWithContext.size() > 0) ? true : false;
    }
    void SendNewValueNotificationToCallees(T value)
    {
      for (int i = 0; i < m_MyCalleesWithContext.size(); ++i)
      {
        if (true == debugModelNewValueProcessor) Serial << "ModelEventNotificationCaller: Sending Notification Context: " << m_MyCalleesWithContext.get(i).Context << "\t" << "Value: " << value << "\n";
        m_MyCalleesWithContext.get(i).Callee->NewValueNotification(value, m_MyCalleesWithContext.get(i).Context);
      }
    }
    virtual void UpdateValue() = 0;
    virtual T GetCurrentValue() = 0;
  private:
    template <class C>
    struct CallerInterfaceData
    {
      ModelEventNotificationCallee<C>* Callee;
      String Context;
      bool operator==(const CallerInterfaceData<C>& cid)
      {
        return (true == ((cid.Callee == Callee) && (cid.Context == Context))) ? true : false;
      }
    };
    LinkedList<CallerInterfaceData<T>> m_MyCalleesWithContext = LinkedList<CallerInterfaceData<T>>();
};

class StatisticalEngineModelInterfaceUsers
{
  public:
    virtual bool RequiresFFT() = 0;
};

class StatisticalEngineModelInterfaceUserTracker
{
  public:
    void RegisterAsUser(StatisticalEngineModelInterfaceUsers &user);
    void DeRegisterAsUser(StatisticalEngineModelInterfaceUsers &user);
    bool UsersRequireFFT();
  private:
    LinkedList<StatisticalEngineModelInterfaceUsers*> m_MyUsers = LinkedList<StatisticalEngineModelInterfaceUsers*>();
};

class StatisticalEngineModelInterface : public NamedItem
                                      , public Task
                                      , public StatisticalEngineModelInterfaceUserTracker
                                      , MicrophoneMeasureCalleeInterface
{
  public:
    StatisticalEngineModelInterface(StatisticalEngine &StatisticalEngine) : NamedItem("StatisticalEngineModelInterface")
                                                                          , Task(GetTitle())
                                                                          , m_StatisticalEngine(StatisticalEngine)
    { 
    }
    virtual ~StatisticalEngineModelInterface()
    {
    }

    //StatisticalEngine Getters
    unsigned int GetNumberOfBands();
    float GetNormalizedSoundPower();
    float GetBandAverage(unsigned int band, unsigned int depth);
    float GetBandAverageForABandOutOfNBands(unsigned int band, unsigned int depth, unsigned int totalBands);
    float GetBandValue(unsigned int band, unsigned int depth);
    MaxBandSoundData_t GetMaxBandSoundData();
    MaxBandSoundData_t GetMaxBinRightSoundData();
    MaxBandSoundData_t GetMaxBinLeftSoundData();
    
    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState_t) {}
    
  private:
    StatisticalEngine &m_StatisticalEngine;
    
    //Task
    void Setup();
    void RunMyPreTask(){}
    bool CanRunMyScheduledTask();
    void RunMyScheduledTask();
    void RunMyPostTask(){}
};

class DataModel: public Model
               , public StatisticalEngineModelInterfaceUsers
{
  public:
    DataModel( String Title
               , StatisticalEngineModelInterface &StatisticalEngineModelInterface)
      : Model(Title)
      , m_StatisticalEngineModelInterface(StatisticalEngineModelInterface)
    {
      if (true == debugMemory) Serial << "New: DataModel\n";
      m_StatisticalEngineModelInterface.RegisterAsUser(*this);
    }
    virtual ~DataModel()
    {
      if (true == debugMemory) Serial << "Delete: DataModel\n";
      m_StatisticalEngineModelInterface.DeRegisterAsUser(*this);
    }

    //ModelEventNotificationCaller
    virtual void UpdateValue() = 0;

    //StatisticalEngineModelInterfaceUsers
    virtual bool RequiresFFT() = 0;

  protected:
    StatisticalEngineModelInterface &m_StatisticalEngineModelInterface;
  private:
    virtual void SetupModel() = 0;
    virtual bool CanRunModelTask() = 0;
    virtual void RunModelTask() = 0;
    void Setup();
    void RunMyPreTask(){}
    bool CanRunMyScheduledTask();
    void RunMyScheduledTask();
    void RunMyPostTask(){}
};

template <class T>
class ModelWithNewValueNotification: public Model
                                   , public ModelEventNotificationCaller<T>
{
  public:
    ModelWithNewValueNotification<T>(String Title): Model(Title)
    {
      if (true == debugMemory) Serial << "New: ModelWithNewValueNotification\n";
    }
    virtual ~ModelWithNewValueNotification<T>()
    {
      if (true == debugMemory) Serial << "Delete: ModelWithNewValueNotification\n";
    }

  protected:
    void NewCalleeRegistered(ModelEventNotificationCallee<T> &callee)
    {
      callee.NewValueNotification(m_CurrentValue);
    }
    T GetCurrentValue()
    {
      return m_CurrentValue;
    }
    void SetCurrentValue(T value)
    {
      m_CurrentValue = value;
      if (m_PreviousValue != m_CurrentValue)
      {
        m_PreviousValue = m_CurrentValue;
        this->SendNewValueNotificationToCallees(m_CurrentValue);
      }
    }
    virtual void SetupModel() = 0;
    virtual bool CanRunModelTask() = 0;
    virtual void RunModelTask() = 0;
    T m_PreviousValue;
    T m_CurrentValue;
};

template <class T>
class DataModelWithNewValueNotification: public DataModel
  , public ModelEventNotificationCaller<T>
{
  public:
    DataModelWithNewValueNotification<T>(String Title, StatisticalEngineModelInterface &StatisticalEngineModelInterface): DataModel(Title, StatisticalEngineModelInterface)
    {
      if (true == debugMemory) Serial << "New: DataModelWithNewValueNotification\n";
    }
    virtual ~DataModelWithNewValueNotification<T>()
    {
      if (true == debugMemory) Serial << "Delete: DataModelWithNewValueNotification\n";
    }

  protected:
    T GetCurrentValue()
    {
      return m_CurrentValue;
    }
    void SetCurrentValue(T value)
    {
      m_CurrentValue = value;
      if (m_CurrentValue != m_PreviousValue)
      {
        m_PreviousValue = m_CurrentValue;
        this->SendNewValueNotificationToCallees(m_CurrentValue);
      }
    }
    virtual void SetupModel() = 0;
    virtual bool CanRunModelTask() = 0;
    virtual void RunModelTask() = 0;
    T m_PreviousValue;
    T m_CurrentValue;

    //StatisticalEngineModelInterfaceUsers
    virtual bool RequiresFFT() = 0;
};

#endif
