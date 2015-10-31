// -*- C++ -*-
/*!
 * @file  CropImage.cpp
 * @brief Crop Image
 * @date $Date$
 *
 * $Id$
 */

#include "CropImage.h"

// Module specification
// <rtc-template block="module_spec">
static const char* cropimage_spec[] =
{
        "implementation_id", "CropImage",
        "type_name",         "CropImage",
        "description",       "Crop Image",
        "version",           "1.0.0",
        "vendor",            "QL",
        "category",          "ImageProcessing",
        "activity_type",     "PERIODIC",
        "kind",              "DataFlowComponent",
        "max_instance",      "1",
        "language",          "C++",
        "lang_type",         "compile",
        ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
CropImage::CropImage(RTC::Manager* manager)
        // <rtc-template block="initializer">
        : RTC::DataFlowComponentBase(manager),
        m_InImageIn("InImage", m_InImage),
        m_CornersIn("Corners", m_Corners),
        m_OutImageOut("OutImage", m_OutImage)

          // </rtc-template>
{
}

/*!
 * @brief destructor
 */
CropImage::~CropImage()
{
}



RTC::ReturnCode_t CropImage::onInitialize()
{
        // Registration: InPort/OutPort/Service
        // <rtc-template block="registration">
        // Set InPort buffers
        addInPort("InImage", m_InImageIn);
        addInPort("Corners", m_CornersIn);

        // Set OutPort buffer
        addOutPort("OutImage", m_OutImageOut);

        // Set service provider to Ports

        // Set service consumers to Ports

        // Set CORBA Service Ports

        // </rtc-template>

        // <rtc-template block="bind_config">
        // </rtc-template>

        return RTC::RTC_OK;
}

/*
   RTC::ReturnCode_t CropImage::onFinalize()
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t CropImage::onStartup(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t CropImage::onShutdown(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */


RTC::ReturnCode_t CropImage::onActivated(RTC::UniqueId ec_id)
{
        return RTC::RTC_OK;
}


RTC::ReturnCode_t CropImage::onDeactivated(RTC::UniqueId ec_id)
{
        m_src.release();
        m_dst.release();
        return RTC::RTC_OK;
}


RTC::ReturnCode_t CropImage::onExecute(RTC::UniqueId ec_id)
{
        if (m_InImageIn.isNew()) {
                m_InImageIn.read();
                m_width = m_InImage.data.image.width;
                m_height = m_InImage.data.image.height;
                m_channels = (m_InImage.data.image.format == CF_GRAY) ? 1 :
                        (m_InImage.data.image.format == CF_RGB) ? 3 :
                        (m_InImage.data.image.raw_data.length()/m_width/m_height);
                m_src.create(m_height, m_width, (m_channels == 3)?CV_8UC3:CV_8UC1);
                for (int i = 0; i < m_height; ++i) {
                        memcpy(&m_src.data[i*m_src.step],
                                        &m_InImage.data.image.raw_data[i*m_width*m_channels],
                                        sizeof(unsigned char)*m_width*m_channels);
                }
                if (m_InImage.data.image.format == CF_RGB) {
                        cv::cvtColor(m_src, m_src, CV_RGB2BGR);
                }
        }

        if (m_CornersIn.isNew()) {
                if (m_src.empty()) {
                        std::cout << "no input image" << std::endl;
                        return RTC::RTC_ERROR;
                }
                m_CornersIn.read();

                int min_x = INT_MAX;
                int min_y = INT_MAX;
                int max_x = 0;
                int max_y = 0;
                for (size_t i = 0; i < 4; i++) {
                        int x = m_Corners.data[i*2];
                        int y = m_Corners.data[i*2+1];
                        if (x < min_x) {
                                min_x = x;
                        }
                        if (x > max_x) {
                                max_x = x;
                        }
                        if (y < min_y) {
                                min_y = y;
                        }
                        if (y > max_y) {
                                max_y = y;
                        }
                }
                if (min_x < 0) {
                        min_x = 0;
                }
                if (max_x >= m_src.cols) {
                        max_x = m_src.cols - 1;
                }
                if (min_y < 0) {
                        min_y = 0;
                }
                if (max_y >= m_src.rows) {
                        max_y = m_src.rows - 1;
                }
                if (max_x == min_x || max_y == min_y) {
                        return RTC::RTC_OK;
                }
                cv::Mat roi(m_src, cv::Rect(min_x, min_y, max_x-min_x+1, max_y-min_y+1));

                cv::cvtColor(roi, m_dst, CV_BGR2RGB);
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
                for(int j = 0; j < 5; ++j) {
                        m_OutImage.data.intrinsic.distortion_coefficient[j] = m_InImage.data.intrinsic.distortion_coefficient[j];
                }

                // copy TimedCameraImage data to Data Port
                m_OutImage.data.image.width = m_dst.cols;
                m_OutImage.data.image.height = m_dst.rows;
                m_OutImage.data.image.raw_data.length(m_dst.cols * m_dst.rows * m_dst.channels());
                m_OutImage.data.image.format = (m_dst.channels() == 3)?Img::CF_RGB:(m_dst.channels() == 1)?Img::CF_GRAY:CF_UNKNOWN;
                for(int i=0; i < m_dst.rows; ++i){
                        memcpy(&m_OutImage.data.image.raw_data[i * m_dst.cols * m_dst.channels()],
                                        &m_dst.data[i * m_dst.step],
                                        sizeof(unsigned char) * m_dst.cols * m_dst.channels());
                }

                // send TimedCameraImage data
                m_OutImageOut.write();
        }

        return RTC::RTC_OK;
}

/*
   RTC::ReturnCode_t CropImage::onAborting(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t CropImage::onError(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t CropImage::onReset(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t CropImage::onStateUpdate(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t CropImage::onRateChanged(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */



extern "C"
{

        void CropImageInit(RTC::Manager* manager)
        {
                coil::Properties profile(cropimage_spec);
                manager->registerFactory(profile,
                                RTC::Create<CropImage>,
                                RTC::Delete<CropImage>);
        }

};


