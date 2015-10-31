// -*- C++ -*-
/*!
 * @file  IR_Transceiver_RaspberryPi.cpp
 * @brief trasmit or receive IR signal for RaspberryPi
 * @date $Date$
 *
 * $Id$
 */

#include "IR_Transceiver_RaspberryPi.h"

// Module specification
// <rtc-template block="module_spec">
static const char* ir_transceiver_raspberrypi_spec[] =
  {
    "implementation_id", "IR_Transceiver_RaspberryPi",
    "type_name",         "IR_Transceiver_RaspberryPi",
    "description",       "trasmit or receive IR signal for RaspberryPi",
    "version",           "1.0.0",
    "vendor",            "VenderName",
    "category",          "Category",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.IR_OUT_PIN", "23",
    "conf.default.IR_IN_PIN", "22",
    // Widget
    "conf.__widget__.IR_OUT_PIN", "text",
    "conf.__widget__.IR_IN_PIN", "text",
    // Constraints
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
IR_Transceiver_RaspberryPi::IR_Transceiver_RaspberryPi(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_InIRDataIn("InIRData", m_InIRData),
    m_OutIRDataOut("OutIRData", m_OutIRData)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
IR_Transceiver_RaspberryPi::~IR_Transceiver_RaspberryPi()
{
}



RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("InIRData", m_InIRDataIn);
  
  // Set OutPort buffer
  addOutPort("OutIRData", m_OutIRDataOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("IR_OUT_PIN", m_IR_OUT_PIN, "23");
  bindParameter("IR_IN_PIN", m_IR_IN_PIN, "22");
  // </rtc-template>

	
  if (wiringPiSetupGpio() == -1) {
		return RTC::RTC_ERROR;
	}
  
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onActivated(RTC::UniqueId ec_id)
{
	pinMode(m_IR_IN_PIN, INPUT);
	pinMode(m_IR_OUT_PIN, OUTPUT);
  return RTC::RTC_OK;
}


RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onDeactivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}


RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onExecute(RTC::UniqueId ec_id)
{
  if (m_state == STATE_WRITE) {
    m_state = writeToPort();
  } else if (m_state == STATE_READ) {
    if(m_InIRDataIn.isNew()) {
      m_InIRDataIn.read();
      m_state = readFromPort();
			if (m_state == STATE_IDLE) {
				transmitSignal(m_IRData, m_IRData_size);
			}
    }
  } else { // m_state == STATE_IDLE
    if(m_InIRDataIn.isNew()) {
      m_InIRDataIn.read();
      if (m_InIRData.data.length() == 0) { //receive
        m_IRData_size = receiveSignal(m_IRData, DATA_SIZE);
        
        
        m_IRData_index = 0;
        
        m_state = writeToPort();
      } else {
        m_IRData_index = 0;
        
        m_state = readFromPort();
				if (m_state == STATE_IDLE) {
					transmitSignal(m_IRData, m_IRData_size);
				}
      }
    }
  }

  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IR_Transceiver_RaspberryPi::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

void IR_Transceiver_RaspberryPi::transmitSignal(const uint16_t* data, uint32_t data_size)
{
	uint16_t time;
	uint32_t count;
	uint32_t us;

	for(count = 0; count < data_size; count++){
		time = data[count];
		us = micros();
		do {
			digitalWrite(m_IR_OUT_PIN, (count&1)^1);
			usleep(8);
			delayMicroseconds(8);
			digitalWrite(m_IR_OUT_PIN, 0);
			usleep(7);
		}while(int32_t(us + time - micros()) > 0);
	}
}

uint32_t IR_Transceiver_RaspberryPi::receiveSignal(uint16_t* data, uint32_t data_size)
{
	uint8_t pre_value;
	uint8_t now_value;
	uint32_t now_us;
	uint32_t pre_us;
	uint32_t count;

	count = 0;
	pre_us = micros();
	while (1) {
		now_value = digitalRead(m_IR_IN_PIN);
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
		now_value = digitalRead(m_IR_IN_PIN);
		now_us = micros();
		if(pre_value != now_value){
			data[count] = now_us - pre_us;
			count++;
			pre_value = now_value;
			pre_us = now_us;    
		}

		if ((now_us - pre_us) > TIMEOUT_RECV_NOSIGNAL) {
			break;
		}
	}

	return count;
}

int IR_Transceiver_RaspberryPi::readFromPort(void)
{
	const uint32_t len = m_InIRData.data.length();
	uint32_t i;
	int ret = STATE_READ;

	for (i = 0; i < len && m_IRData_index < DATA_SIZE; i++, m_IRData_index++) {
		if (m_InIRData.data[i] == 0) {
			m_IRData_size = m_IRData_index;
			ret = STATE_IDLE;
			break;
		}
		m_IRData[m_IRData_index] = m_InIRData.data[i];
	}

	return ret;
}

int IR_Transceiver_RaspberryPi::writeToPort(void)
{
	uint32_t i;
	uint32_t len;
	int ret = STATE_WRITE;

	if ((m_IRData_index+BYTE_PER_CYCLE) > m_IRData_size) {
		len = m_IRData_size - m_IRData_index;
		m_OutIRData.data.length(len+1);      
	} else {
		len = BYTE_PER_CYCLE;
		m_OutIRData.data.length(BYTE_PER_CYCLE);
	}
	for (i = 0; i < BYTE_PER_CYCLE; i++, m_IRData_index++) {
		if (m_IRData_index >= m_IRData_size) {
			m_OutIRData.data[i] = 0;
			ret = STATE_IDLE;
			break;
		}
		m_OutIRData.data[i] = m_IRData[m_IRData_index];
	}
	m_OutIRDataOut.write();  

	return ret;
}

extern "C"
{
 
	void IR_Transceiver_RaspberryPiInit(RTC::Manager* manager)
	{
		coil::Properties profile(ir_transceiver_raspberrypi_spec);
    manager->registerFactory(profile,
                             RTC::Create<IR_Transceiver_RaspberryPi>,
                             RTC::Delete<IR_Transceiver_RaspberryPi>);
  }
  
};


