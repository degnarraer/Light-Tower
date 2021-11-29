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

#ifndef DataManager_H
#define DataManager_H
#define DATA_MANAGER_DEBUG false
#define DATA_MANAGER_LOCK_DEBUG false


#include "EventSystem.h"
#include <LinkedList.h>
#include "DataManagerTypes.h"
#include "DataManager_Config.h"

class DataManager: public EventSystemCaller
                 , DataManagerConfig
{
  public:
    DataManager()
    {
      DataManagerLock = xSemaphoreCreateMutex();
      xSemaphoreGive(DataManagerLock);
    }
    virtual ~DataManager()
    {
      for(int i = 0; i < m_DataItems.size(); ++i)
      {
        delete m_DataItems.get(i)->Object;
        m_DataItems.remove(i);
      }
    }
    
    template <class T>
    bool SetValue(String Name, T *Value, size_t Count)
    {
      if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "Try Set Value\n";
      if(NULL == DataManagerLock) {if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "NULL Semaphore1!\n";}
      else
      {
        if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "Lock Semaphore1\n";
        if(xSemaphoreTakeRecursive(DataManagerLock, DataManagerLockTicWaitTime) == pdFALSE){Serial << "Error Getting Lock1\n";} 
        else
        {
          bool found = false;
          DataItem_t *foundItem;
          for(int i = 0; i < m_DataItems.size(); ++i)
          {
            if(m_DataItems.get(i)->Name == Name && m_DataItems.get(i)->Count == Count)
            {
              found = true;
              foundItem = m_DataItems.get(i);
              memcpy(foundItem->Object, Value, foundItem->Count*sizeof(T));
              if(true == DATA_MANAGER_DEBUG) Serial << "Set value.\n";
              break;
            }
          }
          if(true == DATA_MANAGER_LOCK_DEBUG){ Serial << "UnLock Semaphore1\n";}
          xSemaphoreGiveRecursive(DataManagerLock);
          if(true == found)
          {
            SendNotificationToCallees(foundItem->Name);
          }
        }
      }
    }
    
    template <class T>
    T* GetValue(String Name, size_t Count)
    {
      if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "Try Get Value\n";
      if(NULL == DataManagerLock) {if(true == DATA_MANAGER_LOCK_DEBUG)  Serial << "NULL Semaphore2!\n";}
      else
      {
        if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "Lock Semaphore2\n";
        if(xSemaphoreTakeRecursive(DataManagerLock, DataManagerLockTicWaitTime) == pdFALSE){Serial << "Error Getting Lock2\n";} 
        else
        {
          for(int i = 0; i < m_DataItems.size(); ++i)
          {
            if(true == DATA_MANAGER_DEBUG) Serial << "Get value.\n";
            if(m_DataItems.get(i)->Name == Name && m_DataItems.get(i)->Count == Count)
            {
              return (T*)(m_DataItems.get(i)->Object);
            }
          }
        }
        if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "UnLock Semaphore2\n";
        xSemaphoreGiveRecursive(DataManagerLock);
      }
    }
    void InitializeDataManager()
    {
      if(true == DATA_MANAGER_DEBUG) Serial << "Try Initialize Data Manager\n";
      if(NULL == DataManagerLock) {if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "NULL Semaphore4!\n";}
      else
      {
        if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "Lock Semaphore4\n";
        if(xSemaphoreTakeRecursive(DataManagerLock, DataManagerLockTicWaitTime) == pdFALSE){Serial << "Error Getting Lock4\n";} 
        else
        {
          DataManagerConfig configer;
          DataItemConfig_t *DataItems = configer.GetDataManagerConfig();
          int count = configer.GetCount();
          if(true == DATA_MANAGER_DEBUG) Serial << "Setting Up Memory for " << count << " Data Items\n";
          for( int i = 0; i < count; ++i)
          {
            switch(DataItems[i].DataType)
            {
              case DataType_Int32_t:
              {
                AddDataItem<int32_t>(DataItems[i]);
                break;
              }
              case DataType_Uint32_t:
              {
                AddDataItem<uint32_t>(DataItems[i]);
                break;
              }
              case DataType_String:
              {
                AddDataItem<String>(DataItems[i]);
                break;
              }
              default:
                break;
            }
            ResisterNotificationContext(DataItems[i].Name);
          }
        }
        if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "UnLock Semaphore4\n";
        xSemaphoreGiveRecursive(DataManagerLock);
      }
    }
  private:
    SemaphoreHandle_t DataManagerLock;
    const int DataManagerLockTicWaitTime = portMAX_DELAY;
    LinkedList<DataItem_t *> m_DataItems = LinkedList<DataItem_t *>();
    
    template <class  T>
    bool AddDataItem(DataItemConfig_t Item)
    {
      if(true == DATA_MANAGER_DEBUG) Serial << "Try Adding Data Item\n";
      if(NULL == DataManagerLock) {if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "NULL Semaphore5!\n";}
      else
      {
        if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "Lock Semaphore5\n";
        if(xSemaphoreTakeRecursive(DataManagerLock, DataManagerLockTicWaitTime) == pdFALSE){Serial << "Error Getting Lock5\n";} 
        else
        {
          bool found = false;
          for(int i = 0; i < m_DataItems.size(); ++i)
          {
            if(m_DataItems.get(i)->Name == Item.Name && m_DataItems.get(i)->Count == Item.Count)
            { 
              found = true;
              break; 
            }
          }
          if(false == found)
          {
            DataItem_t *DataItem = new DataItem_t;
            T* Object;
            size_t byteCount = Item.Count * sizeof(T);
            Serial << "Setting Up Memory for " << DataTypeStrings[Item.DataType] << " with a count of " << byteCount << "\n";
            Object = (T*)malloc(byteCount);
            if (!Object) 
            { 
              if(true == DATA_MANAGER_DEBUG) Serial << "Memory Allocation Failed";
              if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "UnLock Semaphore5\n";
              xSemaphoreGiveRecursive(DataManagerLock);
              return false;
            } 
            else 
            { 
              if(true == DATA_MANAGER_DEBUG) Serial << "Memory Allocation Successful\n";
            }
            *DataItem = { Item.Name, Item.DataType, Item.Count, Object };
            m_DataItems.add(DataItem);
            if(true == DATA_MANAGER_DEBUG) Serial << "UnLock Semaphore5\n";
            xSemaphoreGiveRecursive(DataManagerLock);
            return true;
          }
        }
        if(true == DATA_MANAGER_LOCK_DEBUG) Serial << "UnLock Semaphore5\n";
        xSemaphoreGiveRecursive(DataManagerLock);
        return false; 
      }
    }
};


#endif
