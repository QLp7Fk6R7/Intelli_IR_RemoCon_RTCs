// -*- C++ -*-
/*!
 * @file  AKAZEMatching.cpp
 * @brief image matching using AKAZE
 * @date $Date$
 *
 * $Id$
 */

#include "AKAZEMatching.h"

// Module specification
// <rtc-template block="module_spec">
static const char* akazematching_spec[] =
  {
    "implementation_id", "AKAZEMatching",
    "type_name",         "AKAZEMatching",
    "description",       "image matching using AKAZE",
    "version",           "1.0.0",
    "vendor",            "QL",
    "category",          "image processing",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.string_file_name", "None",
    // Widget
    "conf.__widget__.string_file_name", "text",
    // Constraints
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
AKAZEMatching::AKAZEMatching(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_InImageIn("InImage", m_InImage),
    m_OutImageOut("OutImage", m_OutImage),
    m_OutCornersOut("OutCorners", m_OutCorners),
    m_ModelPort("Model"),
		m_AKAZEAlgorithm(cv::AKAZE::create()),
		m_matcher(cv::DescriptorMatcher::create("BruteForce-Hamming"))
    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AKAZEMatching::~AKAZEMatching()
{
}

RTC::ReturnCode_t AKAZEMatching::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("InImage", m_InImageIn);
  
  // Set OutPort buffer
  addOutPort("OutImage", m_OutImageOut);
  addOutPort("OutCorners", m_OutCornersOut);
  
  // Set service provider to Ports
  m_ModelPort.registerProvider("ModelAcceptor", "AcceptModelService", m_ModelAcceptor);
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  addPort(m_ModelPort);
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("string_file_name", m_string_file_name, "None");
  // </rtc-template>

  // add listener to model acceptor port
  m_ModelAcceptor.addEventListener(this);
  m_ModelAcceptor.lockSetModel();
  
  m_onExeFlag = false;
  m_readModelFlag = false;
  
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t AKAZEMatching::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AKAZEMatching::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AKAZEMatching::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t AKAZEMatching::onActivated(RTC::UniqueId ec_id)
{
  ALTH::UTIL::CheckPortConnection cpc;
  cpc.addConnectorPropertyName("port.AcceptModelService.ModelAcceptor");
  m_modelC = cpc(m_ModelPort);

  if( m_modelC > 0 ){  // Port is connected 
    std::cout << "ServicePort:Model  is connected " << std::endl;
    m_objName = "None";
    m_readModelFlag = false;
  }else { // Port is not connected 
    std::cout << "ServicePort:Model  is not connected " << std::endl; 
		m_objName = m_string_file_name;
		load_image();

    m_readModelFlag = true;
  }
	
  m_ModelAcceptor.unlockSetModel();

  return RTC::RTC_OK;
}


RTC::ReturnCode_t AKAZEMatching::onDeactivated(RTC::UniqueId ec_id)
{
  m_ModelAcceptor.lockSetModel();
  
	m_objImage.release();
  m_sceneImage.release();
	m_objKeypoints.clear();
	m_objDescriptor.release();
  src.release();
  dst.release();

  cv::destroyWindow("Load Picture");
  cv::destroyWindow("Matching Template");

  return RTC::RTC_OK;
}


RTC::ReturnCode_t AKAZEMatching::onExecute(RTC::UniqueId ec_id)
{
 // if object key is 'None', do nothing.
  if(m_objName == "None"){return RTC::RTC_OK;}

  // model port is not connected, find target object 
  if( m_modelC < 0 ){
    findObject();
    // std::cout << "finding object in onExecute " << std::endl;
  }

  if(m_readModelFlag){
    m_onExeFlag = true;
    //std::cout << "finding object in onExecute " << std::endl;
    findObject();
    m_onExeFlag = false;
  }


  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t AKAZEMatching::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AKAZEMatching::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AKAZEMatching::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AKAZEMatching::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AKAZEMatching::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

bool AKAZEMatching::load_image(){
  if( read_file("png") )
    return true;
  if( read_file("jpg") )
    return true;
  if( read_file(NULL) )
    return true;
  return false;
}


bool AKAZEMatching::read_file(const char *extension){
	std::string filename;
	if (extension == NULL) {
		filename = m_objName;
	} else {
		filename = m_objName+ "." +extension;
	}
  m_objImage = cv::imread(filename,1);
  if (m_objImage.empty()) {
		return false;
  } 
	std::cout << filename << " was read " << std::endl;

	m_objKeypoints.clear();
	m_objDescriptor.release();
	m_AKAZEAlgorithm->detect(m_objImage, m_objKeypoints);
	m_AKAZEAlgorithm->compute(m_objImage, m_objKeypoints, m_objDescriptor);

	return true;
}


void AKAZEMatching::findObject()
{
	if (m_InImageIn.isNew()) {
		/*******In Put Part Start*********/
		m_InImageIn.read();

    m_width = m_InImage.data.image.width;
    m_height = m_InImage.data.image.height;
    m_channels = (m_InImage.data.image.format == CF_GRAY) ? 1 :
	 (m_InImage.data.image.format == CF_RGB) ? 3 :
	 (m_InImage.data.image.raw_data.length()/m_width/m_height);
		m_sceneImage.create(m_height, m_width, (m_channels == 3)?CV_8UC3:CV_8UC1);
		for (int i = 0; i < m_height; ++i) {
			memcpy(&m_sceneImage.data[i*m_sceneImage.step],
					&m_InImage.data.image.raw_data[i*m_width*m_channels],
					sizeof(unsigned char)*m_width*m_channels);
		}
		cv::cvtColor(m_sceneImage, m_sceneImage, CV_RGB2BGR);

    /*********************In Put Part End*****************************/
    /*********************Image Processing Part Start*****************/

		std::vector<cv::KeyPoint> sceneKeypoints;
		cv::Mat	sceneDescriptor;
		std::vector<cv::DMatch> good_matches;
		std::vector<std::vector<cv::DMatch> > dmatchess;

		m_AKAZEAlgorithm->detect(m_sceneImage, sceneKeypoints);
		if (sceneKeypoints.size() == 0) {
			std::cout << "not found keypoint" << std::endl;
			return;
		}
		m_AKAZEAlgorithm->compute(m_sceneImage, sceneKeypoints, sceneDescriptor);

		m_matcher->knnMatch(m_objDescriptor, sceneDescriptor, dmatchess, 2);
		pickOutGoodMatchesNNDR(dmatchess, good_matches, DRATIO);
		size_t good_matches_size = good_matches.size();
		if (good_matches_size < 4) {
			std::cout << "not enough matches" << std::endl;
			return;
		}

		std::vector<cv::Point2f> obj;
		std::vector<cv::Point2f> scene;
		for (unsigned int i = 0; i < good_matches_size; i++) {
			obj.push_back(m_objKeypoints[good_matches[i].queryIdx].pt);
			scene.push_back(sceneKeypoints[good_matches[i].trainIdx].pt);
		}

		cv::Mat H = cv::findHomography( obj, scene, cv::RANSAC);
		if (H.rows == 0) {
			std::cout << "not found homography" << std::endl;
			return;
		}

		cv::Mat matchImage;	
		std::vector<cv::Point2f> scene_corners(4);
		{
		std::vector<cv::Point2f> obj_corners(4);
		obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( m_objImage.cols, 0 );
		obj_corners[2] = cvPoint( m_objImage.cols, m_objImage.rows ); obj_corners[3] = cvPoint( 0, m_objImage.rows );
		int width = m_objImage.cols + m_sceneImage.cols;
		int height = (m_objImage.rows > m_sceneImage.rows) ? m_objImage.rows : m_sceneImage.rows;
		matchImage = cv::Mat(cv::Size(width, height), CV_8UC3, cv::Scalar(0,0,0));
		cv::Mat roi1(matchImage, cv::Rect(0, 0, m_objImage.cols, m_objImage.rows));
		cv::Mat roi2(matchImage, cv::Rect(m_objImage.cols, 0, m_sceneImage.cols, m_sceneImage.rows));
		m_objImage.copyTo(roi1);
		m_sceneImage.copyTo(roi2);

		//-- Get the corners from the image_1 ( the object to be "detected" )
		cv::perspectiveTransform(obj_corners, scene_corners, H);

		//-- Draw lines between the corners (the mapped object in the scene - image_2 )

		line(matchImage , scene_corners[0] + cv::Point2f( m_objImage.cols, 0), scene_corners[1] + cv::Point2f( m_objImage.cols, 0), cv::Scalar(0, 255, 0), 4 );
		line(matchImage, scene_corners[1] + cv::Point2f( m_objImage.cols, 0), scene_corners[2] + cv::Point2f( m_objImage.cols, 0), cv::Scalar( 0, 255, 0), 4 );
		line(matchImage, scene_corners[2] + cv::Point2f( m_objImage.cols, 0), scene_corners[3] + cv::Point2f( m_objImage.cols, 0), cv::Scalar( 0, 255, 0), 4 );
		line(matchImage, scene_corners[3] + cv::Point2f( m_objImage.cols, 0), scene_corners[0] + cv::Point2f( m_objImage.cols, 0), cv::Scalar( 0, 255, 0), 4 );

		}

    /*********************Image Processing Part End*******************/
    /*********************Out Put Part Start**************************/

		for (size_t i = 0; i < 4; i++) {
			std::cout << scene_corners[i].x << " " << scene_corners[i].y << std::endl;
		}
		std::cout << std::endl;

    // set and send corners data
    setTimestamp(m_OutCorners);
		m_OutCorners.data.length(8);
		for (size_t i = 0; i < 4; i++) {
			m_OutCorners.data[i*2] = scene_corners[i].x;
			m_OutCorners.data[i*2+1] = scene_corners[i].y;
		}
    m_OutCornersOut.write();

		cv::Mat outImage;
    setTimestamp(m_OutImage);
		cv::cvtColor(matchImage, outImage, CV_BGR2RGB);
    m_OutImage.data.captured_time = m_OutImage.tm;

    // copy camera intrinsic matrix to Data Port
    m_OutImage.data.intrinsic.matrix_element[0] = m_InImage.data.intrinsic.matrix_element[0];
    m_OutImage.data.intrinsic.matrix_element[1] = m_InImage.data.intrinsic.matrix_element[1];
    m_OutImage.data.intrinsic.matrix_element[2] = m_InImage.data.intrinsic.matrix_element[2];
    m_OutImage.data.intrinsic.matrix_element[3] = m_InImage.data.intrinsic.matrix_element[3];
    m_OutImage.data.intrinsic.matrix_element[4] = m_InImage.data.intrinsic.matrix_element[4];
    m_OutImage.data.intrinsic.distortion_coefficient.length(m_InImage.data.intrinsic.distortion_coefficient.length());

    // copy distortion coefficient to Data Port
    for(int j = 0; j < 5; ++j) {
	   m_OutImage.data.intrinsic.distortion_coefficient[j] = m_InImage.data.intrinsic.distortion_coefficient[j];
	 }

    // copy TimedCameraImage data to Data Port
    m_OutImage.data.image.width = outImage.cols;
    m_OutImage.data.image.height = outImage.rows;
    m_OutImage.data.image.raw_data.length(outImage.cols * outImage.rows * 3);
    m_OutImage.data.image.format = Img::CF_RGB;
    for(int i=0; i<outImage.rows; ++i){
			 memcpy(&m_OutImage.data.image.raw_data[i*outImage.cols*3],
		   &outImage.data[i*outImage.step],
		   sizeof(unsigned char)*outImage.cols*3);
    }

    // send TimedCameraImage data
    m_OutImageOut.write();
	}
}

// when "m_ModelAcceptor->setModel()" is called by other components,
// this function is executed.
void AKAZEMatching::invokeEvent(const std::string &ev){
  std::cout << "invokeEvent is called." << std::endl;

  // if object ID is current ID, the object model is not updated.
  if (ev == m_objName) {
    std::cout << "Current model ID is same, so model was not changed" << std::endl;
    return;
  }

  // set model ID  
  m_objName = ev;

  // if object ID is "None", the object model is not updated.
  if (ev == "None") {
    std::cout << "Set model ID to None" << std::endl;
    return;
  }

  m_readModelFlag = false;  
  std::cout << "Waiting for finishing to execute function : findObject()"  << std::endl;
  while (m_onExeFlag) {
    coil::usleep(1000);
  } 
  
  std::cout << "set new model [" << m_objName << "] " << std::ends;
  if (!load_image()) {
    std::cout << "... failed." << std::endl;
      m_objName = "None";
  }

  m_readModelFlag = true;

  // std::cout << "Waiting for finishing to read model image"  << std::endl;
  // while(!m_readModelFlag){
  //   if()
  //   coil::usleep(1000);
  // } 

  std::cout << "Change model to " << ev << std::endl;

}

void AKAZEMatching::pickOutGoodMatchesNNDR(const std::vector<std::vector<cv::DMatch> >& dmatchess,
                         std::vector<cv::DMatch>& good_matches, double nndr) {
  double dist1 = 0.0, dist2 = 0.0;
  for (size_t i = 0; i < dmatchess.size(); i++) {
    cv::DMatch dmatch = dmatchess[i][0];
    dist1 = dmatchess[i][0].distance;
    dist2 = dmatchess[i][1].distance;

    if (dist1 < nndr*dist2) {
			good_matches.push_back(dmatch);
    }
  }
}
extern "C"
{
 
  void AKAZEMatchingInit(RTC::Manager* manager)
  {
    coil::Properties profile(akazematching_spec);
    manager->registerFactory(profile,
                             RTC::Create<AKAZEMatching>,
                             RTC::Delete<AKAZEMatching>);
  }
  
};


