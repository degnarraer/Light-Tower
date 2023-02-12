#include "Models_Core.h"

void Model::Setup()
{
  SetupModel();
}
bool Model::CanRunMyScheduledTask()
{
  return CanRunModelTask();
}
void Model::RunMyScheduledTask()
{
  RunModelTask();
  UpdateValue();
}
CRGB Model::DimColor(CRGB color, float scalar)
{
  CHSV hsv = rgb2hsv_approximate(color);
  hsv.value *= scalar;
  CRGB rgb;
  hsv2rgb_spectrum(hsv, rgb);
  return rgb;
}
CRGB Model::GetRainbowColor(unsigned int numerator, unsigned int denominator)
{
  CHSV hsv = CHSV((uint8_t )floor(((float)numerator/(float)denominator*(float)255)), 255, 255);
  CRGB rgb;
  hsv2rgb_spectrum(hsv, rgb);
  return rgb;
}
CRGB Model::GetRandomNonGrayColor()
{ 
  CHSV hsv = CHSV((uint8_t )random8(), 255, 255);
  CRGB rgb;
  hsv2rgb_spectrum(hsv, rgb);
  return rgb;
}

bool BandData::operator == (const BandData& a)
{
  return (true == ((a.Power == Power) && (a.Band == Band) && (a.Color == Color))) ? true : false;
}
bool BandData::operator != (const BandData& a)
{
  return (true == ((a.Power == Power) && (a.Band == Band) && (a.Color == Color))) ? false : true;
}
Print& operator << (Print& os, const BandData& bd)
{
  os << bd.Band << "|" << bd.Power << "|" << bd.Color.red << "|" << bd.Color.green << "|" << bd.Color.blue;
  return os;
}



bool Position::operator==(const Position& a)
{
  return (true == ((a.X == X) && (a.Y == Y))) ? true : false;
}
bool Position::operator!=(const Position& a)
{
  return (true == ((a.X == X) && (a.Y == Y))) ? false : true;
}
Print& operator<<(Print& os, const Position& pos)
{
  os << pos.X << "|" << pos.Y;
  return os;
}



bool Size::operator==(const Size& a)
{
  return (true == ((a.W == W) && (a.H == H))) ? true : false;
}
bool Size::operator!=(const Size& a)
{
  return (true == ((a.W == W) && (a.H == H))) ? false : true;
}



bool Coordinates::operator==(const Coordinates& a)
{
  return (true == ((a.Position.X == Position.X) && (a.Position.Y == Position.Y) && (a.Size.W == Size.W) && (a.Size.H == Size.H))) ? true : false;
}
bool Coordinates::operator!=(const Coordinates& a)
{
  return (true == ((a.Position.X == Position.X) && (a.Position.Y == Position.Y) && (a.Size.W == Size.W) && (a.Size.H == Size.H))) ? false : true;
}


//Task Interface
void StatisticalEngineModelInterface::Setup()
{ 
  m_StatisticalEngine.RegisterForSoundStateChangeNotification(this);
  AddTask(m_StatisticalEngine);
}
bool StatisticalEngineModelInterface::CanRunMyScheduledTask()
{ 
  m_StatisticalEngine.SetProcessFFTStatus(UsersRequireFFT());
  return true; 
}
void StatisticalEngineModelInterface::RunMyScheduledTask()
{
}


//StatisticalEngine Getters
unsigned int StatisticalEngineModelInterface::GetNumberOfBands()
{
  return m_StatisticalEngine.GetNumberOfBands();
}

float StatisticalEngineModelInterface::GetNormalizedSoundPower()
{
  return m_StatisticalEngine.GetNormalizedSoundPower();
}

float StatisticalEngineModelInterface::GetBandAverage(unsigned int band, unsigned int depth)
{
  return m_StatisticalEngine.GetBandAverage(band, depth);
}

float StatisticalEngineModelInterface::GetBandAverageForABandOutOfNBands(unsigned int band, unsigned int depth, unsigned int totalBands)
{
  return m_StatisticalEngine.GetBandAverageForABandOutOfNBands(band, depth, totalBands);
}

float StatisticalEngineModelInterface::GetBandValue(unsigned int band, unsigned int depth)
{
  return m_StatisticalEngine.GetBandValue(band, depth);
}

MaxBandSoundData_t StatisticalEngineModelInterface::GetMaxBandSoundData()
{ 
  return m_StatisticalEngine.GetMaxBandSoundData(); 
}
MaxBandSoundData_t StatisticalEngineModelInterface::GetMaxBinRightSoundData()
{ 
  return m_StatisticalEngine.GetMaxBinRightSoundData(); 
}
MaxBandSoundData_t StatisticalEngineModelInterface::GetMaxBinLeftSoundData()
{ 
  return m_StatisticalEngine.GetMaxBinLeftSoundData();
}


void StatisticalEngineModelInterfaceUserTracker::RegisterAsUser(StatisticalEngineModelInterfaceUsers &user)
{
  m_MyUsers.add(&user);
}

void StatisticalEngineModelInterfaceUserTracker::DeRegisterAsUser(StatisticalEngineModelInterfaceUsers &user)
{
  for (int i = 0; i < m_MyUsers.size(); ++i)
  {
    if (m_MyUsers.get(i) == &user)
    {
      m_MyUsers.remove(i);
      break;
    }
  }
}

bool StatisticalEngineModelInterfaceUserTracker::UsersRequireFFT()
{
  bool result = false;
  for (int u = 0; u < m_MyUsers.size(); ++u)
  {
    if (true == m_MyUsers.get(u)->RequiresFFT())
    {
      result = true;
      break;
    }
  }
  return result;
}

void DataModel::Setup()
{
  SetupModel();
}

bool DataModel::CanRunMyScheduledTask()
{
  return CanRunModelTask();
}

void DataModel::RunMyScheduledTask()
{
  RunModelTask();
  UpdateValue();
}
