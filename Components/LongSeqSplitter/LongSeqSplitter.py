#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- Python -*-

"""
 @file LongSeqSplitter.py
 @brief split LongSeq data
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
longseqsplitter_spec = ["implementation_id", "LongSeqSplitter",
                        "type_name",         "LongSeqSplitter",
                        "description",       "split LongSeq data",
                        "version",           "1.0.0",
                        "vendor",            "VenderName",
                        "category",          "Category",
                        "activity_type",     "STATIC",
                        "max_instance",      "1",
                        "language",          "Python",
                        "lang_type",         "SCRIPT",
                        "conf.default.split_length", "10",
                        "conf.__widget__.split_length", "text",
                        ""]
# </rtc-template>


##
# @class LongSeqSplitter
# @brief split LongSeq data
#
#
class LongSeqSplitter(OpenRTM_aist.DataFlowComponentBase):

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
        """

         - Name:  split_length
         - DefaultValue: 10
        """
        self._split_length = [10]

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
        self.bindParameter("split_length", self._split_length, "10")

        # Set InPort buffers
        self.addInPort("InLongSeq", self._InLongSeqIn)

        # Set OutPort buffers
        self.addOutPort("OutLongSeq", self._OutLongSeqOut)

        # Set service provider to Ports

        # Set service consumers to Ports

        # Set CORBA Service Ports

        return RTC.RTC_OK

    #    ##
    #    #
    #    # The finalize action (on ALIVE->END transition)
    #    # formaer rtc_exiting_entry()
    #    #
    #    # @return RTC::ReturnCode_t
    #
    #    #
#    def onFinalize(self):
    #
    #    return RTC.RTC_OK

    #    ##
    #    #
    #    # The startup action when ExecutionContext startup
    #    # former rtc_starting_entry()
    #    #
    #    # @param ec_id target ExecutionContext Id
    #    #
    #    # @return RTC::ReturnCode_t
    #    #
    #    #
#    def onStartup(self, ec_id):
    #
    #    return RTC.RTC_OK

    #    ##
    #    #
    #    # The shutdown action when ExecutionContext stop
    #    # former rtc_stopping_entry()
    #    #
    #    # @param ec_id target ExecutionContext Id
    #    #
    #    # @return RTC::ReturnCode_t
    #    #
    #    #
#    def onShutdown(self, ec_id):
    #
    #    return RTC.RTC_OK

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
        self.data = []

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

    def writeData(self):
        self._d_OutLongSeq.data = []
        rest = len(self.data)
        end = self._split_length[0] if rest > self._split_length[0] else rest
        for i in range(0, end):
            val = self.data.pop(0)
            self._d_OutLongSeq.data.append(val)
        self._OutLongSeqOut.write()

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
            self.data.extend(self._d_InLongSeq.data)

        if len(self.data) > 0:
            self.writeData()

        return RTC.RTC_OK

    #    ##
    #    #
    #    # The aborting action when main logic error occurred.
    #    # former rtc_aborting_entry()
    #    #
    #    # @param ec_id target ExecutionContext Id
    #    #
    #    # @return RTC::ReturnCode_t
    #    #
    #    #
#    def onAborting(self, ec_id):
    #
    #    return RTC.RTC_OK

    #    ##
    #    #
    #    # The error action in ERROR state
    #    # former rtc_error_do()
    #    #
    #    # @param ec_id target ExecutionContext Id
    #    #
    #    # @return RTC::ReturnCode_t
    #    #
    #    #
#    def onError(self, ec_id):
    #
    #    return RTC.RTC_OK

    #    ##
    #    #
    #    # The reset action that is invoked resetting
    #    # This is same but different the former rtc_init_entry()
    #    #
    #    # @param ec_id target ExecutionContext Id
    #    #
    #    # @return RTC::ReturnCode_t
    #    #
    #    #
#    def onReset(self, ec_id):
    #
    #    return RTC.RTC_OK

    #    ##
    #    #
    #    # The state update action that is invoked after onExecute() action
    #    # no corresponding operation exists in OpenRTm-aist-0.2.0
    #    #
    #    # @param ec_id target ExecutionContext Id
    #    #
    #    # @return RTC::ReturnCode_t
    #    #

    #    #
#    def onStateUpdate(self, ec_id):
    #
    #    return RTC.RTC_OK

    #    ##
    #    #
    #    # The action that is invoked when execution context's rate is changed
    #    # no corresponding operation exists in OpenRTm-aist-0.2.0
    #    #
    #    # @param ec_id target ExecutionContext Id
    #    #
    #    # @return RTC::ReturnCode_t
    #    #
    #    #
#    def onRateChanged(self, ec_id):
    #
    #    return RTC.RTC_OK


def LongSeqSplitterInit(manager):
    profile = OpenRTM_aist.Properties(defaults_str=longseqsplitter_spec)
    manager.registerFactory(profile,
                            LongSeqSplitter,
                            OpenRTM_aist.Delete)


def MyModuleInit(manager):
    LongSeqSplitterInit(manager)

    # Create a component
    manager.createComponent("LongSeqSplitter")


def main():
    mgr = OpenRTM_aist.Manager.init(sys.argv)
    mgr.setModuleInitProc(MyModuleInit)
    mgr.activateManager()
    mgr.runManager()

if __name__ == "__main__":
    main()
