import json
import socket
import struct
import time
import os, stat
import sys
import subprocess
from os.path import dirname

from flatbuffers import util

import numpy as np
sys.path.insert(0, '../../soccer/firmware/jrlmonitor/include')

import SettingsData

from bembelapi.config import ROBOT_ROLES, NaoMonitorAction
from bembelapi.messages import MonitorMessage, MonitorConfigMessage, MonitorCalibrationMessage

from bembelapi.util.ObjectApi import bembelbuf

file_path = dirname(__file__)
identity_file = ""

@bembelbuf(SettingsData)
class ReadableSettingsDataT:
    pass


VR_DTYPE = np.dtype([
    ("vtype"            , np.int32  ),
    ("timestamp"        , np.int32  ),
    ("ics_x1"           , np.int32  ),
    ("ics_y1"           , np.int32  ),
    ("ics_x2"           , np.int32  ),
    ("ics_y2"           , np.int32  ),
    ("ics_width"        , np.int32  ),
    ("ics_height"       , np.int32  ),
    ("ics_confidence"   , np.float32),
    ("rcs_x1"           , np.float32),
    ("rcs_y1"           , np.float32),
    ("rcs_x2"           , np.float32),
    ("rcs_y2"           , np.float32),
    ("rcs_alpha"        , np.float32),
    ("rcs_distance"     , np.float32),
    ("rcs_confidence"   , np.float32),
    ("camera"           , np.int32  ),
    ("extra_int"        , np.int32  ),
    ("extra_float"      , np.float32),
])

class RobotConfig:

    class JerseyNr(int):

        UNKNOWN = 0

        def __str__(self):
            if self == RobotConfig.JerseyNr.UNKNOWN:
                return "Unknown"
            else:
                return super().__str__()

    def __init__(self):
        self.vals = None
        self.robotName = ""
        self.teamNr = RobotConfig.JerseyNr(RobotConfig.JerseyNr.UNKNOWN)
        self.jerseyNr = RobotConfig.JerseyNr(RobotConfig.JerseyNr.UNKNOWN)
        self.role = "Unknown"       
        self.lan = ""
        self.lan_link = False
        self.wlan = ""
        self.essid = ""
        self.signalStrength = None
        self.battery = -1
        self.batteryCharging = False
        self.backendRunning = None
        self.frontendRunning = None
        self.failState = 0
        self.joint_temperatures = None
        self.joint_status = None
        self.configuration = None
        self.calibration = None
        self.buildInfoFrontend = ""
        self.buildInfoBackend = ""

    def setConfiguration(self, conf):
        if len(conf) == 0:
            return
        self.configuration = conf
        self.jerseyNr = conf["jerseyNumber"]
        self.role = ROBOT_ROLES[int(conf["role"])]
        self.teamNr = conf["teamNumber"]

    def setCalibration(self, calib):
        if len(calib) == 0:
            return
        self.calibration = calib

    def loadJson(self, jstr: str) -> dict:
        obj = json.loads(jstr)
        if not obj.get(self.robotName, False):
            return {}
        return obj[self.robotName]

    @classmethod
    def parseData(cls, data):

        buf, offset = util.RemoveSizePrefix(data, 0)
        data = ReadableSettingsDataT.asRoot(buf, offset)

        inst = cls()

        inst.robotName = data.robotName
        inst.lan = data.lanAddress
        inst.lan_link = data.lanLink
        inst.wlan = data.wlanAddress
        inst.signalStrength = data.wlanSignalStrength
        inst.essid = data.wlanEssid
        inst.frontendRunning = data.frontendRunning
        inst.backendRunning = data.backendRunning
        inst.failState = data.failState
        if data.joints:
            inst.joint_temperatures = data.joints.Temperatures()
            inst.joint_status = data.joints.Status()
        inst.battery = data.batteryCharge
        inst.batteryCharging = data.batteryCharging
        inst.buildInfoFrontend = data.buildInfoFrontend
        inst.buildInfoBackend = data.buildInfoBackend

        if data.configuration:
            configuration = inst.loadJson(data.configuration)
            inst.setConfiguration(configuration)

        if data.calibration:
            calibration = inst.loadJson(data.calibration)
            inst.setCalibration(calibration)

        return inst

    def __str__(self):
        msg = "robot %s online with eth0 %s, " % (self.robotName, self.lan)
        msg += "wlan0 %s (connected to %s with strength %d), " % (self.wlan, self.essid, self.signalStrength)
        msg += "battery at %d%%, " % int(self.battery * 100)
        msg += "running backend(%d), frontend(%d). " % (self.backendRunning, self.frontendRunning)
        return msg


class NaoCamImage:
    def __init__(self, image_type=-1, camera=0, timestamp=0, size=0):
        self._camera = camera
        self._type = image_type
        self._timestamp = timestamp
        self._size = size
        self._data = np.array([], np.uint8)
        self._vision_results = np.array([], VR_DTYPE)

    def is_valid(self):
        return self._type != -1 and self._data.size == self._size and self._size != 0

    def set_data(self, string: str):
        self._data = np.fromstring(string, np.uint8)

    def set_vision_results(self, string: str):
        try:
            self._vision_results = np.fromstring(string, VR_DTYPE)
        except ValueError as e:
            print(e)
            self._vision_results = np.array([], VR_DTYPE)

    def get_data(self):
        return self._data

    def get_vision_results(self):
        return self._vision_results

    def size(self):
        return self._size

    def get_camera(self):
        return self._camera

    def get_type(self):
        return self._type
    
    def get_timestamp(self):
        return self._timestamp


class RobotHeartbeat:
    keepalive_interval = 0

    def __init__(self, timestamp=0, connected=False):
        self._keepalive_timestamp = timestamp
        self._connected = connected

    def get_timestamp(self):
        return self._keepalive_timestamp

    def is_connected(self):
        return self._connected

    def is_valid(self):
        return (time.time() - self._keepalive_timestamp) < self.keepalive_interval


class NaoInfo:
    def __init__(self, name: str, client: (str, int), _monitor_socket: socket.socket):
        self._name = name
        self._client = client
        self._monitor_socket = _monitor_socket
        self._debug_client = None
        self._monitor_client = None
        self._debug_address = None
        self._free = True
        self.last_seen = time.time()
        self._config = RobotConfig()
        #os.chmod(identity_file, stat.S_IREAD)

    @property
    def name(self) -> str:
        return self._name

    @property
    def client(self) -> (str, int):
        return self._client

    @property
    def config(self) -> RobotConfig:
        return self._config

    @config.setter
    def config(self, config: RobotConfig) -> None:
        if config.calibration is None and self.config.calibration is not None:
            config.setCalibration(self.config.calibration)

        if config.configuration is None and self.config.configuration is not None:
            config.setConfiguration(self.config.configuration)

        self._config = config

    @property
    def debug_client(self) -> (str, int):
        return self._debug_client

    @debug_client.setter
    def debug_client(self, addr: (str, int)) -> None:
        self._debug_client = addr

    @property
    def monitor_client(self) -> (str, int):
        return self._monitor_client

    @monitor_client.setter
    def monitor_client(self, addr: (str, int)) -> None:
        self._monitor_client = addr

    @property
    def debug_address(self) -> (str, int):
        return self._debug_address

    @debug_address.setter
    def debug_address(self, addr: (str, int)) -> None:
        self._debug_address = addr

    @property
    def free(self) -> bool:
        return self._free

    @free.setter
    def free(self, free) -> None:
        self._free = free

    def touch(self) -> None:
        self.last_seen = time.time()

    def is_debuggable(self) -> bool:
        return self._free and self._config.frontendRunning and self.debug_address is not None

    def is_expired(self, now) -> bool:
        return (now - self.last_seen) > 10

    def send_monitor(self, action: NaoMonitorAction, enabled: bool) -> None:
        message = MonitorMessage(self.name)
        message.add("2i", int(action.value), int(enabled))
        self._monitor_socket.sendto(message.get_bytes(), self.client)

    def send_config(self):
        message = MonitorConfigMessage(self.name, self.config.configuration)
        self._monitor_socket.sendto(message.get_bytes(), self.client)

    def send_calibration(self):
        message = MonitorCalibrationMessage(self.name, self.config.configuration, self.config.calibration)
        self._monitor_socket.sendto(message.get_bytes(), self.client)

    def __hash__(self):
        return hash(self.name)

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return NotImplemented
        return self.name == other.name

    def __repr__(self):
        return '<Nao({%s, last_seen: %s, free: %d})>' % (
            self.name, self.last_seen, self._free)

    def do_ssh(self, use: str):
        username = "nao"
        if use == "eth":
            address = self.config.lan
        elif use == "wlan":
            address = self.config.wlan
        else:
            print("Unknown ssh option " + use + ", should be 'eth' or 'wlan'")
        term_wrapper = os.path.join(file_path, "../termwrapper.sh")
        args = "-e sh -c 'ssh -x -i %s %s@%s'" % (identity_file, username, address)
        subprocess.call("%s %s &" % (term_wrapper, args), shell=True)
