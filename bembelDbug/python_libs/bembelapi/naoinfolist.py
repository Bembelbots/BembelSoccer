import time

from bembelapi.config import NaoEvent
from bembelapi.events import EventEmitter
from bembelapi.nao import NaoInfo


class NaoInfoList:
    def __init__(self, event_emitter: EventEmitter = None):
        self._naos = {}
        self._nao_address_map = {}
        self._event_emitter = event_emitter

    @property
    def keys(self):
        return self._naos.keys()

    @property
    def values(self):
        return self._naos.keys()

    def get(self, robot_name, default=None) -> NaoInfo:
        return self._naos.get(robot_name, default)

    def get_by_ip(self, address, default=None):
        return self._nao_address_map.get(address, default)

    def has(self, robot_name):
        return self.get(robot_name) is not None

    def update(self, nao: NaoInfo):
        if self._naos.get(nao.name) is not None:
            self._event_emitter.emit(NaoEvent.NAO_UPDATE, nao)
            nao.touch()
            return

        if self._event_emitter is not None:
            self._event_emitter.emit(NaoEvent.NAO_FOUND, nao)

        self._naos[nao.name] = nao
        self._nao_address_map[nao.client[0]] = nao

    def tick(self):
        now = time.time()
        current_nao_list = self._naos.copy()
        for key, nao in current_nao_list.items():
            if not nao.is_expired(now):
                continue
            self._naos.pop(key)
            self._nao_address_map.pop(nao.client[0])

            if self._event_emitter is not None:
                self._event_emitter.emit(NaoEvent.NAO_LOST, nao)

    def __len__(self):
        return len(self._naos)

    def __repr__(self):
        return str(self._naos)
