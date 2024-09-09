from typing import Optional

from debugger.bembeldbug import BembelDbug


class BembelDbugFactory:
    __instance = None  # type: Optional[BembelDbug]

    @classmethod
    def get(cls) -> Optional[BembelDbug]:
        if cls.__instance is None:
            cls.__instance = BembelDbug()
        return cls.__instance
