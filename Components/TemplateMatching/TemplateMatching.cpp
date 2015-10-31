// -*- C++ -*-
/*!
 * @file  TemplateMatching.cpp
 * @brief Matching Template Module
 * @date $Date$
 *
 * $Id$
 */

#include "TemplateMatching.h"

// Module specification
// <rtc-template block="module_spec">
static const char* templatematching_spec[] =
  {
    "implementation_id", "TemplateMatching",
    "type_name",         "TemplateMatching",
    "description",       "Matching Template Module",
    "version",           "1.0.0",
    "vendor",            "Osaka Univ.",
    "category",          "Processing Module",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.string_file_name", "lena.png",
    "conf.default.Image_Window", "off",
    // Widget
    "conf.__widget__.string_file_name", "text",
    "conf.__widget__.Image_Window", "text",
    // Constraints
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
TemplateMatching::TemplateMatching(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_InImageIn("InImage", m_InImage),
    m_OutImageOut("OutImage", m_OutImage),
    m_OutCornersOut("OutCorners", m_OutCorners),
    m_ModelPort("Model")

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
TemplateMatching::~TemplateMatching()
{
}



RTC::ReturnCode_t TemplateMatching::onInitialize()
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
  bindParameter("string_file_name", m_string_file_name, "lena.png");
  bindParameter("Image_Window", m_Image_Window, "off");

  // add listener to model acceptor port
  m_ModelAcceptor.addEventListener(this);
  m_ModelAcceptor.lockSetModel();
  
  _onExeFlag = false;
  _readModelFlag = false;
  
  // </rtc-template>
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t TemplateMatching::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t TemplateMatching::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t TemplateMatching::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t TemplateMatching::onActivated(RTC::UniqueId ec_id)
{
  ALTH::UTIL::CheckPortConnection cpc;
  cpc.addConnectorPropertyName("port.AcceptModelService.ModelAcceptor");
  _modelC = cpc(m_ModelPort);

  if( _modelC > 0 ){  // Port is connected 
    std::cout << "ServicePort:Model  is connected " << std::endl;
    _objName = "None";
    _readModelFlag = false;
  }else { // Port is not connected 
    std::cout << "ServicePort:Model  is not connected " << std::endl; 
    tmp = cv::imread(m_string_file_name,1);
    _readModelFlag = true;
  }

  cv::namedWindow("Load Picture", CV_WINDOW_AUTOSIZE);
  cv::namedWindow("Matching Template", CV_WINDOW_AUTOSIZE);

  m_ModelAcceptor.unlockSetModel();


  return RTC::RTC_OK;
}


RTC::ReturnCode_t TemplateMatching::onDeactivated(RTC::UniqueId ec_id)
{
  m_ModelAcceptor.lockSetModel();
  
  image.release();
  src.release();
  otmp.release();
  tmp.release();
  dst.release();

  cv::destroyWindow("Load Picture");
  cv::destroyWindow("Matching Template");

  return RTC::RTC_OK;
}


RTC::ReturnCode_t TemplateMatching::onExecute(RTC::UniqueId ec_id)
{
 // if object key is 'None', do nothing.
  if(_objName == "None"){return RTC::RTC_OK;}

  // model port is not connected, find target object 
  if( _modelC < 0 ){
    findObject();
    // std::cout << "finding object in onExecute " << std::endl;
  }

  // // set new model
  // if(! _readModelFlag){
  //   // std::cout << "set new model [" << _objName << "] " << std::ends;
  //   // if(!load_image()){
  //   //   std::cout << "... failed." << std::endl;
  //   //   _objName = "None";
  //   //   return RTC::OK;
  //   // }
  //   // _readModelFlag = true; // model is loaded
  // }
  // // find target object 
  // else{
  //   _onExeFlag = true;
  //   std::cout << "finding object in onExecute : " << findObject() << std::endl;
  //   _onExeFlag = false;
  // }

  if(_readModelFlag){
    _onExeFlag = true;
    //std::cout << "finding object in onExecute " << std::endl;
    findObject();
    _onExeFlag = false;
  }



  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t TemplateMatching::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t TemplateMatching::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t TemplateMatching::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t TemplateMatching::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t TemplateMatching::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

bool TemplateMatching::load_image(){
  if( read_file("png") )
    return true;
  if( read_file("jpg") )
    return true;

  return false;
}


bool TemplateMatching::read_file(const char *extension){
  string filename;
  filename = _objName+ "." +extension;
  tmp = cv::imread(filename,1);
  if( !tmp.empty() ){
    std::cout << filename << " was read " << std::endl;
    return true;
  } 
  return false;
}


void TemplateMatching::findObject(){
  if(!tmp.empty())
    cv::imshow("Load Picture", tmp);
  cv::waitKey(5);

  /*******In Put Part Start*********/

  if(m_InImageIn.isNew()){
    m_InImageIn.read();

    // fprintf(stderr, "tm.sec %ld\n", m_InImage.tm.sec);
    // fprintf(stderr, "tm.nsec %ld\n", m_InImage.tm.nsec);

    width = m_InImage.data.image.width;
    height = m_InImage.data.image.height;
    channels = (m_InImage.data.image.format == CF_GRAY) ? 1 :
	 (m_InImage.data.image.format == CF_RGB) ? 3 :
	 (m_InImage.data.image.raw_data.length()/width/height);
    // RTC_TRACE(("Capture image size %d x %d", width, height));
    // RTC_TRACE(("Channels %d", channels));

    if(channels == 3)
	 image.create(height, width, CV_8UC3);
    else
	 image.create(height, width, CV_8UC1);

    for(int i=0; i<height; ++i){
	 memcpy(&image.data[i*image.step],
		   &m_InImage.data.image.raw_data[i*width*channels],
		   sizeof(unsigned char)*width*channels);
    }
    // fprintf(stderr, "Intrinsic matrix element\n");
    // for(int i=0; i<5; ++i){
    // 	 fprintf(stderr, " %f",
    // 		    m_InImage.data.intrinsic.matrix_element[i]);
    // 	 RTC_TRACE(("Intrinsic matrix element[%d] %f",
    // 			  i, m_InImage.data.intrinsic.matrix_element[i]));
    // }
    // fprintf(stderr, "\n");

    // fprintf(stderr, "Distortion parameter\n");
    // for(unsigned int i=0;
    // 	   i<m_InImage.data.intrinsic.distortion_coefficient.length(); ++i){
    // 	 fprintf(stderr, " %f",
    // 		    m_InImage.data.intrinsic.distortion_coefficient[i]);
    // 	 RTC_TRACE(("Distortion parameter[%d] %f",
    // 			  i, m_InImage.data.intrinsic.distortion_coefficient[i]));
    // }
    // fprintf(stderr, "\n");

    /*********************In Put Part End*****************************/
    /*********************Image Processing Part Start*****************/

    //reloade tmp picture
    if(channels == 1 && tmp.channels() != 1){
	 otmp = cv::imread(m_string_file_name, 0);
	 cv::cvtColor(otmp, tmp, CV_GRAY2BGR);
    }else if(channels == 3 && tmp.channels() != 3)
	 tmp = cv::imread(m_string_file_name, 1);

    if(channels == 1){
	 cv::cvtColor(image, src, CV_GRAY2BGR);
	 channels = 3;
    }else
	 cv::cvtColor(image, src, CV_RGB2BGR);

    cv::matchTemplate(src, tmp, dst, CV_TM_CCOEFF);

    double maxVal=0;
    cv::Point maxpoint;
    cv::Rect rect(0, 0, tmp.cols, tmp.rows);

    //find max score
	cv::minMaxLoc(dst, NULL, &maxVal, NULL, &maxpoint);
    rect.x = maxpoint.x;
    rect.y = maxpoint.y;

    // std::cout << "(" << maxpoint.x << "," << maxpoint.y << ")" <<std::endl;
    // std::cout << "score = " << maxVal << std::endl;
    // std::cout << std::endl;
    // std::cout << "If (sore > 1.20000e+07) ,draw rectangle" << std::endl;
    // std::cout << std::endl;

    if(maxVal > 1.20000e+07 ){
	 //探索結果の場所に矩形を描画
	 cv::rectangle(src, rect, cv::Scalar(0, 0, 255), 3);
    } else {
			rect.width = 0;
			rect.height = 0;
		}

    cv::cvtColor(src, image, CV_BGR2RGB);

    /*********************Image Processing Part End*******************/

    /*********************Out Put Part Start**************************/

    // set and send Rect data
    setTimestamp(m_OutCorners);
		m_OutCorners.data.length(8);
		m_OutCorners.data[0] = rect.x;
		m_OutCorners.data[1] = rect.y;
		m_OutCorners.data[2] = rect.x + rect.width;
		m_OutCorners.data[3] = rect.y;
		m_OutCorners.data[4] = rect.x + rect.width;
		m_OutCorners.data[5] = rect.y + rect.height;
		m_OutCorners.data[6] = rect.x;
		m_OutCorners.data[7] = rect.y + rect.height;
    m_OutCornersOut.write();

    setTimestamp(m_OutImage);
    m_OutImage.data.captured_time = m_OutImage.tm;

    // copy camera intrinsic matrix to Data Port
    m_OutImage.data.intrinsic.matrix_element[0] = m_InImage.data.intrinsic.matrix_element[0];
    m_OutImage.data.intrinsic.matrix_element[1] = m_InImage.data.intrinsic.matrix_element[1];
    m_OutImage.data.intrinsic.matrix_element[2] = m_InImage.data.intrinsic.matrix_element[2];
    m_OutImage.data.intrinsic.matrix_element[3] = m_InImage.data.intrinsic.matrix_element[3];
    m_OutImage.data.intrinsic.matrix_element[4] = m_InImage.data.intrinsic.matrix_element[4];
    m_OutImage.data.intrinsic.distortion_coefficient.length(m_InImage.data.intrinsic.distortion_coefficient.length());

    // copy distortion coefficient to Data Port
    for(int j = 0; j < 5; ++j)
	 {
	   m_OutImage.data.intrinsic.distortion_coefficient[j] = m_InImage.data.intrinsic.distortion_coefficient[j];
	 }

    // copy TimedCameraImage data to Data Port
    m_OutImage.data.image.width = width;
    m_OutImage.data.image.height = height;
    m_OutImage.data.image.raw_data.length(width * height * channels);
    m_OutImage.data.image.format
	 = (channels == 3) ? Img::CF_RGB :
	   (channels == 1) ? Img::CF_GRAY : Img::CF_UNKNOWN;
    for(int i=0; i<height; ++i){
	 memcpy(&m_OutImage.data.image.raw_data[i*width*channels],
		   &image.data[i*image.step],
		   sizeof(unsigned char)*width*channels);
    }

    // send TimedCameraImage data
    m_OutImageOut.write();
    if( m_Image_Window == "on"){
	 if(!src.empty())
	   cv::imshow("Matching Template", src);
	 cv::waitKey(5);
    }

  }
  
}

// when "m_ModelAcceptor->setModel()" is called by other components,
// this function is executed.
void TemplateMatching::invokeEvent(const std::string &ev){
  std::cout << "invokeEvent is called." << std::endl;

  // if object ID is current ID, the object model is not updated.
  if(ev == _objName){
    std::cout << "Current model ID is same, so model was not changed" << std::endl;
    return;
  }

  // set model ID  
  _objName = ev;

  // if object ID is "None", the object model is not updated.
  if(ev == "None" ){
    std::cout << "Set model ID to None" << std::endl;
    return;
  }

  _readModelFlag = false;  
  std::cout << "Waiting for finishing to execute function : findObject()"  << std::endl;
  while(_onExeFlag){
    coil::usleep(1000);
  } 
  
  std::cout << "set new model [" << _objName << "] " << std::ends;
  if(!load_image()){
    std::cout << "... failed." << std::endl;
      _objName = "None";
  }

  _readModelFlag = true;

  // std::cout << "Waiting for finishing to read model image"  << std::endl;
  // while(!_readModelFlag){
  //   if()
  //   coil::usleep(1000);
  // } 

  std::cout << "Change model to " << ev << std::endl;

}


extern "C"
{
 
  void TemplateMatchingInit(RTC::Manager* manager)
  {
    coil::Properties profile(templatematching_spec);
    manager->registerFactory(profile,
                             RTC::Create<TemplateMatching>,
                             RTC::Delete<TemplateMatching>);
  }
  
};


