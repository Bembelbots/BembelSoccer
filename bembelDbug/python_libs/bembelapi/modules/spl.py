from struct import pack, unpack_from

SPL_STANDARD_MESSAGE_STRUCT_HEADER = b'SPL '
SPL_STANDARD_MESSAGE_STRUCT_VERSION = 7
SPL_STANDARD_MESSAGE_DATA_SIZE = 474
SPL_STANDARD_MESSAGE_MAX_NUM_OF_PLAYERS = 7

SIZE_SPL_MESSAGE = 508
SIZE_ROBOT = 108
SIZE_BALL = 48
SENSOR_DATA_SIZE = 660


class Message:
    def __init__(self, data=None):
        self.messageType = 0
        self.senderHash = 0
        self.sender = 0
        self.teamNumber = 0
        self.timestamp = 0

        if data is not None:
            self.parseData(data)

    def parseData(self, data):
        vals = unpack_from("ic3i", data)
        self.messageType = vals[0]
        self.senderHash = vals[1]
        self.sender = vals[2]
        self.teamNumber = vals[3]
        self.timestamp = vals[4]

    def __str__(self):
        return " (Message Type (%d) from %d team %d at %d) " % (
        self.messageType, self.sender, self.teamNumber, self.timestamp)


class Ball:
    def __init__(self, data=None):
        self.message = Message()
        self.confidence = 0.0
        self.posX = 0.0
        self.posY = 0.0
        self.motionX = 0.0
        self.motionY = 0.0
        self.motionConfidence = 0.0
        self.localConfidence = 0.0

        if data is not None:
            self.parseData(data)

    def parseData(self, data):
        self.message = Message(data[:20])
        vals = unpack_from("7f", data[20:])
        self.confidence = vals[0]
        self.posX = vals[1]
        self.posY = vals[2]
        self.motionX = vals[3]
        self.motionY = vals[4]
        self.motionConfidence = vals[5]
        self.localConfidence = vals[6]

    def __str__(self):
        msg = "Ball at (%0.2f, %0.2f) @ %0.2f with motion (%0.2f, %0.2f) @ %0.2f" % (
        self.posX, self.posY, self.confidence, self.motionX, self.motionY, self.motionConfidence)
        return msg + str(self.message)


class Robot:
    def __init__(self, data=None):
        if data is not None:
            self.message = Message(data[:20])
            vals = unpack_from("fi7f?2i9fi", data[20:])
            self.confidence = vals[0]
            self.robotId = vals[1]
            self.posX = vals[2]
            self.posY = vals[3]
            try:
                self.alpha = int(vals[4])
                self.alpha = float(vals[4])
            except Exception as e:
                self.alpha = 0.0
                print("warning: failed to parse angle: %s!" % str(e))

            self.GTposX = vals[5]
            self.GTposY = vals[6]
            self.GTposAlpha = vals[7]
            self.GTconfidence = vals[8]
            self.active = vals[9]
            self.GTtimestamp = vals[10]
            self.role = vals[11]
            d = {}
            for i in range(12, 21):
                p = i - 12
                d[p / 3, p % 3] = vals[i]
            self.covariance = d
            self.fallenSince = vals[21]
        else:
            self.message = Message()
            self.confidence = 0.0
            self.robotId = 0
            self.posX = 0.0
            self.posY = 0.0
            self.alpha = 0.0
            self.GTposX = 0.0
            self.GTposY = 0.0
            self.GTposAlpha = 0.0
            self.GTconfidence = 0.0

    def setFromString(self, s):
        # Robot id=2 s=2 team=3 t=6130 c=0 @ 2.14, -2.47, 2.04rad GROUNDTRUTH: 0,0,0,Conf: 0
        v = s.split(" ")

        self.message = Message()
        self.message.timestamp = int(v[4][2:])
        self.robotId = int(v[1][3:])
        self.posX = float(v[7][:-1])
        self.posY = float(v[8][:-1])
        self.alpha = float(v[9][:-3])
        gt = v[11].split(",")
        self.GTposX = float(gt[0])
        self.GTposY = float(gt[1])
        self.GTposAlpha = float(gt[2])
        self.confidence = float(v[5][2:])

    def c_str(self):
        msg = "Robot id=%d s=%d team=%d t=%d c=%f @ %f, %f, %frad GROUNDTRUTH: %f,%f,%f,Conf: %f" % \
              (self.robotId,
               self.message.sender,
               self.message.teamNumber,
               self.message.timestamp,
               self.confidence,
               self.posX,
               self.posY,
               self.alpha,
               self.GTposX,
               self.GTposY,
               self.GTposAlpha,
               self.GTconfidence)
        return msg

    def __str__(self):
        msg = "Robot #%d (%0.2f, %0.2f) @ %0.2f with conf %0.2f " % \
              (self.robotId,
               self.posX,
               self.posY,
               self.alpha,
               self.confidence)

        return msg + str(self.message)


class SPLStandardMessage:
    def __init__(self, data):
        if len(data) < SIZE_SPL_MESSAGE:
            print("size does not match spl standard message format!")
#            print(data)
            return

        self.bembelbotsMessage = False

        raw = data[:(SIZE_SPL_MESSAGE - SPL_STANDARD_MESSAGE_DATA_SIZE)]
        # print len(raw)

        vals = unpack_from("4sBbbb3f2f2ff2f2f5bbhhbbh", raw)
        # print(vals)

        self.header = vals[0]

        if self.header != SPL_STANDARD_MESSAGE_STRUCT_HEADER:
            print("packet does not look like a spl standard message!")
            return

        self.version = vals[1]
        if self.version != SPL_STANDARD_MESSAGE_STRUCT_VERSION:
            print("spl message has wrong version!")
            return

        self.playerNum = vals[2]

        self.team = vals[3]
        self.fallen = vals[4]

        self.posX = vals[5]
        self.posY = vals[6]
        self.alpha = vals[7]

        self.walkX = vals[8]
        self.walkY = vals[9]

        self.shootX = vals[10]
        self.shootY = vals[11]

        self.ballAge = vals[12]

        self.ballX = vals[13]
        self.ballY = vals[14]

        self.motionX = vals[15]
        self.motionY = vals[16]

        self.suggestions = vals[17:17 + SPL_STANDARD_MESSAGE_MAX_NUM_OF_PLAYERS]
        self.intention = vals[22]
        self.walkSpeed = vals[23]
        self.kickDist = vals[24]
        self.posConfidence = vals[25]
        self.sideConfidence = vals[26]
        self.dataSize = vals[27]

        blob = data[len(raw) - 2:]
        if self.dataSize > 3 and blob[0] == 'B' and blob[1] == 'B' and blob[2] == ' ':
            try:
                self.bbRobot = Robot(blob[3:3 + SIZE_ROBOT])
                self.bbBall = Ball(blob[3 + SIZE_ROBOT:])
                self.bembelbotsMessage = True
            except Exception as e:
                print("failed to parse additional info\n%s" % str(e))

    def __str__(self):
        out = "---------------SPL MESSAGE-----------------\n"
        out += "protocol version: %d\n" % self.version
        out += "player number: %d\n" % self.playerNum
        out += "team number %d \n" % self.team
        out += "fallen down? %d\n" % self.fallen
        out += "pos (%f,%f) @ %f\n" % (self.posX, self.posY, self.alpha)
        out += "ball age %d\n" % self.ballAge
        out += "ball (%f, %f)\n" % (self.ballX, self.ballY)
        out += "motion (%f, %f)\n" % (self.motionX, self.motionY)
        out += "additional data size %d\n" % self.dataSize
        out += "-------------------------------------------\n"
        return out
