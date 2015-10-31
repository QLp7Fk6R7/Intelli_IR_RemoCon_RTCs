// -*- C++ -*-
/*!
 *
 * THIS FILE IS GENERATED AUTOMATICALLY!! DO NOT EDIT!!
 *
 * @file ImgSkel.h 
 * @brief Img server skeleton header wrapper code
 * @date Wed Aug 10 18:20:44 2011 
 *
 */

#ifndef IMGSKEL_H
#define IMGSKEL_H



#include <rtm/config_rtc.h>
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#if   defined ORB_IS_TAO
#  include "ImgC.h"
#  include "ImgS.h"
#elif defined ORB_IS_OMNIORB
#  if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#    undef USE_stub_in_nt_dll
#  endif
#  include "Img.hh"
#elif defined ORB_IS_MICO
#  include "Img.h"
#elif defined ORB_IS_ORBIT2
#  include "/Img-cpp-stubs.h"
#  include "/Img-cpp-skels.h"
#else
#  error "NO ORB defined"
#endif

#endif // IMGSKEL_H
