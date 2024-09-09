import asyncio
import asyncore
import socket
from struct import unpack_from

import numpy as np
import enum
import math
from PyQt5 import QtCore
from utils.constants import Constants

import bbapi
from bbapi import TeamMessage, RobotName, RobotRole


SPL_HEADER = b'SPL '
SPL_VERSION = 7
SPL_MSG_SIZE = 34

BB_MAGIC = 0xBADC0DE
BB_MSG_SIZE = 140


class SPLMessage:
    def __init__(self):
        self.player_num = 0
        self.team_num = 0
        self.fallen = False
        self.position = [0, 0, 0]
        self.ball_age = 0.0
        self.ball_pos = [0, 0]
        self.data_size = 0
        self.data = b''

    @classmethod
    def fromPacket(cls, packet):
        splmessage = cls()

        vals = unpack_from("4s4B3ff2fH", packet)
        if vals[0] != SPL_HEADER:
            print("SPLStandardMessage: header does not match: got %s, expected %s" % (vals[0], SPL_HEADER))
            return splmessage
        if vals[1] != SPL_VERSION:
            print("SPLStandardMessage: parsing failed, version mismatch: got %d, expected %d" % (vals[0], SPL_VERSION))
            return splmessage

        splmessage.player_num = vals[2]
        splmessage.team_num = vals[3]
        splmessage.fallen = vals[4]
        splmessage.position = [vals[5], vals[6], vals[7]]
        splmessage.ball_age = vals[8]
        splmessage.ball_pos = [vals[9], vals[10]]
        splmessage.data_size = vals[11]
        splmessage.data = packet[SPL_MSG_SIZE:]
        return splmessage

    def __str__(self):
        return ("[{team}] SPLMessage {player} at ({pos[0]:.2f}, {pos[1]:.2f}, {pos[2]:.2f}):"
                "Fallen = {fallen}, Ball at ({ball_pos[0]:.2f}, {ball_pos[1]:.2f}) since {ball_age:.2f}".format(
                    team=self.team_num,
                    player=self.player_num,
                    pos=self.position,
                    fallen=self.fallen,
                    ball_pos=self.ball_pos,
                    ball_age=self.ball_age
                )
                )


class BembelSPLMessage(SPLMessage):
    def __init__(self):
        super().__init__()
        self.name = ""
        self.role = None
        self.teamball_pos = None
        self.teamball_confidence = None
        self.reactivewalk_walktarget_pos = None
        self.reactivewalk_walktarget_alpha = None
        self.reactivewalk_goaltarget_pos = None
        self.reactivewalk_goaltarget_alpha = None
        self.reactivewalk_obstacle_count = None
        self.reactivewalk_obstacles = []
        self.is_nearest_to_ball = False
        self.battery = 0

    @classmethod
    def fromPacket(cls, packet):
        splmessage = SPLMessage.fromPacket(packet)

        if splmessage.data_size != BB_MSG_SIZE:
            print("BembelMessage: parsing failed, data size does not match: got %s, expected %s" % (splmessage.data_size, BB_MSG_SIZE))
            return splmessage

        vals = unpack_from("II7I20si3f3f2fif10f", splmessage.data)

        if vals[0] != BB_MAGIC:
            print("BembelMessage: parsing failed, magic does not match: got %s, expected %s" % (vals[0], BB_MAGIC))
            return splmessage

        bembelmsg = cls()
        bembelmsg.player_num = splmessage.player_num
        bembelmsg.team_num = splmessage.team_num
        bembelmsg.fallen = splmessage.fallen
        bembelmsg.position = splmessage.position
        bembelmsg.ball_age = splmessage.ball_age
        bembelmsg.ball_pos = splmessage.ball_pos

        bembelmsg.name = vals[9].decode().replace('\x00', '')
        bembelmsg.role = vals[10]
        bembelmsg.teamball_pos = [vals[11], vals[12]]
        bembelmsg.teamball_confidence = vals[13]
        bembelmsg.reactivewalk_walktarget_pos = [vals[14], vals[15]]
        bembelmsg.reactivewalk_walktarget_alpha = vals[16]
        bembelmsg.reactivewalk_goaltarget_pos = [vals[17], vals[18]]
        bembelmsg.is_nearest_to_ball = vals[19]
        bembelmsg.battery = int(vals[20]*100)

        obstacle_count = 5
        offset = len(vals) - 2*obstacle_count

        for i in range(obstacle_count):
            if vals[2*i+offset] != -1000:
                bembelmsg.reactivewalk_obstacles.append([vals[2*i+offset], vals[2*i+offset+1]])

        return bembelmsg

    def __str__(self):
        obs = ""
        for o in self.reactivewalk_obstacles:
            obs += "(" + str(o[0]) + "," + str(o[1]) + ")"

        return ("[{team}] BembelSPLMessage {player} at ({pos[0]:.2f}, {pos[1]:.2f}, {pos[2]:.2f}):"
                "Fallen = {fallen}, Ball at ({ball_pos[0]:.2f}, {ball_pos[1]:.2f}) since {ball_age:.2f}\n"
                "\t name={name}, role={role}, teamball=({teamballx},{teambally}), confidence={teamballconfidence}\n"
                "\t walktarget=({walktargetx},{walktargety}@{walktargetalpha})\n"
                "\t goaltarget=({goaltargetx},{goaltargety}), nearesttoball={nearest}, battery={battery}%\n"
                "\t obstacles={obs}".format(
                    team=self.team_num,
                    player=self.player_num,
                    pos=self.position,
                    fallen=self.fallen,
                    ball_pos=self.ball_pos,
                    ball_age=self.ball_age,
                    name=self.name,
                    role=self.role,
                    teamballx=self.teamball_pos[0],
                    teambally=self.teamball_pos[1],
                    teamballconfidence=self.teamball_confidence,
                    walktargetx=self.reactivewalk_walktarget_pos[0],
                    walktargety=self.reactivewalk_walktarget_pos[1],
                    walktargetalpha=self.reactivewalk_walktarget_alpha,
                    goaltargetx=self.reactivewalk_goaltarget_pos[0],
                    goaltargety=self.reactivewalk_goaltarget_pos[1],
                    nearest=self.is_nearest_to_ball,
                    battery=self.battery,
                    obs=obs
                )
                )


class FlatSPLMessage(BembelSPLMessage):
    def __init__(self):
        super().__init__()
        self.position_conf = -1   # In % (int)
        self.ball_conf = -1       # In % (int)

    @classmethod
    def fromPacket(cls, packet):
        flat_msg = cls()

        try:
            prefix = unpack_from("4s", packet)
        except Exception as e:
            print("Could not unpack team message:", e)
            return flat_msg

        if prefix[0] == SPL_HEADER:
            print("Received SPL_HEADER prefixed message. Using old teamCom parser")
            return super().fromPacket(packet)

        try:
            # Using 4 byte offset -> skipping over the size prefix.
            spl_message = bbapi.TeamMessage.TeamMessageT.InitFromObj(bbapi.TeamMessage.TeamMessage.GetRootAs(packet, 4))
        except Exception as e:
            print("Could not deserialize flat buffer packet:", e)
            return flat_msg

        flat_msg.name = Constants.ROBOT_NAMES[spl_message.name]
        flat_msg.player_num = spl_message.playerNum

        flat_msg.battery = spl_message.battery

        flat_msg.role = spl_message.role

        flat_msg.fallen = spl_message.fallen
        if spl_message.position is not None:
            pos = spl_message.position
            flat_msg.position = [pos.x, pos.y, pos.a / 1000]
        flat_msg.position_conf = spl_message.posConf

        if spl_message.ball is not None:
            ball_pos = spl_message.ball
            flat_msg.ball_pos = [ball_pos.x, ball_pos.y]
        flat_msg.ball_age = spl_message.ballAge / 1000
        flat_msg.ball_conf = spl_message.ballConf / 100

        if spl_message.teamBall is not None:
            teamball_pos = spl_message.teamBall
            flat_msg.teamball_pos = [teamball_pos.x, teamball_pos.y]
        flat_msg.teamball_confidence = spl_message.teamBallConf / 100
        flat_msg.is_nearest_to_ball = spl_message.isNearestToBall

        if spl_message.walktarget is not None:
            walktarget = spl_message.walktarget
            flat_msg.reactivewalk_walktarget_pos = [walktarget.x/1000, walktarget.y/1000]
            flat_msg.reactivewalk_walktarget_alpha = walktarget.a / 1000

        if spl_message.goaltarget is not None:
            goaltarget = spl_message.goaltarget
            flat_msg.reactivewalk_goaltarget_pos = [goaltarget.x/1000, goaltarget.y/1000]

        if spl_message.obstacles is not None:
            flat_msg.reactivewalk_obstacles = [[obstacle.x/1000, obstacle.y/1000] for obstacle in spl_message.obstacles]
        flat_msg.reactivewalk_obstacle_count = len(flat_msg.reactivewalk_obstacles)

        return flat_msg

    def __str__(self):
        obs = ""
        for o in self.reactivewalk_obstacles:
            obs += "(" + str(o[0]) + "," + str(o[1]) + ")"

        return ("[{team}] FlatSPLMessage {player} at ({pos[0]:.2f}, {pos[1]:.2f}, {pos[2]:.2f}) with confidence {pos_conf}:"
                "Fallen = {fallen}, Ball at ({ball_pos[0]:.2f}, {ball_pos[1]:.2f}) since {ball_age:.2f} with confidence {ball_conf:.2f}\n"
                "\t name={name}, role={role}, teamball=({teamballx},{teambally}), confidence={teamballconfidence}\n"
                "\t walktarget=({walktargetx},{walktargety}@{walktargetalpha})\n"
                "\t goaltarget=({goaltargetx},{goaltargety}), nearesttoball={nearest}, battery={battery}%\n"
                "\t obstacles={obs}".format(
                    team=self.team_num,
                    player=self.player_num,
                    pos=self.position,
                    pos_conf=self.position_conf,
                    fallen=self.fallen,
                    ball_pos=self.ball_pos,
                    ball_age=self.ball_age,
                    ball_conf=self.ball_conf,
                    name=self.name,
                    role=Constants.ROBOT_ROLES[self.role],
                    teamballx=self.teamball_pos[0] if self.teamball_pos is not None else -1,
                    teambally=self.teamball_pos[1] if self.teamball_pos is not None else -1,
                    teamballconfidence=self.teamball_confidence,
                    walktargetx=self.reactivewalk_walktarget_pos[0] if self.reactivewalk_walktarget_pos is not None else -1,
                    walktargety=self.reactivewalk_walktarget_pos[1] if self.reactivewalk_walktarget_pos is not None else -1,
                    walktargetalpha=self.reactivewalk_walktarget_alpha,
                    goaltargetx=self.reactivewalk_goaltarget_pos[0] if self.reactivewalk_goaltarget_pos is not None else -1,
                    goaltargety=self.reactivewalk_goaltarget_pos[1] if self.reactivewalk_goaltarget_pos is not None else -1,
                    nearest=self.is_nearest_to_ball,
                    battery=self.battery,
                    obs=obs
                )
                )


class AsyncSPLParser:
    def __init__(self, callback):
        super().__init__()
        self.callback = callback

    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, packet, addr):
        splmessage = FlatSPLMessage.fromPacket(packet)
        self.callback(splmessage)


class AsyncSPLReceiver(asyncio.DatagramProtocol):
    def __init__(self, port, callback):
        super().__init__()
        self.port = port
        self.callback = callback

    def run(self):
        loop = asyncio.new_event_loop()
        listen = loop.create_datagram_endpoint(
            lambda: AsyncSPLParser(self.callback), local_addr=('0.0.0.0', self.port))
        loop.run_until_complete(listen)
        loop.run_forever()


class GCRobotInfo:

    penalties = {
        0: None,
        1: "Illegal ball contact",
        2: "Pushing",
        3: "Motion in SET",
        4: "Fallen/inactive robot",
        5: "Illegal position",
        6: "Leaving the field",
        7: "Request for pickup",
        8: "Local game stuck",
        9: "Illegal position in SET",
        14: "Substitute",
        15: "Manually penalized"
    }

    def __init__(self):
        self.penalty = ""
        self.secondsUntilUnpenalised = 0
        self.penaltyTimeRemaining = ""

    @classmethod
    def parse(cls, packet):

        info = cls()

        values = unpack_from("BB", packet)

        info.penalty = cls.penalties.get(values[0], "Unknown penalty")
        info.secondsUntilUnpenalised = values[1]
        info.penaltyTimeRemaining = GCMessage.secondsToString(info.secondsUntilUnpenalised)

        return info


class GCTeamInfo:

    COMMON_SIZE = 8
    SIZE = COMMON_SIZE+2*Constants.NUM_PLAYERS

    teamColors = {
        0: (0, 0, 255),       # blue / cyan
        1: (255, 0, 50),      # red / magenta / pink
        2: (255, 255, 0),     # yellow
        3: (0, 0, 0),         # black / dark gray
        4: (255, 255, 255),   # white
        5: (0, 255, 0),       # green
        6: (255, 100, 0),     # orange
        7: (150, 0, 255),     # purple / violet
        8: (255, 120, 120),   # brown
        9: (170, 170, 170)    # light gray
    }

    def __init__(self):
        self.teamNumber = 0
        self.teamColor = (0, 0, 0)
        self.score = 0
        self.penaltyShotCounter = 0
        self.singleShots = 0
        self.messageBudget = 0
        self.robots = []

    @classmethod
    def parse(cls, packet):
        values = unpack_from("4B2H", packet)

        info = cls()

        info.teamNumber = values[0]

        colorId = values[1]
        info.teamColor = cls.teamColors.get(colorId, (0, 0, 0))

        info.score = values[2]
        info.penaltyShotCounter = values[3]

        info.singleShots = values[4]
        info.messageBudget = values[5]

        offset = info.COMMON_SIZE
        for i in range(Constants.NUM_PLAYERS):
            info.robots.append(GCRobotInfo.parse(packet[offset:(offset+2)]))
            offset += 2

        return info


class GCMessage:

    gamePhases = {
        0: None,
        1: "Penalty shootout",
        2: "Overtime",
        3: "Timeout"
    }

    gameStates = {
        0: "Initial",
        1: "Ready",
        2: "Set",
        3: "Play",
        4: "Finished"
    }

    setPlays = {
        0: None,
        1: "Goal kick",
        2: "Pushing free kick",
        3: "Corner kick",
        4: "Kick in",
        5: "Penalty kick"
    }

    competitionPhases = {
        0: "Round-Robin",
        1: "Playoff"
    }

    competitionTypes = {
        0: "Normal",
        1: "Dynamic Ball Handling Challenge"
    }

    def __init__(self):
        self.competitionPhase = ""
        self.competitionType = ""
        self.gamePhase = None,
        self.gameState = "",
        self.currentSetPlay = None,
        self.half = ""
        self.kickingTeam = -1
        self.secondsRemaining = -1
        self.secondsRemainingSecondary = -1
        self.timeRemaining = ""
        self.timeRemainingSecondary = ""
        self.teams = []

    @staticmethod
    def secondsToString(secs):
        if secs >= 0:
            return "{:01d}:{:02d}".format(secs//60, secs % 60)
        else:
            return "-{:01d}:{:02d}".format((-secs)//60, (-secs) % 60)

    @classmethod
    def fromPacket(cls, packet):
        values = unpack_from("4s10B2h", packet)
        gc = cls()

        gc.competitionPhase = cls.competitionPhases.get(values[4], "")
        gc.competitionType = cls.competitionTypes.get(values[5], "")
        gc.gamePhase = cls.gamePhases.get(values[6], None)
        gc.gameState = cls.gameStates.get(values[7], "")
        gc.currentSetPlay = cls.setPlays.get(values[8], None)
        gc.half = "First" if values[9] else "Second"
        gc.kickingTeam = values[10]
        gc.secondsRemaining = values[11]
        gc.timeRemaining = cls.secondsToString(gc.secondsRemaining)
        gc.secondsRemainingSecondary = values[12]
        gc.timeRemainingSecondary = cls.secondsToString(gc.secondsRemainingSecondary)

        offset = 18
        for i in [0, 1]:
            gc.teams.append(GCTeamInfo.parse(packet[offset:(offset+GCTeamInfo.SIZE)]))
            offset += GCTeamInfo.SIZE

        return gc

    def __str__(self):
        return ("GCMessage: gamePhase={phase}, gameState={state}, setPlay={setPlay}\n"
                "\t half={half}, remaining={Remaining}, secondaryRemaining={secondaryRemaining}\n"
                "\t score={scoreOne}:{scoreTwo}".format(
                    phase=self.gamePhase,
                    state=self.gameState,
                    setPlay=self.currentSetPlay,
                    half=self.half,
                    Remaining=self.timeRemaining,
                    secondaryRemaining=self.timeRemainingSecondary,
                    scoreOne=self.teams[0].score,
                    scoreTwo=self.teams[1].score,
                )
                )


class AsyncGCParser:
    def __init__(self, callback):
        super().__init__()
        self.callback = callback

    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, packet, addr):
        message = GCMessage.fromPacket(packet)
        self.callback(message)


class AsyncGCReceiver(asyncio.DatagramProtocol):
    def __init__(self, callback):
        super().__init__()
        self.callback = callback

    def run(self):
        loop = asyncio.new_event_loop()
        listen = loop.create_datagram_endpoint(
            lambda: AsyncGCParser(self.callback), local_addr=('0.0.0.0', 3838))
        loop.run_until_complete(listen)
        loop.run_forever()


class SPLReceiver(QtCore.QThread):
    message = QtCore.pyqtSignal(SPLMessage)

    def __init__(self, port):
        super().__init__()
        self.port = port
        self.client = None

    def spl_message_callback(self, splmessage):
        # Set team number manually, as it is not send with the flat buffer team com.
        splmessage.team_num = self.port - 10000
        print(splmessage)
        self.message.emit(splmessage)

    def run(self):
        self.client = AsyncSPLReceiver(self.port, self.spl_message_callback)
        self.client.run()


class GCReceiver(QtCore.QThread):
    message = QtCore.pyqtSignal(GCMessage)

    def __init__(self):
        super().__init__()
        self.client = None

    def gc_message_callback(self, splmessage):
        self.message.emit(splmessage)

    def run(self):
        self.client = AsyncGCReceiver(self.gc_message_callback)
        self.client.run()
