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
 
#ifndef Models_H
#define Models_H
#include "TaskInterface.h"
#include <LinkedList.h>
#include "Streaming.h"
#include "Tunes.h"
#include "Statistical_Engine.h"
#include "LEDControllerInterface.h"

struct BandData
{
  float Power;
  unsigned int Band;
  CRGB Color;
  bool operator==(const BandData& a)
  {
    return (true == ((a.Power == Power) && (a.Band == Band) && (a.Color == Color)))? true:false;
  }
  bool operator!=(const BandData& a)
  {
    return (true == ((a.Power == Power) && (a.Band == Band) && (a.Color == Color)))? false:true;
  }
};


template <class T> class ModelEventNotificationCaller;
template <class T>
class ModelEventNotificationCallee
{
  public:
    virtual void NewValueNotification(T Value) = 0;
};

template <class T>
class ModelEventNotificationCaller
{
  public:
    ModelEventNotificationCaller<T>(){}
    virtual ~ModelEventNotificationCaller<T>()
    {
      if(true == debugMemory) Serial << "Delete ModelEventNotificationCaller\n";  
    }
    void RegisterForNotification(ModelEventNotificationCallee<T> &callee)
    {
      if(true == debugModelNotifications) Serial << "ModelEventNotificationCaller: Added\n";        
      m_MyCallees.add(&callee);
      callee.NewValueNotification(GetCurrentValue());
    }
    void DeRegisterForNotification(ModelEventNotificationCallee<T> &callee)
    {
      for(int i = 0; i < m_MyCallees.size(); ++i)
      {
        if(m_MyCallees.get(i) == &callee)
        {
          m_MyCallees.remove(i);
          break;
        }
      }
    }
    bool HasUser()
    {
      return (m_MyCallees.size() > 0)? true:false;
    }
    void SendNewValueNotificationToCallees(T value)
    {
      for(int i = 0; i < m_MyCallees.size(); ++i)
      {
        if(true == debugModelNewValueProcessor) Serial << "ModelEventNotificationCaller: Sending Notification " << i << "\n"; 
        m_MyCallees.get(i)->NewValueNotification(value);
      }
    }
    virtual void UpdateValue() = 0;
    virtual T GetCurrentValue() = 0;
  private:
    LinkedList<ModelEventNotificationCallee<T>*> m_MyCallees = LinkedList<ModelEventNotificationCallee<T>*>();
};

class StatisticalEngineModelInterface;

class Model: public Task
{
  public: 
    Model(String Title): Task(Title){}
    virtual ~Model()
    {
      if(true == debugMemory) Serial << "Delete Model\n";  
    }

    //ModelEventNotificationCaller
    virtual void UpdateValue() = 0;
    
  protected:
    CRGB GetColor(unsigned int numerator, unsigned int denominator);
    CRGB GetRandomNonGrayColor();
  private:
    virtual void SetupModel() = 0;
    virtual bool CanRunModelTask() = 0;
    virtual void RunModelTask() = 0;
    void Setup()
    {
      SetupModel();
    }
    bool CanRunMyTask()
    {
      return CanRunModelTask();
    }
    void RunMyTask()
    {
      RunModelTask();
      UpdateValue();
    }
};

class StatisticalEngineModelInterfaceUsers
{
  public:
    virtual bool RequiresFFT() = 0;
};

class StatisticalEngineModelInterfaceUserTracker
{
  public:
    void RegisterAsUser(StatisticalEngineModelInterfaceUsers &user)
    {
      m_MyUsers.add(&user);
    }
    void DeRegisterAsUser(StatisticalEngineModelInterfaceUsers &user)
    {
      for(int i = 0; i < m_MyUsers.size(); ++i)
      {
        if(m_MyUsers.get(i) == &user)
        {
          m_MyUsers.remove(i);
          break;
        }
      }
    }
    bool UsersRequireFFT()
    {
      bool result = false;
      for(int u = 0; u < m_MyUsers.size(); ++u)
      {
        if(true == m_MyUsers.get(u)->RequiresFFT())
        {
          result = true;
          break;
        }
      }
      return result;
    }
    private:
      LinkedList<StatisticalEngineModelInterfaceUsers*> m_MyUsers = LinkedList<StatisticalEngineModelInterfaceUsers*>();
};

class StatisticalEngineModelInterface : public Task
                                      , public StatisticalEngineModelInterfaceUserTracker
                                      , ADCInterruptHandler 
                                      , MicrophoneMeasureCalleeInterface
{
  public:
    StatisticalEngineModelInterface() : Task("StatisticalEngineModelInterface"){}
    virtual ~StatisticalEngineModelInterface()
    {
      if(true == debugMemory) Serial << "Delete StatisticalEngineModelInterface\n";  
    }

    //StatisticalEngine Getters
    unsigned int GetNumberOfBands() { return m_StatisticalEngine.GetNumberOfBands(); }
    float GetNormalizedSoundPower() { return m_StatisticalEngine.GetNormalizedSoundPower(); }
    float GetBandAverage(unsigned int band, unsigned int depth) { return m_StatisticalEngine.GetBandAverage(band, depth); }
    float GetBandAverageForABandOutOfNBands(unsigned int band, unsigned int depth, unsigned int totalBands) { return m_StatisticalEngine.GetBandAverageForABandOutOfNBands(band, depth, totalBands); }
  
    //ADCInterruptHandler
    void HandleADCInterrupt() { m_StatisticalEngine.HandleADCInterrupt(); }
    
    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState){}

  private:
    StatisticalEngine m_StatisticalEngine;
    //Task
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();
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
        m_StatisticalEngineModelInterface.RegisterAsUser(*this);
     }
    virtual ~DataModel()
    {
      if(true == debugMemory) Serial << "Delete DataModel\n";
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
    void Setup()
    {
      SetupModel();
    }
    bool CanRunMyTask()
    {
      return CanRunModelTask();
    }
    void RunMyTask()
    {
      RunModelTask();
      UpdateValue();
    }
};

template <class T>
class ModelWithNewValueNotification: public Model
                                   , public ModelEventNotificationCaller<T>
{
  public:
    ModelWithNewValueNotification<T>(String Title): Model(Title){}
    virtual ~ModelWithNewValueNotification<T>()
    {
      if(true == debugMemory) Serial << "Delete ModelWithNewValueNotification\n";  
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
      if(m_PreviousValue != m_CurrentValue)
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
    DataModelWithNewValueNotification<T>(String Title, StatisticalEngineModelInterface &StatisticalEngineModelInterface): DataModel(Title, StatisticalEngineModelInterface){}
    virtual ~DataModelWithNewValueNotification<T>()
    {
      if(true == debugMemory) Serial << "Delete DataModelWithNewValueNotification\n";  
    }
  
  protected:
    T GetCurrentValue()
    {
      return m_CurrentValue; 
    }
    void SetCurrentValue(T value)
    {
      m_CurrentValue = value;
      if(m_CurrentValue != m_PreviousValue)
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

class SoundPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    SoundPowerModel(String Title, StatisticalEngineModelInterface &StatisticalEngineModelInterface): DataModelWithNewValueNotification<float>(Title, StatisticalEngineModelInterface){}
    virtual ~SoundPowerModel()
    {
      if(true == debugMemory) Serial << "Delete SoundPowerModel\n";  
    }
    
     //Model
    void UpdateValue() { SetCurrentValue(m_StatisticalEngineModelInterface.GetNormalizedSoundPower()); }

  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() { return false; }
  private:
     //Model
    void SetupModel(){}
    bool CanRunModelTask(){ return true; }
    void RunModelTask(){}
};

class BandPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    BandPowerModel(String Title, unsigned int Band, StatisticalEngineModelInterface &StatisticalEngineModelInterface): DataModelWithNewValueNotification<float>(Title, StatisticalEngineModelInterface)
                                                                                                                     , m_Band(Band){}
    virtual ~BandPowerModel()
    {
      if(true == debugMemory) Serial << "Delete BandPowerModel\n";  
    }
    
     //Model
    void UpdateValue()
    {
      float value = (m_StatisticalEngineModelInterface.GetBandAverage(m_Band, 1) / (float)ADDBITS);
      if(true == debugModels) Serial << "BandPowerModel value: " << value << " for band: " << m_Band << "\n";
      SetCurrentValue( value );
    }
  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() { return true; }
  private:
     //Model
    unsigned int m_Band = 0;
    void SetupModel(){}
    bool CanRunModelTask(){ return true; }
    void RunModelTask(){}
};

class ReducedBandsBandPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    ReducedBandsBandPowerModel( String Title
                              , unsigned int band
                              , unsigned int depth
                              , unsigned int totalBands
                              , StatisticalEngineModelInterface &StatisticalEngineModelInterface)
                              : DataModelWithNewValueNotification<float>(Title, StatisticalEngineModelInterface)
                              , m_Band(band)
                              , m_Depth(depth)
                              , m_TotalBands(totalBands){}
    virtual ~ReducedBandsBandPowerModel()
    {
      if(true == debugMemory) Serial << "Delete ReducedBandsBandPowerModel\n";  
    }
    
     //Model
    void UpdateValue()
    {
      float value = (m_StatisticalEngineModelInterface.GetBandAverageForABandOutOfNBands(m_Band, m_Depth, m_TotalBands) / (float)ADDBITS);
      if(true == debugModels) Serial << "ReducedBandsBandPowerModel value: " << value << " for band: " << m_Band << " of " << m_TotalBands << " bands\n";
      SetCurrentValue( value );
    }
  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() { return true; }
  private:
     //Model
    unsigned int m_Band = 0;
    unsigned int m_Depth = 0;
    unsigned int m_TotalBands = 0;
    void SetupModel(){}
    bool CanRunModelTask(){ return true; }
    void RunModelTask(){}
};

class RandomColorFadingModel: public ModelWithNewValueNotification<CRGB>
{
  public:
    RandomColorFadingModel( String Title
                          , unsigned long Duration)
                          : ModelWithNewValueNotification<CRGB>(Title)
                          , m_Duration(Duration){}
    virtual ~RandomColorFadingModel()
    {
      if(true == debugMemory) Serial << "Delete RandomColorFadingModel\n";  
    }
  private:
     //Model
    void UpdateValue();
    void SetupModel();
    bool CanRunModelTask();
    void RunModelTask();

    //This
    CRGB m_CurrentColor;
    CRGB m_StartColor;
    CRGB m_EndColor;
    unsigned long m_Duration;
    unsigned long m_CurrentDuration;
    unsigned long m_CurrentTime;
    unsigned long m_StartTime;
    void StartFadingNextColor();
};

class RainbowColorModel: public ModelWithNewValueNotification<CRGB>
{
  public:
    RainbowColorModel( String Title
                     , unsigned int Numerator
                     , unsigned int Denominator)
                     : ModelWithNewValueNotification<CRGB>(Title)
                     , m_Numerator(Numerator)
                     , m_Denominator(Denominator){}
    virtual ~RainbowColorModel()
    {
      if(true == debugMemory) Serial << "Delete RainbowColorModel\n";  
    }
  private:
     unsigned int m_Numerator;
     unsigned int m_Denominator;
     CRGB m_Color = CRGB::Black;
     //Model
    void UpdateValue(){ SetCurrentValue(m_Color); }
    void SetupModel(){ }
    bool CanRunModelTask(){ return true; }
    void RunModelTask() { m_Color = GetColor(m_Numerator, m_Denominator); }

    //This
};

class ColorPowerModel: public DataModelWithNewValueNotification<CRGB>
                     , public ModelEventNotificationCallee<CRGB>
{
  public:
    ColorPowerModel( String Title
                   , CRGB Color
                   , StatisticalEngineModelInterface &StatisticalEngineModelInterface)
                   : DataModelWithNewValueNotification<CRGB>(Title, StatisticalEngineModelInterface)
                   , m_InputColor(Color){}
    virtual ~ColorPowerModel()
    {
      if(true == debugMemory) Serial << "Delete ColorPowerModel\n";  
    }
  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() { return true; }
  private:
     CRGB m_InputColor = CRGB::Black;
     CRGB m_OutputColor = CRGB::Black;
     //Model
    void UpdateValue()
    {
      SetCurrentValue( m_OutputColor );
    }
    void SetupModel(){ }
    bool CanRunModelTask(){ return true; }
    void RunModelTask() 
    {    
      float normalizedPower = m_StatisticalEngineModelInterface.GetNormalizedSoundPower();
      m_OutputColor.red = (byte)(m_InputColor.red * normalizedPower);
      m_OutputColor.green = (byte)(m_InputColor.green * normalizedPower);
      m_OutputColor.blue = (byte)(m_InputColor.blue * normalizedPower);
      if(true == debugModels) Serial << "ColorPowerModel normalizedPower: " << normalizedPower << " Resulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }
    
    //ModelEventNotificationCallee
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &Caller) { Caller.RegisterForNotification(*this); }
    void NewValueNotification(CRGB Value) { m_InputColor = Value; }
};


class SettableColorPowerModel: public ModelWithNewValueNotification<CRGB>
                             , public ModelEventNotificationCallee<CRGB>
                             , public ModelEventNotificationCallee<float>
{
  public:
    SettableColorPowerModel( String Title )
                           : ModelWithNewValueNotification<CRGB>(Title){}
    virtual ~SettableColorPowerModel()
    {
      if(true == debugMemory) Serial << "Delete SettableColorPowerModel\n";  
    }
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &Caller) { Caller.RegisterForNotification(*this); }
    void ConnectPowerModel(ModelEventNotificationCaller<float> &Caller) { Caller.RegisterForNotification(*this); }
  private:
     CRGB m_InputColor = CRGB::Black;
     CRGB m_OutputColor = CRGB::Black;
     float m_NormalizedPower = 0;
     //Model
    void UpdateValue(){
      SetCurrentValue( m_OutputColor );
    }
    void SetupModel(){ }
    bool CanRunModelTask(){ return true; }
    void RunModelTask() 
    {
      m_OutputColor.red = (byte)(m_InputColor.red * m_NormalizedPower);
      m_OutputColor.green = (byte)(m_InputColor.green * m_NormalizedPower);
      m_OutputColor.blue = (byte)(m_InputColor.blue * m_NormalizedPower);
      if(true == debugModels) Serial << "SettableColorPowerModel normalizedPower: " << m_NormalizedPower << " Resulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }
    
    //ModelEventNotificationCallee
    void NewValueNotification(CRGB Value) { m_InputColor = Value; }
    void NewValueNotification(float Value) { m_NormalizedPower = Value; }
};

class MaximumBandModel: public DataModelWithNewValueNotification<struct BandData>
{
  public:
    MaximumBandModel( String Title, unsigned int Depth, StatisticalEngineModelInterface &StatisticalEngineModelInterface )
                    : DataModelWithNewValueNotification<struct BandData>(Title, StatisticalEngineModelInterface)
                    , m_Depth(Depth){}
    virtual ~MaximumBandModel()
    {
      if(true == debugMemory) Serial << "Delete MaximumBandPowerModel\n";  
    }
  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() { return true; }
  private:
     BandData m_MaxBandData;
     unsigned int m_Depth = 0;
     //Model
    void UpdateValue(){
      SetCurrentValue( m_MaxBandData );
    }
    void SetupModel(){ }
    bool CanRunModelTask(){ return true; }
    void RunModelTask() 
    {
      unsigned int maxBandIndex = 0;
      float maxBandPowerValue = 0.0;
      unsigned int numBands = m_StatisticalEngineModelInterface.GetNumberOfBands();
      for(int b = 0; b < m_StatisticalEngineModelInterface.GetNumberOfBands(); ++b)
      {
        float power = m_StatisticalEngineModelInterface.GetBandAverage(b, m_Depth) / (float)ADDBITS;
        if(power > maxBandPowerValue)
        {
          maxBandPowerValue = power;
          maxBandIndex = b;
        }
      }
      m_MaxBandData.Power = maxBandPowerValue;
      m_MaxBandData.Band = maxBandIndex;
      m_MaxBandData.Color = GetColor(maxBandIndex, numBands-1); 
    }
};

class BandDataColorModel: public ModelWithNewValueNotification<CRGB>
                        , public ModelEventNotificationCallee<BandData>
{
  public:
    BandDataColorModel( String Title )
                      : ModelWithNewValueNotification<CRGB>(Title){}
    virtual ~BandDataColorModel()
    {
      if(true == debugMemory) Serial << "Delete BandDataColorModel\n";  
    }
    void ConnectBandDataModel(ModelEventNotificationCaller<BandData> &Caller) { Caller.RegisterForNotification(*this); }
  private:
    CRGB m_InputColor = CRGB::Black;
    CRGB m_OutputColor = CRGB::Black;
    float m_NormalizedPower = 0;
    
    //Model
    void UpdateValue(){
      SetCurrentValue( m_OutputColor );
    }
    void SetupModel(){ }
    bool CanRunModelTask(){ return true; }
    void RunModelTask() 
    {
      if(m_NormalizedPower > 1.0) m_NormalizedPower = 1.0;
      m_OutputColor.red = (byte)(m_InputColor.red * m_NormalizedPower);
      m_OutputColor.green = (byte)(m_InputColor.green * m_NormalizedPower);
      m_OutputColor.blue = (byte)(m_InputColor.blue * m_NormalizedPower);
      if(true == debugModels) Serial << "SettableColorPowerModel normalizedPower: " << m_NormalizedPower << " Resulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }
    
    //ModelEventNotificationCallee
    void NewValueNotification(BandData Value) 
    { 
      m_InputColor = Value.Color;
      m_NormalizedPower = Value.Power;
    }
};
#endif
