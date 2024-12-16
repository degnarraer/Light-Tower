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
class GravitationalModel: public ModelWithNewValueNotification<Position>
                        , public ModelEventNotificationCallee<Position>
{
  public:
    GravitationalModel( std::string title
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
