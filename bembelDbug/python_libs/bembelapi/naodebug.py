from typing import Optional

from bembelapi.connection_manager import NaoConnectionManager
from bembelapi.message_handlers.splmessage import SplMessageHandler
from bembelapi.modules.findnao import FindNao
from bembelapi.modules.heartbeat import NaoHeartbeat
from bembelapi.modules.symbols import NaoSymbolsListener
from bembelapi.modules.save_blackboards import SaveBlackboardsListener
from bembelapi.message_handlers.naoimage import NaoImageHandler
from bembelapi.message_handlers.naomessage import NaoMessageHandler
from bembelapi.naoconnection import NaoConnection
from bembelapi.naoinfolist import NaoInfoList
from bembelapi.server import NaoDebugUdpServer, NaoTcpClient, NaoSplMessageServer
from bembelapi.server_manager import ServerManager

TEAM_NUMBER = 3


class NaoDebug(ServerManager):
    def __init__(self):
        super().__init__()

        self._debug_server = NaoDebugUdpServer(self, NaoMessageHandler)
        self._image_server = NaoTcpClient(self, NaoImageHandler)
        #self._spl_message_server = NaoSplMessageServer(self, SplMessageHandler, TEAM_NUMBER) # this breaks standalone TeamViewer!

        self._connection_manager = NaoConnectionManager(self, self._debug_server)

        self._servers = [
            self._debug_server,
            #self._spl_message_server
        ]

        self._nao_finder = FindNao(self._debug_server, self._connection_manager.nao_list)

        self._modules = [
            self._nao_finder,
            NaoHeartbeat(self._debug_server, self._connection_manager),
            NaoSymbolsListener(self._connection_manager),
            SaveBlackboardsListener(self._connection_manager),
        ]

    def add_nao_ip(self, ip):
        self._nao_finder.add_search_ip(ip)

    def remove_nao_ip(self, ip):
        self._nao_finder.remove_search_ip(ip)

    def clear_nao_ips(self):
        self._nao_finder.clear_search_ips()

    @property
    def connection(self) -> Optional[NaoConnection]:
        return self._connection_manager.connection

    @property
    def nao_list(self) -> NaoInfoList:
        return self._connection_manager.nao_list
