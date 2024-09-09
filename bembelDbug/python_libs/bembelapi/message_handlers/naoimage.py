import socket
import threading

import time
from struct import unpack_from

from bembelapi.config import NaoEvent
from bembelapi.events import EventEmitter
from bembelapi.message_handler import MessageHandler, MessageHandlerEvent
from bembelapi.nao import NaoCamImage


class NaoImageEvent(MessageHandlerEvent):
    def __init__(self, data: NaoCamImage, client_address: (str, int), _socket: socket):
        super().__init__(client_address, _socket)
        self.data = data

    def dispatch(self, event_emitter: EventEmitter):
        # TODO: update signature
        event_emitter.emit(NaoEvent.NAO_IMAGE, self.data, self.client_address)


class NaoImageHandler(MessageHandler):
    _IMAGE_HEADER_IDENTIFIER = b"BBimage\0"
    _IMAGE_HEADER_VERSION = 2
    _IMAGE_HEADER_LENGTH = 32
    _READ_SIZE = 100

    def __init__(self, request, client_address, server):
        super().__init__(request, client_address, server)

    def _recv_data(self, client, buffer_size):
        part = None
        try:
            part = client.recv(buffer_size)

            if len(part) == 0:
                part = None
        except socket.timeout:
            raise socket.timeout
        except Exception as e:
            print("image receive error: %s" % str(e))

        # invalid data
        if part is None:
            print("No data recv. Connection is dead.")
            self.server._running = False
            self.request.close()
            return b""

        return part

    def handle(self):
        msg = b""

        while self.server._running:
            # read some data (this will usually start with header, but we are being extra paranoid)
            try:
                data = self._recv_data(self.request, 2*self._IMAGE_HEADER_LENGTH)
            except socket.timeout:
                continue

            # search for header magic within a sliding window of 4*self._IMAGE_HEADER_LENGTH
            msg = msg[-2*self._IMAGE_HEADER_LENGTH:] + data
            offset = msg.find(self._IMAGE_HEADER_IDENTIFIER)
            if offset < 0 or len(msg)-offset < self._IMAGE_HEADER_LENGTH:
                # header not found of incomplete
                continue

            # parse header
            msg = msg[offset:]
            (header, version, camera, codec, tick, size, vrSize) = unpack_from("8sBBBIQQ", msg)
            
            # sanity checks
            if header != self._IMAGE_HEADER_IDENTIFIER:
                # this should not happen, something went really wrong
                print("NaoImageHandler: magic code in header does not match, parser is broken!")
                continue
            
            if version != self._IMAGE_HEADER_VERSION:
                print("NaoImageHandler: skipping incompatible image (header version mismatch)")
                continue

            msg = msg[self._IMAGE_HEADER_LENGTH:]

            while len(msg) < vrSize:
                try:
                    msg += self._recv_data(self.request, vrSize - len(msg))
                except socket.timeout:
                    continue
            
            # read remaining image data
            image = NaoCamImage(codec, camera, tick, size)
            
            if vrSize > 0:
                image.set_vision_results(msg)

            msg = msg[vrSize:] 
            while len(msg) < size:
                try:
                    msg += self._recv_data(self.request, size - len(msg))
                except socket.timeout:
                    continue

            # set image data and emit event
            image.set_data(msg)
            self._emit(NaoImageEvent(image, self.client_address, self.request))
            
            # clear data
            msg = b""

        # main loop done
        self.request.close()
