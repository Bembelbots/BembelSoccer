import queue
import sys
import threading

import time

from bembelapi.events import EventEmitter
from bembelapi.message_handler import MessageHandlerEvent
from bembelapi.naomodule import NaoPlugin


class ServerManager(EventEmitter):
    _servers = []
    _modules = []
    _plugins = []

    def __init__(self):
        super().__init__()
        self.queue = queue.Queue()

    def register_plugin(self, plugin: NaoPlugin):
        self._plugins.append(plugin)

    def start_in_thread(self):
        thread = threading.Thread(target=self.start)
        thread.daemon = True
        thread.start()

    def start(self):
        modules = self._modules + self._plugins

        self._start_servers()
        self._setup_modules(modules)

        try:
            self._run(modules)
        except Exception as e:
            print(e)
        except KeyboardInterrupt:
            pass
        finally:
            self._cleanup(modules)

    def _run(self, modules):
        while True:
            self._tick(modules)
            self._dispatch_events()
            time.sleep(0.01)

    def _tick(self, modules):
        for module in modules:
            module.tick()

    def _cleanup(self, modules):
        for server in self._servers:
            server.server_close()
        for module in modules:
            module.shutdown()
        sys.exit()

    def _start_servers(self):
        for server in self._servers:
            thread = threading.Thread(target=server.serve_forever)
            thread.daemon = True
            thread.start()

    def _setup_modules(self, modules):
        for module in modules:
            module.setup(self)

    def _dispatch_events(self):
        try:
            while not self.queue.empty():
                event = self.queue.get_nowait()  # type: MessageHandlerEvent
                event.dispatch(self)
        except Exception as e:
            print(e)
