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
    CRGB GetColor(unsigned int numerator, unsigned int denominator);
    CRGB GetRandomNonGrayColor();
    CRGB FadeColor(CRGB color, float scalar);
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
    MaxBinSoundData_t GetMaxBinSoundData();
    MaxBinSoundData_t GetMaxBinRightSoundData();
    MaxBinSoundData_t GetMaxBinLeftSoundData();
    
    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState) {}
    
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

class SoundPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    SoundPowerModel( String Title
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
      ++m_CircularBufferIndex;
    }
};

class BandPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    BandPowerModel(String Title, unsigned int Band, StatisticalEngineModelInterface &StatisticalEngineModelInterface): DataModelWithNewValueNotification<float>(Title, StatisticalEngineModelInterface)
      , m_Band(Band)
    {
      if (true == debugMemory) Serial << "New: BandPowerModel\n";
    }
    virtual ~BandPowerModel()
    {
      if (true == debugMemory) Serial << "Delete: BandPowerModel\n";
    }

    //Model
    void UpdateValue()
    {
      float value = (m_StatisticalEngineModelInterface.GetBandAverage(m_Band, 1));
      if (true == debugModels) Serial << "BandPowerModel value: " << value << " for band: " << m_Band << "\n";
      SetCurrentValue( value );
    }
  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() {
      return true;
    }
  private:
    //Model
    unsigned int m_Band = 0;
    void SetupModel() {}
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask() {}
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
      , m_TotalBands(totalBands)
    {
      if (true == debugMemory) Serial << "New: ReducedBandsBandPowerModel\n";
    }
    virtual ~ReducedBandsBandPowerModel()
    {
      if (true == debugMemory) Serial << "Delete: ReducedBandsBandPowerModel\n";
    }

    //Model
    void UpdateValue()
    {
      float value = (m_StatisticalEngineModelInterface.GetBandAverageForABandOutOfNBands(m_Band, m_Depth, m_TotalBands));
      if (true == debugModels) Serial << "ReducedBandsBandPowerModel value: " << value << " for band: " << m_Band << " of " << m_TotalBands << " bands\n";
      SetCurrentValue( value );
    }
  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() {
      return true;
    }
  private:
    //Model
    unsigned int m_Band = 0;
    unsigned int m_Depth = 0;
    unsigned int m_TotalBands = 0;
    void SetupModel() {}
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask() {}
};

class RandomColorFadingModel: public ModelWithNewValueNotification<CRGB>
{
  public:
    RandomColorFadingModel( String Title
                          , unsigned long Duration)
    : ModelWithNewValueNotification<CRGB>(Title)
    , m_Duration(Duration)
    {
      if (true == debugMemory) Serial << "New: RandomColorFadingModel\n";
    }
    virtual ~RandomColorFadingModel()
    {
      if (true == debugMemory) Serial << "Delete: RandomColorFadingModel\n";
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

class ColorFadingModel: public ModelWithNewValueNotification<CRGB>
  , public ModelEventNotificationCallee<CRGB>
  , public ModelEventNotificationCallee<BandData>
{
  public:
    ColorFadingModel( String Title
                      , unsigned long Duration
                      , unsigned long minimumUpdateTime)
      : ModelWithNewValueNotification<CRGB>(Title)
      , m_Duration(Duration)
      , m_MinimumUpdateTime(minimumUpdateTime)
    {
      if (true == debugMemory) Serial << "New: ColorFadingModel\n";
    }
    virtual ~ColorFadingModel()
    {
      if (true == debugMemory) Serial << "Delete: ColorFadingModel\n";
    }

    void ConnectBandDataModel(ModelEventNotificationCaller<BandData> &Caller) {
      Caller.RegisterForNotification(*this, "");
    }
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &Caller) {
      Caller.RegisterForNotification(*this, "");
    }

  private:
    //Model
    void UpdateValue();
    void SetupModel();
    bool CanRunModelTask();
    void RunModelTask();

    //ModelEventNotificationCallee
    void NewValueNotification(CRGB value, String context);
    void NewValueNotification(BandData value, String context);

    //This
    CRGB m_CurrentColor;
    CRGB m_StartColor;
    CRGB m_EndColor;
    unsigned long m_Duration;
    unsigned long m_CurrentDuration;
    unsigned long m_CurrentTime;
    unsigned long m_StartTime;
    unsigned long m_MinimumUpdateTime;
};

class RainbowColorModel: public ModelWithNewValueNotification<CRGB>
{
  public:
    RainbowColorModel( String Title
                     , unsigned int Numerator
                     , unsigned int Denominator)
      : ModelWithNewValueNotification<CRGB>(Title)
      , m_Numerator(Numerator)
      , m_Denominator(Denominator)
    {
      if (true == debugMemory) Serial << "New: RainbowColorModel\n";
    }
    virtual ~RainbowColorModel()
    {
      if (true == debugMemory) Serial << "Delete: RainbowColorModel\n";
    }
  private:
    unsigned int m_Numerator;
    unsigned int m_Denominator;
    CRGB m_Color = CRGB::Black;
    //Model
    void UpdateValue() {
      SetCurrentValue(m_Color);
    }
    void SetupModel() { }
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask() {
      m_Color = GetColor(m_Numerator, m_Denominator);
    }
};

class ColorPowerModel: public DataModelWithNewValueNotification<CRGB>
  , public ModelEventNotificationCallee<CRGB>
{
  public:
    ColorPowerModel( String Title
                     , CRGB Color
                     , StatisticalEngineModelInterface &StatisticalEngineModelInterface)
      : DataModelWithNewValueNotification<CRGB>(Title, StatisticalEngineModelInterface)
      , m_InputColor(Color)
    {
      if (true == debugMemory) Serial << "New: ColorPowerModel\n";
    }
    virtual ~ColorPowerModel()
    {
      if (true == debugMemory) Serial << "Delete: ColorPowerModel\n";
    }
  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() {
      return true;
    }
  private:
    CRGB m_InputColor = CRGB::Black;
    CRGB m_OutputColor = CRGB::Black;
    //Model
    void UpdateValue()
    {
      SetCurrentValue( m_OutputColor );
    }
    void SetupModel() { }
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask()
    {
      float normalizedPower = m_StatisticalEngineModelInterface.GetNormalizedSoundPower();
      m_OutputColor = FadeColor(m_InputColor, normalizedPower);
      if (true == debugModels) Serial << "ColorPowerModel normalizedPower: " << normalizedPower << " Resulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }

    //ModelEventNotificationCallee
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &Caller) {
      Caller.RegisterForNotification(*this, "");
    }
    void NewValueNotification(CRGB Value, String context) {
      m_InputColor = Value;
    }
};


class SettableColorPowerModel: public ModelWithNewValueNotification<CRGB>
  , public ModelEventNotificationCallee<CRGB>
  , public ModelEventNotificationCallee<float>
{
  public:
    SettableColorPowerModel( String Title )
      : ModelWithNewValueNotification<CRGB>(Title)
    {
      if (true == debugMemory) Serial << "New: SettableColorPowerModel\n";
    }
    virtual ~SettableColorPowerModel()
    {
      if (true == debugMemory) Serial << "Delete: SettableColorPowerModel\n";
    }
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &Caller) {
      Caller.RegisterForNotification(*this, "");
    }
    void ConnectPowerModel(ModelEventNotificationCaller<float> &Caller) {
      Caller.RegisterForNotification(*this, "");
    }
  private:
    CRGB m_InputColor = CRGB::Black;
    CRGB m_OutputColor = CRGB::Black;
    CHSV m_HSV = rgb2hsv_approximate(m_InputColor);
    float m_NormalizedPower = 0.0;
    //Model
    void UpdateValue() {
      SetCurrentValue( m_OutputColor );
    }
    void SetupModel() { }
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask()
    {
      CHSV hsv = m_HSV;
      hsv.value = (uint8_t ) round(((float)hsv.value * m_NormalizedPower));
      hsv2rgb_rainbow(hsv, m_OutputColor);
      if (true == debugModels) Serial << "SettableColorPowerModel normalizedPower: " << m_NormalizedPower<< " Input Color:  R:" << m_InputColor.red << " G:" << m_InputColor.green << " B:" << m_InputColor.blue << "\tResulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }

    //ModelEventNotificationCallee
    void NewValueNotification(CRGB Value, String context) {
      m_InputColor = Value;
      m_HSV = rgb2hsv_approximate(m_InputColor);
    }
    void NewValueNotification(float Value, String context) {
      m_NormalizedPower = Value;
    }
};

class MaximumBandModel: public DataModelWithNewValueNotification<struct BandData>
{
public:
    MaximumBandModel( String Title, unsigned int Depth, StatisticalEngineModelInterface &StatisticalEngineModelInterface )
      : DataModelWithNewValueNotification<struct BandData>(Title, StatisticalEngineModelInterface)
      , m_Depth(Depth)
    {
      if (true == debugMemory) Serial << "New: MaximumBandPowerModel\n";
    }
    virtual ~MaximumBandModel()
    {
      if (true == debugMemory) Serial << "Delete: MaximumBandPowerModel\n";
    }
protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() {
      return true;
    }
private:
    BandData m_MaxBandData;
    unsigned int m_Depth = 0;
    //Model
    void UpdateValue() {
      SetCurrentValue( m_MaxBandData );
    }
    void SetupModel() { }
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask()
    {
      unsigned int maxBandIndex = 0;
      float maxBandPowerValue = 0.0;
      unsigned int numBands = m_StatisticalEngineModelInterface.GetNumberOfBands();
      for (int b = 0; b < m_StatisticalEngineModelInterface.GetNumberOfBands(); ++b)
      {
        float power = m_StatisticalEngineModelInterface.GetBandAverage(b, m_Depth);
        if (power > maxBandPowerValue)
        {
          maxBandPowerValue = power;
          maxBandIndex = b;
        }
      }
      m_MaxBandData.Power = maxBandPowerValue;
      m_MaxBandData.Band = maxBandIndex;
      m_MaxBandData.Color = GetColor(maxBandIndex, numBands - 1);
    }
};

class MaximumBinModel: public DataModelWithNewValueNotification<struct BandData>
{
public:
    MaximumBinModel( String Title, unsigned int Depth, StatisticalEngineModelInterface &StatisticalEngineModelInterface )
      : DataModelWithNewValueNotification<struct BandData>(Title, StatisticalEngineModelInterface)
      , m_Depth(Depth)
    {
      if (true == debugMemory) Serial << "New: MaximumBandPowerModel\n";
    }
    virtual ~MaximumBinModel()
    {
      if (true == debugMemory) Serial << "Delete: MaximumBandPowerModel\n";
    }
protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() {
      return true;
    }
private:
    BandData m_MaxBandData;
    unsigned int m_Depth = 0;
    //Model
    void UpdateValue() {
      SetCurrentValue( m_MaxBandData );
    }
    void SetupModel() { }
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask()
    {
      MaxBinSoundData_t MaxBinSoundData = m_StatisticalEngineModelInterface.GetMaxBinSoundData();
      m_MaxBandData.Power = MaxBinSoundData.MaxBinNormalizedPower;
      m_MaxBandData.Band = MaxBinSoundData.MaxBinIndex;
      m_MaxBandData.Color = GetColor(MaxBinSoundData.MaxBinIndex, MaxBinSoundData.TotalBins);
    }
};
class BandDataColorModel: public ModelWithNewValueNotification<CRGB>
  , public ModelEventNotificationCallee<BandData>
{
  public:
    BandDataColorModel( String Title )
      : ModelWithNewValueNotification<CRGB>(Title)
    {
      if (true == debugMemory) Serial << "New: BandDataColorModel\n";
    }
    virtual ~BandDataColorModel()
    {
      if (true == debugMemory) Serial << "Delete: BandDataColorModel\n";
    }
    void ConnectBandDataModel(ModelEventNotificationCaller<BandData> &Caller) {
      Caller.RegisterForNotification(*this, "BandData");
    }
  private:
    CRGB m_InputColor = CRGB::Black;
    CRGB m_OutputColor = CRGB::Black;
    CHSV m_HSV = rgb2hsv_approximate(m_InputColor);
    float m_NormalizedPower = 0;

    //Model
    void UpdateValue() {
      SetCurrentValue( m_OutputColor );
    }
    void SetupModel() { }
    bool CanRunModelTask() {
      return true;
    }
    void RunModelTask()
    {
      CHSV hsv = m_HSV;
      hsv.value = (uint8_t ) round(((float)hsv.value * m_NormalizedPower));
      hsv2rgb_rainbow(hsv, m_OutputColor);
      if (true == debugModels) Serial << "SettableColorPowerModel normalizedPower: " << m_NormalizedPower << " Resulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }

    //ModelEventNotificationCallee
    void NewValueNotification(BandData Value, String context)
    {
      if(context.equals("BandData"))
      {
        m_InputColor = Value.Color;
        m_HSV = rgb2hsv_approximate(m_InputColor);
        m_NormalizedPower = Value.Power;
      }
    }
};

class GravitationalModel: public ModelWithNewValueNotification<Position>
  , public ModelEventNotificationCallee<Position>
{
  public:
    GravitationalModel( String title
                        , float gravitationalScaler
                        , float maxInitialVelocity)
      : ModelWithNewValueNotification<Position>(title)
      , m_GravitationalScaler(gravitationalScaler)
      , m_MaxInitialVelocity(maxInitialVelocity)
    {
      if (true == debugMemory) Serial << "New: GravitationalModel\n";
    }
    virtual ~GravitationalModel()
    {
      if (true == debugMemory) Serial << "Delete: GravitationalModel\n";
    }

    void ConnectPositionModel(ModelEventNotificationCaller<Position> &Caller) { Caller.RegisterForNotification(*this, "Position"); }
    void ConnectXPositionModel(ModelEventNotificationCaller<Position> &Caller) { Caller.RegisterForNotification(*this, "X"); }
    void ConnectYPositionModel(ModelEventNotificationCaller<Position> &Caller) { Caller.RegisterForNotification(*this, "Y"); }

  private:
    unsigned long m_StartTime;
    unsigned long m_CurrentTime;
    unsigned long m_PreviousLoopTime;
    float m_Duration = 0.0;
    float m_GravitationalScaler = 1.0;
    float m_InitialVelocity = 0.0;
    float m_MaxInitialVelocity = 0.0;
    Position m_Position = { 0, 0 };
    Position m_LowerPositionLimit = { 0, 0 };
    Position m_StartingPosition = { 0, 0 };

    //Model
    void UpdateValue() 
    {
      SetCurrentValue( m_Position );
    }
    void SetupModel()
    {
      m_StartTime = millis();
      m_CurrentTime = m_StartTime;
      m_PreviousLoopTime = m_StartTime;
    }
    bool CanRunModelTask()
    {
      return true;
    }
    void RunModelTask()
    {
      m_CurrentTime = millis();
      m_Duration = (m_CurrentTime - m_StartTime) / 1000.0;
      m_Position.Y = m_StartingPosition.Y + (GetGravitationalPosition(m_Duration, m_InitialVelocity, m_GravitationalScaler) * LEDS_PER_METER);
      if (m_Position.Y < m_LowerPositionLimit.Y) StartExperiment();
      if (true == debugModels || (true == debugGravitationalModel)) Serial << "GravitationalModel: \tPosition.Y " << m_Position.Y << "\tStart Position.Y " << m_StartingPosition.Y << "\n";
      m_PreviousLoopTime = m_CurrentTime;
    }

    //ModelEventNotificationCallee
    void NewValueNotification(Position value, String context)
    {
      if(context == "Position")
      {
        m_LowerPositionLimit = value;
      }
      else if (context == "X")
      {
        m_LowerPositionLimit.X = value.X;
      }
      else if(context == "Y")
      {
        m_LowerPositionLimit.Y = value.Y;
      }
    }
    void StartExperiment()
    {
      m_StartTime = millis();
      float dT = (m_CurrentTime - m_PreviousLoopTime) / 1000.0;
      int dY = m_LowerPositionLimit.Y - m_Position.Y;   //Delta Pixels
      float dM = (float)dY / (float)LEDS_PER_METER;  //Delta Meters
      if (dT == 0)
      {
        m_InitialVelocity = m_MaxInitialVelocity;
      }
      else
      {
        m_InitialVelocity = dM / dT;
      }
      if (m_InitialVelocity > m_MaxInitialVelocity) m_InitialVelocity = m_MaxInitialVelocity;
      m_Position.Y = m_LowerPositionLimit.Y;
      m_StartingPosition.Y = m_LowerPositionLimit.Y;
      if (true == debugModels || (true == debugGravitationalModel)) Serial << "GravitationalModel: \tdY: " << dY << "\tdM: " << dM <<  "\tdT: " << dT << "\tInitial Velocity: " << m_InitialVelocity << "\n";
    }
    float GetGravitationalPosition(float t, float initialVelocity, float gravitationalScaler)
    {
      //Returns distance an object falls in meters for t seconds
      float d = 0;
      const float g = -9.802 * gravitationalScaler;
      d = initialVelocity * t + ((0.5 * g) * (pow(t, 2)));
      return d;
    }
};

class BinPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    BinPowerModel( String title
                 , unsigned int startBin
                 , unsigned int endBin
                 , StatisticalEngineModelInterface &statisticalEngineModelInterface )
                 : DataModelWithNewValueNotification<float>(title, statisticalEngineModelInterface)
                 , m_StartBin(startBin)
                 , m_EndBin(endBin)
    {
      if (true == debugMemory) Serial << "New: BinPowerModel\n";
    }
    virtual ~BinPowerModel()
    {
      if (true == debugMemory) Serial << "Delete: BinPowerModel\n";
    }
  private:
    unsigned int m_StartBin = 0;
    unsigned int m_EndBin = 0;
    float m_Result;
    //Model
    bool RequiresFFT() { return true; }
    void UpdateValue() 
    {
      SetCurrentValue( m_Result );
    }
    void SetupModel()
    {
    }
    bool CanRunModelTask()
    {
      return true;
    }
    void RunModelTask()
    {
      m_Result = 0.0;
      for(int i = m_StartBin; i<= m_EndBin; ++i)
      {
        //TODO
        //m_Result += m_StatisticalEngineModelInterface.GetNormalizedBinValue(i);
      }
    }
};
#endif
