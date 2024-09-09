import socket

from bembelapi.messages import (
        ChangeSymbolMessage, 
        ChangeValueMessage, 
        SaveBlackboardsMessage,
    )

from bembelapi.nao import NaoInfo, RobotHeartbeat, RobotConfig

class NaoConnection:
    """
    Interface to the connected nao.
    """ 
    def __init__(self, nao_info: NaoInfo, socket : socket.socket, server):
        self._info = nao_info
        self._server = server
        self._socket = socket
        self._symbol_names = {}
        self._heartbeat = RobotHeartbeat()

    @property
    def name(self) -> str:
        return self._info.name

    @property
    def info(self) -> NaoInfo:
        return self._info

    @property
    def config(self) -> RobotConfig:
        return self._info.config

    @property
    def symbol_names(self):
        return self._symbol_names

    @symbol_names.setter
    def symbol_names(self, symbol_names):
        self._symbol_names = symbol_names

    def is_connected(self):
        return self._heartbeat.is_connected() and not self._info.free and self._info.debug_address is not None

    def is_frontend_running(self):
        return self._info.config.frontendRunning == 1

    def send(self, message):
        self._socket.sendto(str.encode(str(message)), self._info.debug_address)

    def start_receiving_symbol(self, blackboard:str, symbol:str):
        """
        Tell nao to start sending the value of a symbol on the blackboard.

        Handling of received values is asynchrone. To work with the received values 
        you have to start listening for the @NAO_EVENT@ NAO_SYMBOL_VALUES.

        :param blackboard: Name of the blackboard where the symbol is located.
        :param symbol    : Name of the symbol.
        """
        self.set_symbol_status(blackboard, symbol, True)

    def stop_receiving_symbol(self, blackboard:str, symbol:str):
        """
        Stop receiving the provided symbol. Reverts start_receiveing_symbol().
        Will do nothing if the symbol wasnt broadcasted in the first place.

        :param blackboard: Name of the blackboard where the symbol is located.
        :param symbol    : Name of the symbol.
        """
        self.set_symbol_status(blackboard, symbol, False)

    def set_symbol_status(self, blackboard:str, symbol:str, receive:bool):
        """
        Set wether the nao should send the specified value.

        Only use this function if the parameter receive value is not known beforehand.
        If you always want to receive the symbol or unsubscribe from it, use
        the functions start_receiving_symbol / stop_receiving_symbol instead for better 
        code readability.

        :param blackboard: Name of the blackboard where the symbol is located.
        :param symbol    : Name of the symbol.
        :param receive   : Wether the Nao should start or stop sending the symbol.
        """
        self._receive_symbol(blackboard, symbol, int(receive))

    def _receive_symbol(self, blackboard:str, key:str, value:int):
        blackboard_dict = self.symbol_names.get(blackboard, {})
        blackboard_dict[key] = str(value)
        self.symbol_names[blackboard] = blackboard_dict
        self.send(ChangeSymbolMessage(blackboard, key, value))

    def change_value(self, blackboard:str, key:str, value:str):
        self.change_values(blackboard, {key: value})

    def change_values(self, blackboard:str, values:dict):
        self.send(ChangeValueMessage(blackboard, values))

    def save_blackboards(self):
        self.send(SaveBlackboardsMessage())

    def update_heartbeat(self, heartbeat: RobotHeartbeat):
        self._heartbeat = heartbeat

    def get_heartbeat(self):
        return self._heartbeat

    def recv_images(self):
        self._server._image_server.connect((self._info.debug_address[0], 10350))
        return self._server._image_server

    def stop_recv_images(self):
        self._server._image_server.disconnect()

