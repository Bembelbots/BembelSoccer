import base64
import struct

import numpy as np

class RobotMessage:
    def __init__(self, data=None):
        self.timestamp = 0

        if data is not None:
            self.parseData(data)

    def parseData(self, data):
        vals = struct.unpack_from("i", data)
        self.timestamp = vals[0]

    def __str__(self):
        return " (Message at %d) " % self.timestamp

class BallData:

    # size of the ball data in bytes (size of the "Ball" class in worldmodel, packed in memory)
    # this includes the timestamp int (RobotMessage)
    SIZE = 48

    def __init__(self, data=None):
        if data is not None:
            self.message = RobotMessage(data[:4])
            vals = struct.unpack_from("i7f", data[4:])
            self.ballId = vals[0]

            # Coord pos
            self.posX = vals[1]
            self.posY = vals[2]

            # float posConfidence
            self.posConfidence = vals[3]

            # Coord motion
            self.motionX = vals[4]
            self.motionY = vals[5]

            # float motionConfidence
            self.motionConfidence = vals[6]

            self.localConfidence = vals[7]
        else:
            self.message = RobotMessage()
            self.ballId = -1

            # Coord pos
            self.posX = 0.0
            self.posY = 0.0

            # float posConfidence
            self.posConfidence = 0.0

            # Coord motion
            self.motionX = 0.0
            self.motionY = 0.0

            # float motionConfidence
            self.motionConfidence = 0.0

            self.localConfidence = 0.0


# class to parse the packed binary data of the "Robot"-class in the WorldModel.
# the boolean active is one byte long, and is padded by 3 following bytes by the compiler
class RobotData:

    # size of the robot data in bytes (size of the "Robot" class in worldmodel, packed in memory)
    # this includes the timestamp int (RobotMessage)
    SIZE = 64

    def __init__(self, data=None):
        if data is not None:
            self.message = RobotMessage(data[:4])
            vals = struct.unpack_from("3i?xxx4f4fi", data[4:])
            self.robotId = vals[0]
            self.role = vals[1]
            self.fallenSince = vals[2]
            self.active = vals[3]      
            try:
                self.alpha = float(vals[4])
                self.alphaDeg = np.rad2deg(self.alpha)
            except Exception as e:
                self.alphaDeg = 0
                self.alpha = 0.0
                print("warning: failed to parse angle: %s!" % str(e))
            self.posX = vals[5]
            self.posY = vals[6]
            self.confidence = vals[7]
            self.GTposAlpha = float(vals[8])
            self.GTposAlphaDeg = np.rad2deg(self.GTposAlpha)
            self.GTposX = vals[9]
            self.GTposY = vals[10]
            self.GTconfidence = vals[11]
            self.GTtimestamp = vals[12]
        else:
            self.message = RobotMessage()
            self.robotId = -1
            self.role = -1
            self.fallenSince = 0.0
            self.active = 0
            self.alphaDeg = 0
            self.alpha = 0.0
            self.posX = 0.0
            self.posY = 0.0
            self.confidence = 0.0
            self.GTposAlphaDeg = 0
            self.GTposAlpha = 0.0
            self.GTposX = 0.0
            self.GTposY = 0.0
            self.GTconfidence = 0.0
            self.GTtimestamp = 0

    def setFromString(self, s):
        # new Robot id=0 t=3179 c=1 @ -3.21, -3, 1.568(rad)rad GROUNDTRUTH: 0,0,0(rad),Conf: 0
        # Robot id=2 s=2 team=3 t=6130 c=0 @ 2.14, -2.47, 2.04rad GROUNDTRUTH: 0,0,0,Conf: 0
        v = s.split(" ")

        self.message = RobotMessage()
        self.message.timestamp = int(v[2][2:])
        self.robotId = int(v[1][3:])
        self.posX = float(v[5][:-1])
        self.posY = float(v[6][:-1])
        self.alpha = float(v[7][:-8])
        gt = v[9].split(",")
        self.GTposX = float(gt[0])
        self.GTposY = float(gt[1])
        self.GTposAlpha = float(gt[2][:-5])
        self.confidence = float(v[3][2:])

    def c_str(self):
        msg = "Robot id=%d t=%d c=%f @ %f, %f, %frad GROUNDTRUTH: %f,%f,%f,Conf: %f" % \
              (self.robotId, self.message.timestamp, self.confidence,
               self.posX,
               self.posY, self.alpha, self.GTposX, self.GTposY, self.GTposAlpha, self.GTconfidence)
        return msg

    def __str__(self):
        msg = "Robot #%d (%0.2f, %0.2f) @ %0.2f with conf %0.2f " % \
              (self.robotId, self.posX, self.posY, self.alpha, self.confidence)

        return msg + str(self.message)

class DirectedCoord():
    def __init__(self, data=None):
        if data is None:
            self.position = (0,0,0)
        else:
            self.setFromString(data)

    def setFromString(self, s):
        s = s.split(",")
        s2 = s[1].split("@")
        x,y,a = float(s[0]),float(s2[0]), float(s2[1])
        self.position = (x,y,a)

class Coord():
    def __init__(self, data=None):
        if data is None:
            self.position = (0,0)
        else:
            self.setFromString(data)

    def setFromString(self, s):
        s = s.split(",")
        x,y = float(s[0]),float(s[1])
        self.position = (x,y)
