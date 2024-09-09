import json
import struct

from bembelapi.config import NaoMonitorAction


class ChangeSymbolMessage:
    def __init__(self, blackboard, key, value):
        self._blackboard = blackboard
        self._key = key
        self._value = value

    def __str__(self):
        return json.dumps({
            'debugv2': {
                self._blackboard: {
                    self._key: self._value
                },
                'msg_type': 'change_symbol'
            }
        })


class ChangeValueMessage:
    def __init__(self, blackboard, values: dict):
        self._blackboard = blackboard
        self._values = values

    def __str__(self):
        return json.dumps({
            'debugv2': {
                self._blackboard: self._values,
                'msg_type': 'change_value'
            }
        })

class SaveBlackboardsMessage:
    
    def __str__(self):
        return json.dumps({
            'debugv2': {
                'msg_type': 'save_blackboards'
            }
        })


class MonitorMessage:
    def __init__(self, nao_name: str):
        self._data = b""
        self._set_name(nao_name)

    def _set_name(self, name: str):
        self._data = str.encode(name) + b"".join(b'\0' for _ in range(20-len(name)))

    def add(self, fmt, *args):
        self._data += struct.pack(fmt, *args)

    def add_str(self, value: str):
        self._data += value.encode("utf8")

    def get_bytes(self):
        return self._data


class MonitorConfigMessage(MonitorMessage):
    def __init__(self, nao_name: str, config: str):
        super().__init__(nao_name)
        self.add("i", NaoMonitorAction.SET_JSON.value)
        config_json = json.dumps({nao_name: config})
        self.add("i", len(config_json))
        self.add_str(config_json)


class MonitorCalibrationMessage(MonitorConfigMessage):
    # TODO: fix api to handle setting configuration separately
    def __init__(self, nao_name: str, config: str, calibration: str):
        super().__init__(nao_name, config)
        calibration_json = json.dumps({nao_name: calibration})
        self.add("i", len(calibration_json))
        self.add_str(calibration_json)
