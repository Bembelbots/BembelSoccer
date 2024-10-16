from enum import Enum

from bembelapi.events import EventType

DEBUG_PORT = 10350
MONITOR_PORT = 10399

KEEPALIVE_INTERVAL = 1  # must be less than the timeout

ROBOT_ROLES = [
    "NONE",
    "STRIKER",
    "DEFENDER",
    "GOALKEEPER",
    "SUPPORTER_DEFENSE",
    "SUPPORTER_OFFENSE",
    "SEARCHER",
    "DEMO",
    "PENALTYKICKER",
    "PENALTYGOALY",
    "OBSTACLE_AVOIDER",
    "PASSING_ROBOT_OWN_HALF",
    "PASSING_ROBOT_OPPONENT_HALF",
    "ONE_VS_ONE_DUELLIST",
    "CALIBRATION_EVALUATOR",
]


class NaoEvent(EventType):
    NAO_CONFIG = 0
    NAO_FOUND = 1
    NAO_LOST = 2
    NAO_CONNECT = 3
    NAO_CONNECTED = 4
    NAO_DISCONNECTED = 5
    NAO_IMAGE = 7
    NAO_UPDATE = 8
    NAO_DISCONNECT = 9
    NAO_SYMBOL_NAMES = 10
    NAO_SYMBOL_VALUES = 11
    SPL_STANDARD_MESSAGE = 12
    NAO_SAVE_BLACKBOARDS_OK = 13
    NAO_SAVE_BLACKBOARDS_ERROR = 14


class NaoMonitorAction(Enum):
    BACKEND = 0
    FRONTEND = 1
    AUTO_RESTART = 2
    SEND_CONFIG = 3
    SET_JSON = 4
