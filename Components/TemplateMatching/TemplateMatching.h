// -*- C++ -*-
/*!
 * @file  TemplateMatching.h
 * @brief Matching Template Module
 * @date  $Date$
 *
 * $Id$
 */

#ifndef TEMPLATEMATCHING_H
#define TEMPLATEMATCHING_H

#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/CorbaPort.h>
#include <rtm/DataInPort.h>
#include <rtm/DataOutPort.h>
#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/idl/ExtendedDataTypesSkel.h>
#include <rtm/idl/InterfaceDataTypesSkel.h>
// #include <rtm/idl/Img.h>

// Service implementation headers
// <rtc-template block="service_impl_h">
#include "AcceptModelSVC_impl.h"

// </rtc-template>

// Service Consumer stub headers
// <rtc-template block="consumer_stub_h">

// </rtc-template>
// for util
#include "MultiCastEventListener.h"
#include "ImageViewer.h"
#include "TimeStampUtil_forRTC.h"
#include "ConnectionCheckUtil_forRTC.h"

#include "ImgStub.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace RTC;
using namespace Img;
using namespace ALTH;
using namespace ALTH::UTIL;

/*!
 * @class TemplateMatching
 * @brief Matching Template Module
 *
 */
class TemplateMatching
  : public RTC::DataFlowComponentBase,
    public EventListener<std::string>

{
 public:
  /*!
   * @brief constructor
   * @param manager Maneger Object
   */
  TemplateMatching(RTC::Manager* manager);

  /*!
   * @brief destructor
   */
  ~TemplateMatching();

  // <rtc-template block="public_attribute">
  
  // </rtc-template>

  // <rtc-template block="public_operation">
  
  // </rtc-template>

  /***
   *
   * The initialize action (on CREATED->ALIVE transition)
   * formaer rtc_init_entry() 
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onInitialize();

  /***
   *
   * The finalize action (on ALIVE->END transition)
   * formaer rtc_exiting_entry()
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onFinalize();

  /***
   *
   * The startup action when ExecutionContext startup
   * former rtc_starting_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onStartup(RTC::UniqueId ec_id);

  /***
   *
   * The shutdown action when ExecutionContext stop
   * former rtc_stopping_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onShutdown(RTC::UniqueId ec_id);

  /***
   *
   * The activated action (Active state entry action)
   * former rtc_active_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onActivated(RTC::UniqueId ec_id);

  /***
   *
   * The deactivated action (Active state exit action)
   * former rtc_active_exit()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onDeactivated(RTC::UniqueId ec_id);

  /***
   *
   * The execution action that is invoked periodically
   * former rtc_active_do()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

  /***
   *
   * The aborting action when main logic error occurred.
   * former rtc_aborting_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onAborting(RTC::UniqueId ec_id);

  /***
   *
   * The error action in ERROR state
   * former rtc_error_do()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onError(RTC::UniqueId ec_id);

  /***
   *
   * The reset action that is invoked resetting
   * This is same but different the former rtc_init_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onReset(RTC::UniqueId ec_id);
  
  /***
   *
   * The state update action that is invoked after onExecute() action
   * no corresponding operation exists in OpenRTm-aist-0.2.0
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onStateUpdate(RTC::UniqueId ec_id);

  /***
   *
   * The action that is invoked when execution context's rate is changed
   * no corresponding operation exists in OpenRTm-aist-0.2.0
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onRateChanged(RTC::UniqueId ec_id);


 protected:
  // <rtc-template block="protected_attribute">
  
  // </rtc-template>

  // <rtc-template block="protected_operation">
  
  // </rtc-template>

  // Configuration variable declaration
  // <rtc-template block="config_declare">
  /*!
   * 
   * - Name:  string_file_name
   * - DefaultValue: lena.png
   */
  std::string m_string_file_name;
  /*!
   * 
   * - Name:  Image_Window
   * - DefaultValue: off
   */
  std::string m_Image_Window;
  // </rtc-template>

  // DataInPort declaration
  // <rtc-template block="inport_declare">
  TimedCameraImage m_InImage;
  /*!
   */
  InPort<TimedCameraImage> m_InImageIn;
  
  // </rtc-template>


  // DataOutPort declaration
  // <rtc-template block="outport_declare">
  TimedCameraImage m_OutImage;
  /*!
   */
  OutPort<TimedCameraImage> m_OutImageOut;


  TimedDoubleSeq m_OutCorners;
  /*!
   */
	OutPort<TimedDoubleSeq> m_OutCornersOut;
  
  // TimedDoubleSeq m_xy;
  // /*!
  //  */
  // OutPort<TimedDoubleSeq> m_OutXY;
  

  // </rtc-template>

  // CORBA Port declaration
  // <rtc-template block="corbaport_declare">
  /*!
   */
  RTC::CorbaPort m_ModelPort;
  
  // </rtc-template>

  // Service declaration
  // <rtc-template block="service_declare">
  /*!
   */
  AcceptModelServiceSVC_impl m_ModelAcceptor;
  
  // </rtc-template>

  // Consumer declaration
  // <rtc-template block="consumer_declare">
  
  // </rtc-template>

 private:
  // <rtc-template block="private_attribute">
  
  // </rtc-template>

  // <rtc-template block="private_operation">
  
  // </rtc-template>

  int width, height, channels;
  cv::Mat image;
  cv::Mat src, otmp, tmp, dst;
  int i;

  // name
  std::string _objName;

  // Flag if findObject() function execute  
  bool _onExeFlag;
  
  // read model is  flag
  bool _flag;
  // Flag ;  model is not read : false, model is read : true 
  bool _readModelFlag;

  // connection check
  int _modelC;

  // load template image 
  bool load_image();
  bool read_file(const char *extension);
  // main function for template matching 
  void findObject();

  
 public:
  // when "m_ModelAcceptor->setModel()" is called by other components,
  // this function is executed.
  void invokeEvent(const std::string &ev);


  

};


extern "C"
{
  DLL_EXPORT void TemplateMatchingInit(RTC::Manager* manager);
};

#endif // TEMPLATEMATCHING_H
