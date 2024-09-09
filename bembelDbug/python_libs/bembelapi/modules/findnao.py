import json
import socket

import time
from bembelapi.config import NaoEvent, NaoMonitorAction
from bembelapi.nao import RobotConfig, NaoInfo
from bembelapi.naoinfolist import NaoInfoList
from bembelapi.naomodule import NaoModule
from bembelapi.server import NaoDebugUdpServer


class FindNao(NaoModule):
    _packet = {
        "debugv2": {
            "msg_type": "findnao"
        }
    }

    def __init__(self, server: NaoDebugUdpServer, nao_list: NaoInfoList):
        super().__init__()
        self.socket = server.socket
        self.port = server.server_address[1]
        self.broadcast_port = server.broadcast_port
        self._broadcast_toggle = False
        self._packet = str.encode(json.dumps(self._packet))
        self._event_emitter = None
        self.nao_list = nao_list
        self._last_sent = 0
        self.clear_search_ips()

    def add_search_ip(self, ip):
        if ip not in self._ip_list:
            self._ip_list.append(ip)

    def remove_search_ip(self, ip):
        self._ip_list.remove(ip)

    def clear_search_ips(self):
        self._ip_list = []

    def setup(self, parent):
        super().setup(parent)
        parent.add_listener(NaoEvent.NAO_CONFIG, self)
        parent.add_listener("message_hellodebug", self)

    def handle_nao_config(self, config: RobotConfig, client: (str, int), _socket: socket):
        robot_name = config.robotName.lower()
        nao = self.nao_list.get(robot_name, NaoInfo(robot_name, client, _socket))
        nao.config = config
        if nao.config.calibration is None or nao.config.configuration is None:
            nao.send_monitor(NaoMonitorAction.SEND_CONFIG, True)

        self.nao_list.update(nao)

    def handle_message_hellodebug(self, message: dict, client: (str, int), _socket: socket):
        robot_name = message["robotname"].lower()

        debug_client = None
        if message.get("debug_client", "") != "":
            addr_parts = message["debug_client"].split(":")
            debug_client = (addr_parts[0], int(addr_parts[1]))

        free = (message["connected"] == "false")

        nao = self.nao_list.get(robot_name, NaoInfo(robot_name, client, _socket))

        if nao is None:
            return

        nao.debug_client = debug_client
        nao.debug_address = client
        nao.free = free
        # TODO: Is this the right place for this?
        nao.config.frontendRunning = 1

        self.nao_list.update(nao)

    def tick(self):
        if time.time() - self._last_sent < 1:
            return

        import ipaddress
        from netifaces import interfaces, ifaddresses, AF_INET

        iterface_ips = ["127.0.0.1"]

        for interface in interfaces():
            for link in ifaddresses(interface).get(AF_INET, ()):
                # netifaces does not return correct bcast addresses for openvpn tap interface,
                # as workaround this code calculates the bcast address using ipaddress lib
                ip = link.get("addr")
                mask = link.get("netmask")
                net = ipaddress.IPv4Network(ip + '/' + mask, False)
                bcast = net.broadcast_address
                iterface_ips.append(str(bcast))

        ip_list = self._ip_list + iterface_ips
        for ip in ip_list:
            try:
                self.socket.sendto(self._packet, (ip, self.broadcast_port))
            except:
                pass

        self.nao_list.tick()
        self._last_sent = time.time()
