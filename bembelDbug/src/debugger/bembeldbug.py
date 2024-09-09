from PyQt5.QtCore import QObject
from PyQt5.QtCore import pyqtSignal
from bembelapi.config import NaoEvent
from bembelapi.events import EventType
from bembelapi.naodebugfactory import NaoDebugFactory, NaoDebug
from bembelapi.naomodule import NaoPlugin


#  pyqt thread safety http://stackoverflow.com/a/33453124
from config import Config


class BembelDbug(QObject):
    event = pyqtSignal(str, tuple)

    def __init__(self):
        super().__init__()
        self._events = {}
        self.listeners = {}
        self._debugger = NaoDebugFactory.get_debugger() # type: NaoDebug
        self.event.connect(self.event_dispatcher)
        self._config = Config()

    @property
    def config(self):
        return self._config

    @property
    def nao_debug(self) -> NaoDebug:
        return self._debugger

    @config.setter
    def config(self, config: Config):
        self._config = config

    def add_listener(self, event, func: object):
        event_name = self._get_event_name(event)

        if not self._events.get(event_name, False):
            self._events[event_name] = True
            self.event.connect(self.event_handler)
            self._debugger.add_listener(event, self.event_handler(event_name))

        self.listeners[event_name] = self.listeners.get(event_name, set())
        self.listeners[event_name].add(func)

    def remove_listener(self, event, func: object):
        event_name = self._get_event_name(event)
        event_listeners = self.listeners.get(event_name, set())
        try:
            event_listeners.remove(func)
        except KeyError:
            pass

    def event_handler(self, event_name: str):
        def handler(*args):
            self.event.emit(event_name, args)
        return handler

    def event_dispatcher(self, name: str, args: tuple):
        for listener in self.listeners[name]:
            listener(*args)

    def connect_to_nao(self, name):
        self._debugger.emit(NaoEvent.NAO_CONNECT, name)

    def disconnect_from_nao(self, name):
        self._debugger.emit(NaoEvent.NAO_DISCONNECT, name)

    def _get_event_name(self, event):
        event_name = event
        if isinstance(event, EventType):
            event_name = event.name
        return event_name

    def register_plugin(self, plugin: NaoPlugin):
        self._debugger.register_plugin(plugin)

    def start(self):
        self._debugger.start_in_thread()

