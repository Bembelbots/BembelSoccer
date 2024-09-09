import queue
import socket
import socketserver
import threading
import time


class NaoUdpServer(socketserver.UDPServer):
    allow_reuse_address = True

    max_packet_size = 65515

    server_address = None

    def __init__(self, parent, request_handler_class, bind_and_activate=True):
        if self.server_address is None:
            raise NotImplementedError

        self.parent = parent
        self.queue = parent.queue

        super().__init__(self.server_address, request_handler_class, False)

        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

        if bind_and_activate:
            try:
                self.server_bind()
                self.server_activate()
            except:
                self.server_close()
                raise

"""
class NaoTcpServer(socketserver.TCPServer):
    allow_reuse_address = True

    server_address = None

    def __init__(self, parent, request_handler_class, bind_and_activate=True):
        if self.server_address is None:
            raise NotImplemented

        self.parent = parent
        self.queue = parent.queue

        super().__init__(self.server_address, request_handler_class, bind_and_activate)
"""

class NaoClientThread(threading.Thread):
    def __init__(self, server, address):
        super().__init__()
        self._parent = server
        self.queue = self._parent.queue
        self._address = address
        self._running = False
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 10485760)

        self._socket.settimeout(10)

    def run(self):
        self._running = True
        self._socket.connect(self._address)

        inst = self._parent.message_handler(self._socket, self._address, self)

        while self._running:
            inst.handle()
            time.sleep(0.001)

    def stop(self):
        self._socket.close()
        self._running = False

class NaoTcpClient:
    def __init__(self, parent, message_handler):
        self.parent = parent
        self.queue = parent.queue
        self.message_handler = message_handler
        self.recv_thread = None # type: threading.thread

    def connect(self, address):
        self.disconnect()
        self.recv_thread = NaoClientThread(self, address)
        self.recv_thread.start()

    def disconnect(self):
        if self.recv_thread:
            self.recv_thread.stop()
            self.recv_thread = None

    


class NaoDebugUdpServer(NaoUdpServer):
    server_address = ("0.0.0.0", 0)
    broadcast_port = 10350

    def __init__(self, parent, request_handler_class):
        super().__init__(parent, request_handler_class, True)


"""
class NaoDebugTcpServer(NaoTcpServer):
    server_address = ("0.0.0.0", 10350)
"""

class NaoSplMessageServer(NaoUdpServer):
    max_packet_size = 3096

    def __init__(self, parent, request_handler_class, team_number: int):
        self.server_address = ("0.0.0.0", 10000 + team_number)
        super().__init__(parent, request_handler_class, True)
