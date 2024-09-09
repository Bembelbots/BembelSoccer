import queue
import socket


from bootstrap import bootstrap
bootstrap()

from bembelapi.naodebug import NaoDebug
from bembelapi.config import NaoEvent
from bembelapi.messages import ChangeValueMessage
from bembelapi.nao import NaoInfo
from bembelapi.naoconnection import NaoConnection
from bembelapi.naodebugfactory import NaoDebugFactory
from bembelapi.naomodule import NaoPlugin
from bembelapi.plugins.controller import NaoControllerPlugin


class ControllerMapper(NaoPlugin):
    STAND = 0
    TIPPLE = 1
    WALK_FORWARD = 2
    WALK_BACKWARD = 3
    TURN_L = 4
    TURN_R = 5
    STRAFE_L = 6
    STRAFE_R = 7
    KICK = 9
    HEAD_SWEEP = 12
    HEAD_STATIC = 13

    _blackboard = "BehaviorBlackboard"

    def __init__(self):
        super().__init__()
        self._enabled = False
        self._message_queue = queue.Queue()

    def setup(self, parent: NaoDebug):
        super().setup(parent)
        parent.add_listener("joybutton_up", self)
        parent.add_listener("joybutton_down", self)
        parent.add_listener("joyaxis_motion", self)
        parent.add_listener("joyhat_motion", self)

    def handle_joybutton_up(self, button):
        if button == 7:
            self._enabled ^= True
            if self._enabled:
                self._message_queue.put(ChangeValueMessage(self._blackboard, "test_mode", 1))
                self._message_queue.put(ChangeValueMessage(self._blackboard, "test_start", 1))
            else:
                self._message_queue.put(ChangeValueMessage(self._blackboard, "test_mode", 0))
        elif button == 2:
            self._message_queue.put(ChangeValueMessage(self._blackboard, "remote_comm", self.TIPPLE))
        elif button == 1:
            self._message_queue.put(ChangeValueMessage(self._blackboard, "remote_comm", self.STAND))
        elif button == 4:
            self._message_queue.put(ChangeValueMessage(self._blackboard, "remote_comm", self.STRAFE_L))
        elif button == 5:
            self._message_queue.put(ChangeValueMessage(self._blackboard, "remote_comm", self.STRAFE_R))
        elif button == 0:
            self._message_queue.put(ChangeValueMessage(self._blackboard, "remote_comm", self.KICK))

    def handle_joybutton_down(self, button):
        pass

    def handle_joyaxis_motion(self, axis_motion):
        axis, offset = axis_motion
        pass

    def handle_joyhat_motion(self, hat_motion):
        hat, offset = hat_motion
        if offset == (0, 1):
            self._message_queue.put(ChangeValueMessage(self._blackboard, "remote_comm", self.WALK_FORWARD))
        elif offset == (-1, 0):
            self._message_queue.put(ChangeValueMessage(self._blackboard, "remote_comm", self.TURN_L))
        elif offset == (1, 0):
            self._message_queue.put(ChangeValueMessage(self._blackboard, "remote_comm", self.TURN_R))

    def tick(self, nao_connection: NaoConnection, _socket: socket.socket, port: int):
        if nao_connection is None:
            self._enabled = False
            self._message_queue.queue.clear()
            return

        while not self._message_queue.empty():
            message = self._message_queue.get_nowait()
            print(message)
            nao_connection.send(message)


class NaoConnect(NaoPlugin):
    def tick(self, nao_connection: NaoConnection, _socket: socket.socket, port: int):
        if nao_connection is None:
            self.parent.emit(NaoEvent.NAO_CONNECT, "headbanger")


def nao_connected(nao: NaoConnection):
    print("%s connected" % nao.info().name)
    # nao.receive_symbol("VisionData", "_bottomRawImage", 1)
    # nao.receive_symbol("VisionData", "_topRawImage", 1)
    # nao.receive_symbol("VisionData", "highQuality", 1)
    # nao.send(ChangeSymbolMessage("CameraParameter", "autoExposure", 1))
    # nao.send(ChangeSymbolMessage("CameraParameter", "autoWhiteBalancing", 1))
    # nao.receive_symbol("CameraParameter", "exposureTop", 1)


def nao_disconnected(nao: NaoConnection):
    print("%s disconnected" % nao.info().name)


def nao_found(nao: NaoInfo):
    print("%s found" % nao.name)


def nao_lost(nao: NaoInfo):
    print("%s lost" % nao.name)


def message_symbol_values(message: dict, sender: (str, int)):
    print(message, sender)


def message_symbol_names(message: dict, sender: (str, int)):
    print(message, sender)


if __name__ == '__main__':
    debugger = NaoDebugFactory.get_debugger()
    debugger.register_plugin(NaoControllerPlugin())
    debugger.register_plugin(ControllerMapper())
    debugger.register_plugin(NaoConnect())
    debugger.start()
