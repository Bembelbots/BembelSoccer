from typing import Optional

from bembelapi.naodebug import NaoDebug


class NaoDebugFactory:
    __naodebug = None  # type: Optional[NaoDebug]

    @classmethod
    def get_debugger(cls) -> Optional[NaoDebug]:
        if cls.__naodebug is None:
            cls.__naodebug = NaoDebug()
        return cls.__naodebug