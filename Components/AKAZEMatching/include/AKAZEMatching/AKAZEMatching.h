// -*- C++ -*-
/*!
 * @file  AKAZEMatching.h
 * @brief image matching using AKAZE
 * @date  $Date$
 *
 * $Id$
 */

#ifndef AKAZEMATCHING_H
#define AKAZEMATCHING_H

#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/idl/ExtendedDataTypesSkel.h>
#include <rtm/idl/InterfaceDataTypesSkel.h>

// Service implementation headers
// <rtc-template block="service_impl_h">
// </rtc-template>

// Service Consumer stub headers
// <rtc-template block="consumer_stub_h">
#include "InterfaceDataTypesStub.h"
#include "InterfaceDataTypesStub.h"
#include "BasicDataTypeStub.h"

// </rtc-template>

// Service Consumer stub headers
// <rtc-template block="port_stub_h">
// </rtc-template>


#include "ImgStub.h"
#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/CorbaPort.h>
#include <rtm/DataInPort.h>
#include <rtm/DataOutPort.h>

#include "AcceptModelSVC_impl.h"
#include "ConnectionCheckUtil_forRTC.h"

#include <opencv2/videoio.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>

using namespace RTC;
using namespace Img;

/*!
 * @class AKAZEMatching
 * @brief image matching using AKAZE
 *
 */
class AKAZEMatching
  : public RTC::DataFlowComponentBase,
    public EventListener<std::string>
{
 public:
  /*!
   * @brief constructor
   * @param manager Maneger Object
   */
  AKAZEMatching(RTC::Manager* manager);

  /*!
   * @brief destructor
   */
  ~AKAZEMatching();

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
   * - DefaultValue: None
   */
  std::string m_string_file_name;

  // </rtc-template>

  // DataInPort declaration
  // <rtc-template block="inport_declare">
  Img::TimedCameraImage m_InImage;
  /*!
   */
  InPort<Img::TimedCameraImage> m_InImageIn;
  
  // </rtc-template>


  // DataOutPort declaration
  // <rtc-template block="outport_declare">
  Img::TimedCameraImage m_OutImage;
  /*!
   */
  OutPort<Img::TimedCameraImage> m_OutImageOut;
  RTC::TimedDoubleSeq m_OutCorners;
  /*!
   */
  OutPort<RTC::TimedDoubleSeq> m_OutCornersOut;
  
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

	cv::Ptr<cv::AKAZE> m_AKAZEAlgorithm;
	cv::Ptr<cv::DescriptorMatcher> m_matcher;

  int m_width, m_height, m_channels;
  cv::Mat m_sceneImage;
  cv::Mat src, dst;
	cv::Mat m_objImage;
  int i;

	std::vector<cv::KeyPoint> m_objKeypoints;
	cv::Mat m_objDescriptor;
  // name
  std::string m_objName;

  // Flag if findObject() function execute  
  bool m_onExeFlag;
  
  // read model is  flag
//  bool m_flag;
  // Flag ;  model is not read : false, model is read : true 
  bool m_readModelFlag;

  // connection check
  int m_modelC;

  // load template image 
  bool load_image();
  bool read_file(const char *extension);
  // main function for template matching 
  void findObject();
	void pickOutGoodMatchesNNDR(const std::vector<std::vector<cv::DMatch> >& dmatchess,
                         std::vector<cv::DMatch>& good_matches, double nndr);

	static const double DRATIO = 0.80f;                 ///< NNDR Matching value


 public:
  // when "m_ModelAcceptor->setModel()" is called by other components,
  // this function is executed.
  void invokeEvent(const std::string &ev);
};


extern "C"
{
  DLL_EXPORT void AKAZEMatchingInit(RTC::Manager* manager);
};

#endif // AKAZEMATCHING_H
