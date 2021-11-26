
#ifndef DataManagerTypes_H
#define DataManagerTypes_H 


enum DataType_t
{
  DataType_Int16_t,
  DataType_Int32_t,
  DataType_Uint32_t,
  DataType_String,
};

static const char* DataTypeStrings[] =
{
  "DataType_Int16_t",
  "DataType_Int32_t",
  "DataType_Uint32_t",
  "DataType_String",
};

struct DataItemConfig_t
{
  String Name;
  DataType_t DataType;
  int32_t Count;
};

struct DataItem_t
{
  String Name;
  DataType_t DataType;
  int32_t Count;
  void* Object;
};

#endif
