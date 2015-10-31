#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -*- Python -*-

"""
@file WebUI.py
@brief Web User Interface
@date $Date$


"""
import sys
import time
sys.path.append(".")

# Import RTM module
import RTC
import OpenRTM_aist

import Img

import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.httpserver

import threading
import base64
import os
import yaml
import binascii

import numpy as np
import cv2

WEBSERVER_PORT = 8080
CONFIG_FILE = "config.yml"
g_img = ""
g_IRReceiver = None
g_Instruction = ""


class IRReceiver():

    def __init__(self):
        self.recv_events = [threading.Event(), threading.Event()]
        self.recv_events[0].clear()
        self.recv_events[1].clear()
        self.data = ""

    def receive(self):
        while self.recv_events[0].is_set():
            time.sleep(0.5)
        self.recv_events[1].set()
        while not self.recv_events[0].is_set():
            time.sleep(0.5)
        data = self.data
        self.recv_events[0].clear()
        return data

    def setData(self, data):
        self.data = data
        self.recv_events[0].set()
        self.recv_events[1].clear()

    def getState(self):
        return self.recv_events[1].is_set()


class HttpHandler(tornado.web.RequestHandler):

    def initialize(self):
        pass

    def post(self):
        self.render(self.request.path.lstrip("/") + ".html")

    def get(self):
        self.render(self.request.path.lstrip("/") + ".html")


class HttpHandlerRoot(HttpHandler):

    def moveItem(self, updown, slug):
        f = open(CONFIG_FILE, 'r')
        data = yaml.load(f)
        f.close()
        isNeedToWrite = True
        i = 0
        while i < len(data):
            if data[i]["slug"] == slug:
                break
            i += 1
        if updown == 0:
            if i > 0:
                tmp = data[i]
                data[i] = data[i - 1]
                data[i - 1] = tmp
            else:
                isNeedToWrite = False
        else:
            if (i + 1) < len(data):
                tmp = data[i]
                data[i] = data[i + 1]
                data[i + 1] = tmp
            else:
                isNeedToWrite = False

        if isNeedToWrite:
            f = open(CONFIG_FILE, 'w')
            f.write(yaml.dump(data))
            f.close()

    def removeItem(self, slug):
            f = open(CONFIG_FILE, 'r')
            data = yaml.load(f)
            f.close()
            i = 0
            while i < len(data):
                if data[i]["slug"] == slug:
                    del data[i]
                    break
                i += 1
            f = open(CONFIG_FILE, 'w')
            f.write(yaml.dump(data))
            f.close()

    def post(self):
        p = self.get_argument('p')
        slug = self.get_argument('slug')
        if p == "powerOnOff":
            global g_Instruction
            onoff = self.get_argument('onoff')
            onoff = int(onoff)
            f = open(CONFIG_FILE, 'r')
            data = yaml.load(f)
            f.close()
            i = 0
            while i < len(data):
                if data[i]["slug"] == slug:
                    del data[i]
                    break
                i += 1
            g_Instruction = "%d %s" % (i, "on" if onoff == 0 else "off")
            print("powerOnOff %d %s" % (onoff, slug))
        elif p == "moveItem":
            updown = self.get_argument('updown')
            updown = int(updown)
            print("moveItem %d %s" % (updown, slug))
            self.moveItem(updown, slug)
        elif p == "removeItem":
            self.removeItem(slug)
        self.write("")

    def get(self):
        try:
            f = open(CONFIG_FILE, 'r')
            data = yaml.load(f)
            f.close()
        except IOError:
            data = []

        self.render("index.html", data=data)


class HttpHandlerAdd(HttpHandler):

    def post(self):
        if self.get_argument('p') == "ir":
            global g_IRReceiver
            data = g_IRReceiver.receive()
            self.write(data)
        else:
            name = self.get_argument('name')
            ir_signal = self.get_argument('ir_signal')
            ledonoff = self.get_argument('ledonoff')
            ledcolor = self.get_argument('ledcolor')
            base64img = self.get_argument('img')

            name = name.encode('utf-8')
            ir_signal = map(int, ir_signal.split(","))
            ledonoff = int(ledonoff)
            ledcolor = int(ledcolor)
            slug = '%08x' % (binascii.crc32(name) & 0xffffffff)
            try:
                f = open(CONFIG_FILE, 'r')
                data = yaml.load(f)
                f.close()
            except IOError:
                data = []
            f = open(slug + ".png", "w")
            f.write(base64.b64decode(base64img))
            f.close()
            data.append({"slug": slug, "name": name, "ir_signal": ir_signal, "ledonoff": ledonoff, "ledcolor": ledcolor})
            f = open(CONFIG_FILE, 'w')
            f.write(yaml.dump(data))
            f.close()
            self.render(self.request.path.lstrip("/") + ".html")


class HttpHandlerCamera(HttpHandler):

    def post(self):
        print("post")
        base64img = self.get_argument('img')
        f = open("out.png", "w")
        f.write(base64.b64decode(base64img))
        f.close()
        self.write(open("index.html").read())
        HttpHandler.post()


class WSHandler(tornado.websocket.WebSocketHandler):

    def initialize(self):
        self.state = True

    def open(self):
        print(self.request.remote_ip, ": connection opened")
        t = threading.Thread(target=self.loop)
        t.setDaemon(True)
        t.start()

    def loop(self):
        global g_img
        import time
        print("start WSHandler Thread")
        while True:
            self.write_message(g_img, binary=True)
            if not self.state:
                break
            time.sleep(0.1)

    def on_close(self):
        self.state = False
        self.close()
        print(self.request.remote_ip, ": connection closed")


class WebServer():

    def __init__(self, port):
        self.port = port
        self.thread = None

        handlers = [
            (r"/", HttpHandlerRoot),
            (r"/index.html", HttpHandlerRoot),
            (r"/camera", WSHandler),
            (r"/add", HttpHandlerAdd),
        ]
        settings = dict(
            static_path=os.path.join(os.path.dirname(__file__), "static"),
            template_path=os.path.join(os.path.dirname(__file__), "templates"),
        )
        app = tornado.web.Application(handlers, **settings)
        http_server = tornado.httpserver.HTTPServer(app)
        http_server.listen(self.port)

    def start(self):
        if self.thread is None:
            self.thread = threading.Thread(target=self.target)
            self.thread.start()
        else:
            print("WebServer Thread is already working")

    def target(self):
        print("start WebServer Thread")
        tornado.ioloop.IOLoop.instance().start()
        print("stop WebServer Thread")

    def stop(self):
        tornado.ioloop.IOLoop.instance().stop()
        self.thread.join()
        self.thread = None


# Import Service implementation class
# <rtc-template block="service_impl">

# </rtc-template>

# Import Service stub modules
# <rtc-template block="consumer_import">
# </rtc-template>


# This module's spesification
# <rtc-template block="module_spec">
webui_spec = ["implementation_id", "WebUI",
              "type_name",         "WebUI",
              "description",       "Web User Interface",
              "version",           "1.0.0",
              "vendor",            "QL",
              "category",          "User Interfac",
              "activity_type",     "STATIC",
              "max_instance",      "1",
              "language",          "Python",
              "lang_type",         "SCRIPT",
              ""]
# </rtc-template>


##
# @class WebUI
# @brief Web User Interface
#
#
class WebUI(OpenRTM_aist.DataFlowComponentBase):

    ##
    # @brief constructor
    # @param manager Maneger Object
    #

    def __init__(self, manager):
        OpenRTM_aist.DataFlowComponentBase.__init__(self, manager)

        self._d_InImage = Img.TimedCameraImage(RTC.Time(0, 0), 0, 0)
        """
        """
        self._InImageIn = OpenRTM_aist.InPort("InImage", self._d_InImage)
        self._d_InIRSignal = RTC.TimedLongSeq(RTC.Time(0, 0), [])
        """
        """
        self._InIRSignalIn = OpenRTM_aist.InPort("InIRSignal", self._d_InIRSignal)
        self._d_OutLongSeq = RTC.TimedLongSeq(RTC.Time(0, 0), [])
        """
        """
        self._OutLongSeqOut = OpenRTM_aist.OutPort("OutLongSeq", self._d_OutLongSeq)
        self._d_Instruction = RTC.TimedString(RTC.Time(0, 0), 0)
        """
        """
        self._InstructionOut = OpenRTM_aist.OutPort("Instruction", self._d_Instruction)

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
        self.addInPort("InImage", self._InImageIn)
        self.addInPort("InIRSignal", self._InIRSignalIn)

        # Set OutPort buffers
        self.addOutPort("OutLongSeq", self._OutLongSeqOut)
        self.addOutPort("Instruction", self._InstructionOut)

        # Set service provider to Ports

        # Set service consumers to Ports

        # Set CORBA Service Ports

        self.server = WebServer(WEBSERVER_PORT)

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
        self.isReceiving = False
        self.server.start()

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
        self.server.stop()

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
        global g_img
        global g_IRReceiver
        global g_Instruction
        if self._InImageIn.isNew():
            self._d_InImage = self._InImageIn.read()
            data = np.frombuffer(self._d_InImage.data.image.raw_data, dtype=np.uint8)
            img = data.reshape((self._d_InImage.data.image.height, self._d_InImage.data.image.width, 3))
            img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)

            _, jpg_img = cv2.imencode(".jpg", img)
            g_img = jpg_img.tostring()

        if g_Instruction != "":
            self._d_Instruction.data = g_Instruction
            self._InstructionOut.write()
            g_Instruction = ""

        if self.isReceiving:
            if self._InIRSignalIn.isNew():
                self._d_InIRSignal = self._InIRSignalIn.read()
                data = ",".join(map(str, self._d_InIRSignal.data))
                g_IRReceiver.setData(data)
                self.isReceiving = False
        elif g_IRReceiver.getState():
            self._OutLongSeqOut.write()
            self.isReceiving = True

        return RTC.RTC_OK


def WebUIInit(manager):
    profile = OpenRTM_aist.Properties(defaults_str=webui_spec)
    manager.registerFactory(profile,
                            WebUI,
                            OpenRTM_aist.Delete)


def MyModuleInit(manager):
    WebUIInit(manager)

    # Create a component
    manager.createComponent("WebUI")


def main():
    global g_IRReceiver
    g_IRReceiver = IRReceiver()
    mgr = OpenRTM_aist.Manager.init(sys.argv)
    mgr.setModuleInitProc(MyModuleInit)
    mgr.activateManager()
    mgr.runManager()

if __name__ == "__main__":
    main()
