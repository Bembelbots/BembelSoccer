import socket

from bembelapi.naoconnection import NaoConnection


class NaoModule:
    def __init__(self):
        self.parent = None

    def setup(self, parent):
        self.parent = parent

    def tick(self):
        pass

    def shutdown(self):
        pass


class NaoPlugin(NaoModule):
    pass
