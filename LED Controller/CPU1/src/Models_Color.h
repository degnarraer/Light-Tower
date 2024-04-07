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

#ifndef Models_Color_H
#define Models_Color_H

#include "Models_Core.h"

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
    void UpdateValue()
    {
      SetCurrentValue(m_Color);
    }
    void SetupModel() { }
    bool CanRunModelTask()
    {
      return true;
    }
    void RunModelTask()
    {
      m_Color = GetRainbowColor(m_Numerator, m_Denominator);
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
    bool RequiresFFT() 
    {
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
    bool CanRunModelTask()
    {
      return true;
    }
    void RunModelTask()
    {
      float normalizedPower = m_StatisticalEngineModelInterface.GetNormalizedSoundPower();
      m_OutputColor = DimColor(m_InputColor, normalizedPower);
      if (true == debugModels) Serial << "ColorPowerModel normalizedPower: " << normalizedPower << " Resulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }

    //ModelEventNotificationCallee
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &Caller)
    {
      Caller.RegisterForNotification(*this, "");
    }
    void NewValueNotification(CRGB Value, String context)
    {
      m_InputColor = Value;
    }
};

class SettableColorPowerModel: public ModelWithNewValueNotification<CRGB>
                             , public ModelEventNotificationCallee<CRGB>
                             , public ModelEventNotificationCallee<float>
{
  public:
    SettableColorPowerModel( String Title ) : ModelWithNewValueNotification<CRGB>(Title)
    {
      if (true == debugMemory) Serial << "New: SettableColorPowerModel\n";
    }
    virtual ~SettableColorPowerModel()
    {
      if (true == debugMemory) Serial << "Delete: SettableColorPowerModel\n";
    }
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &Caller)
    {
      Caller.RegisterForNotification(*this, "");
    }
    void ConnectPowerModel(ModelEventNotificationCaller<float> &Caller)
    {
      Caller.RegisterForNotification(*this, "");
    }
  private:
    CRGB m_InputColor = CRGB::Black;
    CRGB m_OutputColor = CRGB::Black;
    float m_NormalizedPower = 0.0;
    //Model
    void UpdateValue() { SetCurrentValue( m_OutputColor ); }
    void SetupModel() { }
    bool CanRunModelTask(){return true;}
    void RunModelTask()
    {
      m_OutputColor = DimColor(m_InputColor, m_NormalizedPower); 
      if (true == debugModels) Serial << "SettableColorPowerModel normalizedPower: " << m_NormalizedPower<< " Input Color:  R:" << m_InputColor.red << " G:" << m_InputColor.green << " B:" << m_InputColor.blue << "\tResulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }

    //ModelEventNotificationCallee
    void NewValueNotification(CRGB Value, String context) 
    {
      m_InputColor = Value;
    }
    void NewValueNotification(float Value, String context)
    {
        m_NormalizedPower = Value;
        if(m_NormalizedPower > 1.0) m_NormalizedPower = 1.0; 
    }
};

class BandDataColorModel: public ModelWithNewValueNotification<CRGB>
                        , public ModelEventNotificationCallee<BandData>
{
  public:
    BandDataColorModel( String Title ): ModelWithNewValueNotification<CRGB>(Title)
    {
      if (true == debugMemory) Serial << "New: BandDataColorModel\n";
    }
    virtual ~BandDataColorModel()
    {
      if (true == debugMemory) Serial << "Delete: BandDataColorModel\n";
    }
    void ConnectBandDataModel(ModelEventNotificationCaller<BandData> &Caller)
    {
      Caller.RegisterForNotification(*this, "BandData");
    }
  private:
    CRGB m_InputColor = CRGB::Black;
    CRGB m_OutputColor = CRGB::Black;
    float m_NormalizedPower = 0;

    //Model
    void UpdateValue()
    {
      SetCurrentValue( m_OutputColor );
    }
    void SetupModel() { }
    bool CanRunModelTask()
    {
      return true;
    }
    void RunModelTask()
    {
      m_OutputColor = DimColor(m_InputColor, m_NormalizedPower);
      if (true == debugModels) Serial << "SettableColorPowerModel normalizedPower: " << m_NormalizedPower << " Resulting Color:  R:" << m_OutputColor.red << " G:" << m_OutputColor.green << " B:" << m_OutputColor.blue << " \n";
    }

    //ModelEventNotificationCallee
    void NewValueNotification(BandData Value, String context)
    {
      if(context.equals("BandData"))
      {
        m_InputColor = Value.Color;
        m_NormalizedPower = Value.Power;
        if(m_NormalizedPower > 1.0) m_NormalizedPower = 1.0; 
      }
    }
};

#endif
