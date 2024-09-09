import json
import queue
from enum import Enum

from copy import deepcopy

from util import read_file, write_file

PROJECT_NAME = "BembelDbug"

# used for QSettings
APPLICATION_NAME = PROJECT_NAME
ORGANIZATION_NAME = 'Bembelbots'
ORGANIZATION_DOMAIN = 'bembelbots.de'



class NaoStatusColors(Enum):
    STATUS_CONNECTED = 'background-color: rgb(9, 100, 125)'
    STATUS_CONNECTING = 'background-color: rgb(128, 128, 128)'
    STATUS_DISCONNECTED = 'background-color: rgb(166, 113, 37)'
    STATUS_BUSY = 'background-color: rgb(128, 128, 128)'


class Config:
    def __init__(self, path: str = None, default_config_path: str = None):
        self._path = path
        self._default_config_path = default_config_path
        self.data = {}
        self._default_config = {}
        self._load()

    def save(self):
        delta = self._calculate_delta(self._default_config, self.data)
        write_file(self._path, json.dumps(delta, indent=2))

    def _load(self):
        if self._default_config_path:
            self._default_config = self._load_json(self._default_config_path)

        if self._path:
            self.data = self._load_json(self._path)
            self._merge(self.data, self._default_config)

    def __str__(self):
        return json.dumps(self.data, indent=2)

    @staticmethod
    def _load_json(path: str) -> dict:
        json_str = "{}"

        try:
            json_str = read_file(path)
        except FileNotFoundError:
            pass

        return json.loads(json_str)

    @staticmethod
    def _calculate_delta(dict1: dict, dict2: dict) -> dict:
        q = queue.deque()
        index = "delta"

        tmp_dict = {index: {}}
        delta = tmp_dict[index]

        q.append((tmp_dict, index, dict1, dict2))

        while len(q) > 0:
            offset, index, base, current = q.popleft()

            for key in current.keys():
                if offset.get(index) is None:
                    offset[index] = {}

                base_has_key = base.get(key) is not None

                if not base_has_key:
                    offset[index][key] = current[key]
                    continue

                if isinstance(current[key], dict) and isinstance(base[key], dict):
                    q.append((offset[index], key, base[key], current[key]))
                    continue

                if current[key] != base[key]:
                    offset[index][key] = current[key]

                if len(offset[index]) == 0:
                    del offset[index]

        for key in list(delta.keys()):
            if len(delta[key]) == 0:
                del delta[key]

        return delta

    @staticmethod
    def _merge(dict1: dict, dict2: dict) -> None:
        q = queue.deque()
        q.append((dict1, dict2))

        while len(q) > 0:
            inst, obj = q.popleft()
            for key in obj.keys():

                if inst.get(key) is None:
                    inst[key] = deepcopy(obj[key])
                    continue

                if isinstance(obj[key], dict) and isinstance(inst[key], dict):
                    q.append((inst[key], obj[key]))
                    continue
