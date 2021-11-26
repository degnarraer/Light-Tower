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
#define DEBUG_DATA_MANAGER false

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
    }
    virtual ~DataManager(){}

    void SetMicrophoneSoundBufferMemorySize(size_t BitsPerSample, size_t Samples, size_t Channels)
    {
      DataManagerConfig configer;
      DataItemConfig_t *DataItems = configer.GetDataManagerConfig();
      int count = configer.GetCount();
      if(true == DEBUG_DATA_MANAGER) Serial << "Setting Up Memory for " << count << " Data Items\n";
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
    template <class T>
    bool SetValue(String Name, T *Value, size_t Count)
    {
      for(int i = 0; i < m_DataItems.size(); ++i)
      {
        if(m_DataItems.get(i)->Name == Name && m_DataItems.get(i)->Count == Count)
        {
          if(true == DEBUG_DATA_MANAGER) Serial << "Set value.\n";
          memcpy(m_DataItems.get(i)->Object, Value, m_DataItems.get(i)->Count*sizeof(T));
          SendNotificationToCallees(m_DataItems.get(i)->Name);
          return true;
        }
      }
      return false;
    }
    
    template <class T>
    T* GetValue(String Name, size_t Count)
    {
      for(int i = 0; i < m_DataItems.size(); ++i)
      {
        if(true == DEBUG_DATA_MANAGER) Serial << "Get value.\n";
        if(m_DataItems.get(i)->Name == Name && m_DataItems.get(i)->Count == Count)
        {
          return m_DataItems.get(i)->Object;
        }
      }
    }
    template <class T>
    T GetValueAtIndex(String Name, size_t Count, int index)
    {
      for(int i = 0; i < m_DataItems.size(); ++i)
      {
        if(true == DEBUG_DATA_MANAGER) Serial << "Get value at index " << index << "\n";
        if(m_DataItems.get(i)->Name == Name && m_DataItems.get(i)->Count == Count)
        {
          T* Object = (T*)(m_DataItems.get(i)->Object);
          return Object[index];
        }
      }
    }
  private:
    LinkedList<DataItem_t *> m_DataItems = LinkedList<DataItem_t *>();
    
    template <class  T>
    bool AddDataItem(DataItemConfig_t Item)
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
          if(true == DEBUG_DATA_MANAGER) Serial << "Memory Allocation Failed"; 
          return false; exit(0); 
        } 
        else 
        { 
          if(true == DEBUG_DATA_MANAGER) Serial << "Memory Allocation Successful\n";
        }
        *DataItem = { Item.Name, Item.DataType, Item.Count, Object };
        m_DataItems.add(DataItem);
        return true;
      }
      return false;
    }
};


#endif
