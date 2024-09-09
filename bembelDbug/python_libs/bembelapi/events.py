from enum import Enum


class EventType(Enum):
    pass


class NaoMessage:
    def __init__(self, message, addr):
        self.message = message
        self.addr = addr


class EventEmitter:
    def __init__(self):
        self._listeners = {}

    def emit(self, event, *args):
        event_listeners = self._listeners.get(event, set())

        for listener in event_listeners:
            listener(*args)

    def add_listener(self, event, obj: object):
        handle_method = obj

        if not callable(handle_method):
            handle_method = self._get_handler(event, obj)

        if not handle_method or not callable(handle_method):
            raise Exception(obj.__class__.__name__ + " missing handler %s" % self._get_handler_name(event))

        event_listeners = self._listeners.get(event, set())
        event_listeners.add(handle_method)
        self._listeners[event] = event_listeners

    def remove_listner(self, event, obj: object):
        event_listeners = self._listeners.get(event, set())
        try:
            event_listeners.remove(obj)
        except KeyError:
            pass

    def _get_handler(self, event, obj: object):
        return getattr(obj, self._get_handler_name(event), None)

    def _get_handler_name(self, event: str):
        if isinstance(event, EventType):
            event = event.name
        return "handle_" + event.lower()
