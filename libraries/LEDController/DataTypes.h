#ifndef DataTypes_H
#define DataTypes_H 

class NamedItem
{
  public:
	NamedItem(String Title): m_Title(Title){}
	virtual ~NamedItem(){}
    String GetTitle()
    { 
      return m_Title;
    }
  private:
    String m_Title;
};

enum DataType_t
{
  DataType_Int16_t,
  DataType_Int32_t,
  DataType_Uint16_t,
  DataType_Uint32_t,
  DataType_String,
};

static const char* DataTypeStrings[] =
{
  "Int16_t",
  "Int32_t",
  "Uint16_t",
  "Uint32_t",
  "String",
};

struct DataItemConfig_t
{
  String Name;
  DataType_t DataType;
  size_t Count;
};

struct DataItem_t
{
  String Name;
  DataType_t DataType;
  size_t Count;
  QueueHandle_t QueueHandle;
  void* Object;
};

#endif
