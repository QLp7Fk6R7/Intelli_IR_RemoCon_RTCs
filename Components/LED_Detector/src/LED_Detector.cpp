// -*- C++ -*-
/*!
 * @file  LED_Detector.cpp
 * @brief LED Detector
 * @date $Date$
 *
 * $Id$
 */

#include "LED_Detector.h"

// Module specification
// <rtc-template block="module_spec">
static const char* led_detector_spec[] =
  {
    "implementation_id", "LED_Detector",
    "type_name",         "LED_Detector",
    "description",       "LED Detector",
    "version",           "1.0.0",
    "vendor",            "QL",
    "category",          "Image Processin",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.show_result", "0",
    "conf.default.threshold", "28",
    "conf.default.min", "5",
    "conf.default.max", "20",
    "conf.default.frame_count", "5",
    "conf.default.permissible_rate", "0.3",
    // Widget
    "conf.__widget__.show_result", "text",
    "conf.__widget__.threshold", "text",
    "conf.__widget__.min", "text",
    "conf.__widget__.max", "text",
    "conf.__widget__.frame_count", "text",
    "conf.__widget__.permissible_rate", "text",
    // Constraints
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
LED_Detector::LED_Detector(RTC::Manager* manager)
        // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_InImageIn("InImage", m_InImage),
    m_resultOut("result", m_result)

          // </rtc-template>
{
}

/*!
 * @brief destructor
 */
LED_Detector::~LED_Detector()
{
}



RTC::ReturnCode_t LED_Detector::onInitialize()
{
        // Registration: InPort/OutPort/Service
        // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("InImage", m_InImageIn);
  
  // Set OutPort buffer
  addOutPort("result", m_resultOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
        // </rtc-template>

        // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("show_result", m_show_result, "0");
  bindParameter("threshold", m_threshold, "28");
  bindParameter("min", m_min, "5");
  bindParameter("max", m_max, "20");
  bindParameter("frame_count", m_frame_count, "5");
  bindParameter("permissible_rate", m_permissible_rate, "0.3");
        // </rtc-template>

        return RTC::RTC_OK;
}

/*
   RTC::ReturnCode_t LED_Detector::onFinalize()
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t LED_Detector::onStartup(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t LED_Detector::onShutdown(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */


RTC::ReturnCode_t LED_Detector::onActivated(RTC::UniqueId ec_id)
{
        if (m_show_result != 0) {
                cv::namedWindow("LED Detector Result", CV_WINDOW_AUTOSIZE);
                cv::waitKey(30);
        }

        m_is_ready_to_ouput = false;
        m_index = 0;
        m_results_size = m_frame_count;

        m_results.reserve(m_results_size);
        for (size_t i = 0; i < m_results_size; i++) {
                m_results[i].reserve(3);
        }

        return RTC::RTC_OK;
}


RTC::ReturnCode_t LED_Detector::onDeactivated(RTC::UniqueId ec_id)
{
        if (m_show_result != 0) {
                cv::destroyWindow("LED Detector Result");
                cv::waitKey(30);
        }

        m_sceneImage.release();

        return RTC::RTC_OK;
}


RTC::ReturnCode_t LED_Detector::onExecute(RTC::UniqueId ec_id)
{
        if(!m_InImageIn.isNew()) {
                return RTC::RTC_OK;
        }
        m_InImageIn.read();

        if (m_InImage.data.image.format != CF_RGB) {
                std::cout << "unexpected format image" << std::endl;
                return RTC::RTC_ERROR;
        }

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
        if (m_InImage.data.image.format == CF_RGB) {
                cv::cvtColor(m_sceneImage, m_sceneImage, CV_RGB2BGR);
        }

        cv::Mat img_channels[3];
        cv::Mat img_bin[3];
        cv::Mat img_gray;
        cv::Mat img_shown;

        cvtColor(m_sceneImage, img_gray, CV_RGB2GRAY);
        cv::split(m_sceneImage, img_channels);

        for (int i = 0; i < 3; i++) {
                cv::threshold(img_channels[i] - img_gray, img_bin[2-i], m_threshold, 255.0, cv::THRESH_BINARY);
        }

        if (m_show_result != 0) {
                img_shown = m_sceneImage.clone();
        }

        for (unsigned int j = 0; j < 3; j++) {
                std::vector<std::vector<cv::Point> > contours;
                cv::findContours(img_bin[j], contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
                m_results[m_index][j] = 0;
                for(unsigned int i = 0; i < contours.size(); ++i) {
                        int count = contours[i].size();
                        if(count < m_min || count > m_max) {
                                continue; // （小さすぎる|大きすぎる）輪郭を除外
                        }
                        m_results[m_index][j] = 1;

                        if (m_show_result != 0) {
                                cv::Mat pointsf;
                                cv::Mat(contours[i]).convertTo(pointsf, CV_32F);
                                // 楕円フィッティング
                                cv::RotatedRect box = cv::fitEllipse(pointsf);
                                // 楕円の描画
                                if (j == 0) {
                                        cv::ellipse(img_shown, box, cv::Scalar(0,255,255), 2, CV_AA);
                                } else if (j == 1) {
                                        cv::ellipse(img_shown, box, cv::Scalar(255,0,255), 2, CV_AA);
                                } else {
                                        cv::ellipse(img_shown, box, cv::Scalar(255,255,0), 2, CV_AA);
                                }
                        }
                }
        }

        m_index++;
        if (m_index == m_results_size) {
                m_index = 0;
                m_is_ready_to_ouput = true;
        }

        if (m_is_ready_to_ouput) {
                int result_count_channels[3] = {0, 0, 0};
                for (size_t i = 0; i < m_results_size; i++) {
                        for (int j = 0; j < 3; j++) {
                                result_count_channels[j] += m_results[i][j];
                        }
                }
                printf("rate: ");
                const char rgb_chars[] = {'R', 'G', 'B'};
                char result = 0;
                for (int j = 0; j < 3; j++) {
                        double rate = (double)result_count_channels[j] / (double)m_results_size;
                        if (rate >= m_permissible_rate) {
                                result |= 1<<j;
                        }
                        printf("%c = %f, ", rgb_chars[j], rate);
                }
                printf("\n");

                m_result.data = result;
                m_resultOut.write();
        }


        if (m_show_result != 0) {
                cv::imshow("LED Detector Result", img_shown);
                cv::waitKey(30);
        }

        return RTC::RTC_OK;
}

/*
   RTC::ReturnCode_t LED_Detector::onAborting(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t LED_Detector::onError(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t LED_Detector::onReset(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t LED_Detector::onStateUpdate(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */

/*
   RTC::ReturnCode_t LED_Detector::onRateChanged(RTC::UniqueId ec_id)
   {
   return RTC::RTC_OK;
   }
   */



extern "C"
{

        void LED_DetectorInit(RTC::Manager* manager)
        {
                coil::Properties profile(led_detector_spec);
                manager->registerFactory(profile,
                                RTC::Create<LED_Detector>,
                                RTC::Delete<LED_Detector>);
        }

};


