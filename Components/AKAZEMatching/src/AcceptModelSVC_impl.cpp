// -*-C++-*-
/*!
 * @file  AcceptModelSVC_impl.cpp
 * @brief Service implementation code of AcceptModel.idl
 *
 */

#include <iostream>
#include <string>
#include "AcceptModelSVC_impl.h"

/*
 * Example implementational code for IDL interface AcceptModelService
 */
AcceptModelServiceSVC_impl::AcceptModelServiceSVC_impl()
{
  // Please add extra constructor code here.
}


AcceptModelServiceSVC_impl::~AcceptModelServiceSVC_impl()
{
  // Please add extra destructor code here.
}


/*
 * Methods corresponding to IDL attributes and operations
 */
void AcceptModelServiceSVC_impl::setModel(const AcceptModelService::OctetSeq& model)
{
  while(1){
    if(!_setModelFlag){
      break;
    }
  }

  std::string cmodel((char*)&model[0], model.length());
  std::cout << "AcceptModelServiceSVC_impl::setModel() is called.:" << cmodel << std::endl;
  
  notifyEvent(cmodel);
}



// End of example implementational code



