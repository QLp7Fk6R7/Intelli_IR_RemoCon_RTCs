#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- Python -*-

"""
@file IRController.py
@brief IRController
@date $Date$


"""
import sys
sys.path.append(".")

# Import RTM module
import RTC
import OpenRTM_aist

# import AcceptModel_idl

# Import Service implementation class
# <rtc-template block="service_impl">

# </rtc-template>

# Import Service stub modules
# <rtc-template block="consumer_import">
import _GlobalIDL
# import _GlobalIDL__POA


# </rtc-template>

import yaml
import struct

CONFIG_FILE = "config.yml"
SPEECH_FILE = "speechout.yml"

# This module's spesification
# <rtc-template block="module_spec">
ircontroller_spec = ["implementation_id", "IRController",
                     "type_name",         "IRController",
                     "description",       "IRController",
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
# @class IRController
# @brief IRController
#
#
class IRController(OpenRTM_aist.DataFlowComponentBase):

    ##
    # @brief constructor
    # @param manager Maneger Object
    #

    def __init__(self, manager):
        OpenRTM_aist.DataFlowComponentBase.__init__(self, manager)

        self._d_Instruction = RTC.TimedString(RTC.Time(0, 0), 0)
        """
        """
        self._InstructionIn = OpenRTM_aist.InPort("Instruction", self._d_Instruction)
        self._d_LEDState = RTC.TimedChar(RTC.Time(0, 0), 0)
        """
        """
        self._LEDStateIn = OpenRTM_aist.InPort("LEDState", self._d_LEDState)
        self._d_Speechout = RTC.TimedString(RTC.Time(0, 0), 0)
        """
        """
        self._SpeechoutOut = OpenRTM_aist.OutPort("Speechout", self._d_Speechout)
        self._d_IRSignal = RTC.TimedLongSeq(RTC.Time(0, 0), [])
        """
        """
        self._IRSignalOut = OpenRTM_aist.OutPort("IRSignal", self._d_IRSignal)

        """
        """
        self._ModelPort = OpenRTM_aist.CorbaPort("Model")

        """
        """
        self._ModelAcceptor = OpenRTM_aist.CorbaConsumer(interfaceType=_GlobalIDL.AcceptModelService)

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
        self.addInPort("Instruction", self._InstructionIn)
        self.addInPort("LEDState", self._LEDStateIn)

        # Set OutPort buffers
        self.addOutPort("Speechout", self._SpeechoutOut)
        self.addOutPort("IRSignal", self._IRSignalOut)

        # Set service provider to Ports

        # Set service consumers to Ports
        self._ModelPort.registerConsumer("ModelAcceptor", "AcceptModelService", self._ModelAcceptor)

        # Set CORBA Service Ports
        self.addPort(self._ModelPort)

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
        f = open(SPEECH_FILE, 'r')
        self.speechout = yaml.load(f)
        f.close()

        i = 0
        while i < 4:
            self.speechout[i] = self.speechout[i].encode('utf-8')
            i += 1

        self.targets = []
        self.state = 0

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

    def appendNewInstruction(self):
        self._d_Instruction = self._InstructionIn.read()
        inst = self._d_Instruction.data.split(" ")
        if len(inst) != 2:
            print "fatal error: illegal instruction"
            return False
        if inst[0] != "all" and not inst[0].isdigit():
            print "fatal error: illegal instruction"
            return False
        if not inst[1] != "on" and not inst[1] != "off":
            print "fatal error: illegal instruction"
            return False

        onoff = 0 if inst[1] == "on" else 1
        f = open(CONFIG_FILE, 'r')
        data = yaml.load(f)
        f.close()
        if inst[0] == "all":
            for d in data:
                d["onoff"] = onoff
            self.targets.extend(data)
        else:
            num = int(inst[0])
            if num < len(data):
                data[num]["onoff"] = onoff
                self.targets.append(data[num])
            else:
                print "error: Number.%s object does not exist" % (inst[0],)
                return True

        return True

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
        if self._InstructionIn.isNew():
            res = self.appendNewInstruction()
            if not res:
                return RTC.RTC_ERROR

        if len(self.targets) == 0:  # idle
            return RTC.RTC_OK

        target = self.targets[0]
        if self.state == 0:
            self._ModelAcceptor._ptr().setModel(target["slug"])
            self.state = 1
        else:
            if self._LEDStateIn.isNew():
                self._d_LEDState = self._LEDStateIn.read()
                led_state = struct.unpack("b", self._d_LEDState.data)
                print(led_state)
                cond = ((led_state[0] >> int(target["ledcolor"])) & 1) ^ int(target["ledonoff"]) ^ 1
                print(((led_state[0] >> int(target["ledcolor"])) & 1))
                print(cond)
                print(cond ^ target["onoff"])
                if cond ^ target["onoff"]:
                    self._d_Speechout.data = self.speechout[target["onoff"]]
                    self._d_IRSignal.data = target["ir_signal"]
                    self._IRSignalOut.write()
                else:
                    self._d_Speechout.data = self.speechout[target["onoff"] + 2]
                self._SpeechoutOut.write()
                self.targets.pop(0)
                self.state = 0

        return RTC.RTC_OK


def IRControllerInit(manager):
    profile = OpenRTM_aist.Properties(defaults_str=ircontroller_spec)
    manager.registerFactory(profile,
                            IRController,
                            OpenRTM_aist.Delete)


def MyModuleInit(manager):
    IRControllerInit(manager)

    # Create a component
    manager.createComponent("IRController")


def main():
    mgr = OpenRTM_aist.Manager.init(sys.argv)
    mgr.setModuleInitProc(MyModuleInit)
    mgr.activateManager()
    mgr.runManager()

if __name__ == "__main__":
    main()
