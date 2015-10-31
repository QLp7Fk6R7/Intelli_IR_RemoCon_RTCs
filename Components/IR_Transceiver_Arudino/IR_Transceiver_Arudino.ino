//========================================================
// Config
//========================================================

#define IR_IN                  (8)       /* Input      */
#define IR_OUT                 (12)      /* Output     */
 
#define TIMEOUT_RECV_NOSIGNAL  (50000)
#define TIMEOUT_RECV           (5000000)
#define TIMEOUT_SEND           (2000000)
#define BYTE_PER_CYCLE         (20)

//========================================================

#include <UART.h>
#include <RTno.h>

/**
 * This function is called at first.
 * conf._default.baudrate: baudrate of serial communication
 * exec_cxt.periodic.type: reserved but not used.
 */
void rtcconf(config_str& conf, exec_cxt_str& exec_cxt) {
  conf._default.connection_type = ConnectionTypeSerial1;
  conf._default.baudrate = 19200;
  exec_cxt.periodic.type = ProxySynchronousExecutionContext;
}

/** 
 * Declaration Division:
 *
 * DataPort and Data Buffer should be placed here.
 *
 * Currently, following 6 types are available.
 * TimedLong:
 * TimedDouble:
 * TimedFloat:
 * TimedLongSeq:
 * TimedDoubleSeq:
 * TimedFloatSeq:
 *
 * Please refer following comments. If you need to use some ports,
 * uncomment the line you want to declare.
 **/

TimedLongSeq InIRData;
InPort<TimedLongSeq> InIRDataIn("InIRData", InIRData);

TimedLongSeq OutIRData;
OutPort<TimedLongSeq> OutIRDataOut("OutIRData", OutIRData);

#define DATA_SIZE              (512)

#define STATE_IDLE 0
#define STATE_READ 1
#define STATE_WRITE 2

unsigned char g_State = STATE_IDLE;
unsigned short g_IRData[DATA_SIZE];
unsigned int g_IRData_index = 0;
unsigned int g_IRData_size = 0;

void sendSignal(const unsigned short* data, unsigned int data_size)
{
  unsigned short time;
  unsigned long count;
  unsigned long us;

  for(count = 0; count < data_size; count++){
      time = data[count];
      us = micros();
      do {
        digitalWrite(IR_OUT, !(count&1));
        delayMicroseconds(8);
        digitalWrite(IR_OUT, 0);
        delayMicroseconds(7);
      }while(long(us + time - micros()) > 0);
    }
}

unsigned long recvSignal(unsigned short* data, unsigned int data_size)
{
  unsigned char pre_value;
  unsigned char now_value;
  unsigned long now_us;
  unsigned long pre_us;
  unsigned long count;

  count = 0;
  pre_us = micros();
  while (1) {
    now_value = digitalRead(IR_IN);
    now_us = micros();
    if (now_value != HIGH) {
      break;
    }
    
    if((now_us - pre_us) > TIMEOUT_RECV){
      return count;
    }    
  }
  
  pre_value = now_value;
  pre_us = now_us;
  while (count < data_size) {
    now_value = digitalRead(IR_IN);
    now_us = micros();
    if(pre_value != now_value){
      data[count] = now_us - pre_us;
      count++;
      pre_value = now_value;
      pre_us = now_us;    
    }
    
    if((now_us - pre_us) > TIMEOUT_RECV_NOSIGNAL){
       break;
    }
  }
  
  return (count < data_size) ? count : data_size;
}


//////////////////////////////////////////
// on_initialize
//
// This function is called in the initialization
// sequence. The sequence is triggered by the
// PC. When the RTnoRTC is launched in the PC,
// then, this function is remotely called
// through the USB cable.
// In on_initialize, usually DataPorts are added.
//
//////////////////////////////////////////
int RTno::onInitialize() {
  addInPort(InIRDataIn);
  
  addOutPort(OutIRDataOut);
  
  pinMode(IR_IN, INPUT);
  pinMode(IR_OUT, OUTPUT);
     
  return RTC_OK; 
}

////////////////////////////////////////////
// on_activated
// This function is called when the RTnoRTC
// is activated. When the activation, the RTnoRTC
// sends message to call this function remotely.
// If this function is failed (return value 
// is RTC_ERROR), RTno will enter ERROR condition.
////////////////////////////////////////////
int RTno::onActivated() {  
  return RTC_OK; 
}

/////////////////////////////////////////////
// on_deactfivated
// This function is called when the RTnoRTC
// is deactivated.
/////////////////////////////////////////////
int RTno::onDeactivated()
{
  return RTC_OK;
}

void writeToRTC()
{
    unsigned int i;
    unsigned int len;

    if ((g_IRData_index+BYTE_PER_CYCLE) > g_IRData_size) {
      len = g_IRData_size - g_IRData_index;
      OutIRData.data.length(len+1);      
    } else {
      len = BYTE_PER_CYCLE;
      OutIRData.data.length(BYTE_PER_CYCLE);
    }
    for (i = 0; i < BYTE_PER_CYCLE; i++, g_IRData_index++) {
     if (g_IRData_index >= g_IRData_size) {
       OutIRData.data[i] = 0;
       g_State = STATE_IDLE;
       break;
      }
      OutIRData.data[i] = g_IRData[g_IRData_index];
    }
    OutIRDataOut.write();  
}

void readFromRTC()
{
    const unsigned int len = InIRData.data.length();
    unsigned int i;
    for (i = 0; i < len && g_IRData_index < DATA_SIZE; i++, g_IRData_index++) {
      if (InIRData.data[i] == 0) {
        break;
      }
      g_IRData[g_IRData_index] = InIRData.data[i];
    }
    
    if (i < len) {
      g_IRData_size = g_IRData_index;
      sendSignal(g_IRData, g_IRData_size);
      g_State = STATE_IDLE;
    }
}

//////////////////////////////////////////////
// This function is repeatedly called when the 
// RTno is in the ACTIVE condition.
// If this function is failed (return value is
// RTC_ERROR), RTno immediately enter into the 
// ERROR condition.r
//////////////////////////////////////////////
int RTno::onExecute() {
  if (g_State == STATE_WRITE) {
    writeToRTC();
  } else if (g_State == STATE_READ) {
    if(InIRDataIn.isNew()) {
      InIRDataIn.read();
      readFromRTC();
    }
  } else { // idle
    if(InIRDataIn.isNew()) {
      InIRDataIn.read();
      if (InIRData.data.length() == 0) { //receive
        g_IRData_size = recvSignal(g_IRData, DATA_SIZE);
        
        
        g_IRData_index = 0;
        g_State = STATE_WRITE;
        
        writeToRTC();
      } else {
        g_IRData_index = 0;
        g_State = STATE_READ;
        
        readFromRTC();
      }
    }
  }
  
  return RTC_OK;     
}


//////////////////////////////////////
// on_error
// This function is repeatedly called when
// the RTno is in the ERROR condition.
// The ERROR condition can be recovered,
// when the RTno is reset.
///////////////////////////////////////
int RTno::onError()
{
  return RTC_OK;
}

////////////////////////////////////////
// This function is called when 
// the RTno is reset. If on_reset is
// succeeded, the RTno will enter into
// the INACTIVE condition. If failed 
// (return value is RTC_ERROR), RTno
// will stay in ERROR condition.ec
///////////////////////////////////////
int RTno::onReset()
{
  return RTC_OK;
}


