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

#ifndef EventSystem_H
#define EventSystem_H

#define EVENT_SYSTEM_DEBUG false
#define EVENT_SYSTEM_REGISTRATION_DEBUG false

#include <LinkedList.h>
#include "Streaming.h"

class EventSystemCallee
{
  public:
    virtual void EventSystemNotification(String context) = 0;
};

class EventSystemCaller
{
  public:
    bool RegisterForEventNotification(EventSystemCallee *callee, String context)
    {
      if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Try Registerring for Event: " << context << "\n";
      CallerInterfaceData cid;
      cid.Callee = callee;
      cid.Context = context;
      bool found = false;
      for (int i = 0; i < m_MyCalleesWithContext.size(); ++i)
      {
        if (m_MyCalleesWithContext.get(i).Callee == callee && m_MyCalleesWithContext.get(i).Context == context)
        {
          found = true;
        }
      }
      if(false == found)
      {
        m_MyCalleesWithContext.add(cid);
        if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Registered for Event: " << context << "\n";
        return true;
      }
      return false;
    }
    bool DeRegisterForEventNotification(EventSystemCallee *callee, String context)
    {
      CallerInterfaceData cid;
      cid.Callee = callee;
      cid.Context = context;
      if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Try DeRegistering for Event: " << context << "\n";
      for (int i = 0; i < m_MyCalleesWithContext.size(); ++i)
      {
        if (m_MyCalleesWithContext.get(i) == cid)
        {
          m_MyCalleesWithContext.remove(i);
          if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "DeRegistered for Event: " << context << "\n";
          return true;
        }
      }
      return false;
    }
    void SendNotificationToCallees(String context)
    {
      if(true == EVENT_SYSTEM_DEBUG) Serial << "Try Sending Event Notification: " << context << "\n";
      for (int i = 0; i < m_MyCalleesWithContext.size(); ++i)
      {
        if(context == m_MyCalleesWithContext.get(i).Context)
        {
          if(true == EVENT_SYSTEM_DEBUG) Serial << "Sent Event Notification: " << context << "\n";
          m_MyCalleesWithContext.get(i).Callee->EventSystemNotification(context);
        }
      }
    }
  protected:
    bool ResisterNotificationContext(String notificationContext)
    {
      if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Try Registering Notification: " << notificationContext << "\n";
      bool found = false;
      for (int i = 0; i < m_MyNotificationContexts.size(); ++i)
      {
        if (m_MyNotificationContexts.get(i) == notificationContext)
        {
          if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Notification Registration Failed: " << notificationContext << "\n";
          found = true;
        }
      }
      if(false == found)
      {
        if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Notification Registration Success: " << notificationContext << "\n";
        m_MyNotificationContexts.add(notificationContext);
        return true;
      }
      return false;
    }
    bool DeResisterNotificationContext(String notificationContext)
    {
      if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Try DeRegistering Notification: " << notificationContext << "\n";
      for (int i = 0; i < m_MyNotificationContexts.size(); ++i)
      {
        if (m_MyNotificationContexts.get(i) == notificationContext)
        {
          if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Notification DeRegistration Success: " << notificationContext << "\n";
          m_MyNotificationContexts.remove(i);
          return true;
        }
      }
      if(true == EVENT_SYSTEM_REGISTRATION_DEBUG) Serial << "Notification DeRegistration Fail: " << notificationContext << "\n";
      return false;
    }
  private:
    struct CallerInterfaceData
    {
      EventSystemCallee* Callee;
      String Context;
      bool operator==(const CallerInterfaceData& cid)
      {
        return (true == ((cid.Callee == Callee) && (cid.Context == Context))) ? true : false;
      }
    };
    LinkedList<CallerInterfaceData> m_MyCalleesWithContext = LinkedList<CallerInterfaceData>();
    LinkedList<String> m_MyNotificationContexts = LinkedList<String>();
};

#endif
