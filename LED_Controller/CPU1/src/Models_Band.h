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
#include "Models_Core.h"

class BandPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    BandPowerModel( std::string Title
                  , unsigned int Band
                  , StatisticalEngineModelInterface &StatisticalEngineModelInterface )
                  : DataModelWithNewValueNotification<float>(Title, StatisticalEngineModelInterface)
                  , m_Band(Band)
    {
      if (true == debugMemory) Serial << "New: BandPowerModel\n";
    }
    virtual ~BandPowerModel(){if (true == debugMemory) Serial << "Delete: BandPowerModel\n";}

    //Model
    void UpdateValue()
    {
      float value = (m_StatisticalEngineModelInterface.GetBandAverage(m_Band, 1));
      if (true == debugModels) Serial << "BandPowerModel value: " << value << " for band: " << m_Band << "\n";
      SetCurrentValue( value );
    }
  protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() {return true;}
  private:
    //Model
    unsigned int m_Band = 0;
    void SetupModel() {}
    bool CanRunModelTask() {return true;}
    void RunModelTask() {}
};

class ReducedBandsBandPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    ReducedBandsBandPowerModel( std::string Title
                                , unsigned int band
                                , unsigned int depth
                                , unsigned int totalBands
                                , StatisticalEngineModelInterface &StatisticalEngineModelInterface )
                                : DataModelWithNewValueNotification<float>(Title, StatisticalEngineModelInterface)
                                , m_Band(band)
                                , m_Depth(depth)
                                , m_TotalBands(totalBands)
    {
      if (true == debugMemory) Serial << "New: ReducedBandsBandPowerModel\n";
    }
    virtual ~ReducedBandsBandPowerModel(){ if(true == debugMemory) Serial << "Delete: ReducedBandsBandPowerModel\n"; }

    //Model
    void UpdateValue()
    {
      float value = (m_StatisticalEngineModelInterface.GetBandAverageForABandOutOfNBands(m_Band, m_Depth, m_TotalBands));
      if (true == debugModels) Serial << "ReducedBandsBandPowerModel value: " << value << " for band: " << m_Band << " of " << m_TotalBands << " bands\n";
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
    void SetupModel() {}
    bool CanRunModelTask() {return true;}
    void RunModelTask() {}
};

class MaximumBandModel: public DataModelWithNewValueNotification<struct BandData>
{
public:
    MaximumBandModel( std::string Title
                    , unsigned int Depth
                    , StatisticalEngineModelInterface &StatisticalEngineModelInterface )
                    : DataModelWithNewValueNotification<struct BandData>(Title, StatisticalEngineModelInterface)
                    , m_Depth(Depth)
    {
      if (true == debugMemory) Serial << "New: MaximumBandModel\n";
    }
    virtual ~MaximumBandModel(){if (true == debugMemory) Serial << "Delete: MaximumBandModel\n"; }
protected:
    //StatisticalEngineModelInterfaceUsers
    bool RequiresFFT() {return true;}
private:
    BandData m_MaxBandData;
    unsigned int m_Depth = 0;
    //Model
    void UpdateValue()
    {
      SetCurrentValue( m_MaxBandData );
    }
    void SetupModel() { }
    bool CanRunModelTask() {return true;}
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
      m_MaxBandData.Color = GetRainbowColor(maxBandIndex, numBands - 1);
    }
};

class BinPowerModel: public DataModelWithNewValueNotification<float>
{
  public:
    BinPowerModel( std::string title
                 , unsigned int startBin
                 , unsigned int endBin
                 , StatisticalEngineModelInterface &statisticalEngineModelInterface )
                 : DataModelWithNewValueNotification<float>(title, statisticalEngineModelInterface)
                 , m_StartBin(startBin)
                 , m_EndBin(endBin)
    {
      if (true == debugMemory) Serial << "New: BinPowerModel\n";
    }
    virtual ~BinPowerModel(){ if (true == debugMemory) Serial << "Delete: BinPowerModel\n"; }
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
    bool CanRunModelTask(){ return true; }
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
