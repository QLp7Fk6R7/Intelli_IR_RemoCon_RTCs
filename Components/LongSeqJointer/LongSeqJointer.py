#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- Python -*-

"""
@file LongSeqJointer.py
@brief joint LongSeq data
@date $Date$


"""
import sys
sys.path.append(".")

# Import RTM module
import RTC
import OpenRTM_aist


# Import Service implementation class
# <rtc-template block="service_impl">

# </rtc-template>

# Import Service stub modules
# <rtc-template block="consumer_import">
# </rtc-template>


# This module's spesification
# <rtc-template block="module_spec">
longseqjointer_spec = ["implementation_id", "LongSeqJointer",
                       "type_name",         "LongSeqJointer",
                       "description",       "joint LongSeq data",
                       "version",           "1.0.0",
                       "vendor",            "QL",
                       "category",          "Category",
                       "activity_type",     "STATIC",
                       "max_instance",      "1",
                       "language",          "Python",
                       "lang_type",         "SCRIPT",
                       ""]
# </rtc-template>


##
# @class LongSeqJointer
# @brief joint LongSeq data
#
#
class LongSeqJointer(OpenRTM_aist.DataFlowComponentBase):

    ##
    # @brief constructor
    # @param manager Maneger Object
    #

    def __init__(self, manager):
        OpenRTM_aist.DataFlowComponentBase.__init__(self, manager)

        self._d_InLongSeq = RTC.TimedLongSeq(RTC.Time(0, 0), [])
        """
        """
        self._InLongSeqIn = OpenRTM_aist.InPort("InLongSeq", self._d_InLongSeq)
        self._d_OutLongSeq = RTC.TimedLongSeq(RTC.Time(0, 0), [])
        """
        """
        self._OutLongSeqOut = OpenRTM_aist.OutPort("OutLongSeq", self._d_OutLongSeq)

        # initialize of configuration-data.
        # <rtc-template block="init_conf_param">

        # </rtc-template>

    ##
    #
    # The initialize action (on CREATED->ALIVE transition)
    # formaer rtc_init_entry()
    #
    # @return RTC::ReturnCode_t
    #
    #
    def onInitialize(self):
        # Bind variables and configuration variable

        # Set InPort buffers
        self.addInPort("InLongSeq", self._InLongSeqIn)

        # Set OutPort buffers
        self.addOutPort("OutLongSeq", self._OutLongSeqOut)

        # Set service provider to Ports

        # Set service consumers to Ports

        # Set CORBA Service Ports

        return RTC.RTC_OK

    ##
    #
    # The activated action (Active state entry action)
    # former rtc_active_entry()
    #
    # @param ec_id target ExecutionContext Id
    #
    # @return RTC::ReturnCode_t
    #
    #
    def onActivated(self, ec_id):
        self.buf = []

        return RTC.RTC_OK

    ##
    #
    # The deactivated action (Active state exit action)
    # former rtc_active_exit()
    #
    # @param ec_id target ExecutionContext Id
    #
    # @return RTC::ReturnCode_t
    #
    #
    def onDeactivated(self, ec_id):

        return RTC.RTC_OK

    ##
    #
    # The execution action that is invoked periodically
    # former rtc_active_do()
    #
    # @param ec_id target ExecutionContext Id
    #
    # @return RTC::ReturnCode_t
    #
    #
    def onExecute(self, ec_id):
        if self._InLongSeqIn.isNew():
            self._d_InLongSeq = self._InLongSeqIn.read()
            self.buf.extend(self._d_InLongSeq.data)
            try:
                end = self.buf.index(0) + 1
                self._d_OutLongSeq.data = self.buf[:end]
                self._OutLongSeqOut.write()
                self.buf = []
            except ValueError:
                pass

        return RTC.RTC_OK


def LongSeqJointerInit(manager):
    profile = OpenRTM_aist.Properties(defaults_str=longseqjointer_spec)
    manager.registerFactory(profile,
                            LongSeqJointer,
                            OpenRTM_aist.Delete)


def MyModuleInit(manager):
    LongSeqJointerInit(manager)

    # Create a component
    manager.createComponent("LongSeqJointer")


def main():
    mgr = OpenRTM_aist.Manager.init(sys.argv)
    mgr.setModuleInitProc(MyModuleInit)
    mgr.activateManager()
    mgr.runManager()

if __name__ == "__main__":
    main()
