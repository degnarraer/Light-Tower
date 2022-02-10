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

enum Transciever_T
{
	Transciever_None,
	Transciever_TX,
	Transciever_RX,
	Transciever_TXRX
};

enum DataType_t
{
  DataType_Int8_t,
  DataType_Int16_t,
  DataType_Int32_t,
  DataType_Uint8_t,
  DataType_Uint16_t,
  DataType_Uint32_t,
  DataType_String,
  DataType_Float,
  DataType_ProcessedSoundData_t,
  DataType_MaxBinSoundData_t,
  DataType_Undef,
};

static const char* DataTypeStrings[] =
{
  "Int8_t",
  "Int16_t",
  "Int32_t",
  "Uint8_t",
  "Uint16_t",
  "Uint32_t",
  "String",
  "Float",
  "ProcessedSoundData_t",
  "MaxBinSoundData_t",
  "Undefined"
};

struct DataItemConfig_t
{
  String Name;
  DataType_t DataType;
  size_t Count;
  Transciever_T TransceiverConfig;
  size_t QueueCount;
};

struct DataItem_t
{
  String Name;
  QueueHandle_t QueueHandle_RX = NULL;
  QueueHandle_t QueueHandle_TX = NULL;
  Transciever_T TransceiverConfig;
  DataType_t DataType;
  size_t Count = 0;
  size_t TotalByteCount = 0;
  void* DataBuffer;
};

struct ProcessedSoundData_t
{
	float NormalizedPower;
	float PowerDB;
	int32_t Minimum;
	int32_t Maximum;
};

struct MaxBinSoundData_t
{
	float MaxBinNormalizedPower;
	int16_t MaxBinIndex;
	int16_t TotalBins;
};

#endif
