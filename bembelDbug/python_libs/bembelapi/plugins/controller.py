import pygame
import time

from bembelapi.naomodule import NaoPlugin


class NaoControllerPlugin(NaoPlugin):
    def __init__(self):
        super().__init__()
        self.activity_timestamp = 0

    def setup(self, parent):
        super().setup(parent)
        pygame.display.init()
        pygame.event.set_blocked(pygame.MOUSEMOTION)
        pygame.joystick.init()

    def _connect_joystick(self):
        self.joystick_init = False
        pygame.joystick.quit()
        pygame.joystick.init()
        for x in range(pygame.joystick.get_count()):
            self.joystick_init = True
            joystick = pygame.joystick.Joystick(x)
            joystick.init()

        self.activity_timestamp = time.time()

        if self.joystick_init:
            pygame.event.clear()

    def tick(self):
        if pygame.joystick.get_count() == 0 or (time.time() - self.activity_timestamp) >= 5:
            self._connect_joystick()
            return

        has_joy_event = False

        for event in pygame.event.get():

            if event.type not in [pygame.JOYBUTTONUP, pygame.JOYBUTTONDOWN, pygame.JOYAXISMOTION, pygame.JOYHATMOTION]:
                continue

            has_joy_event = True

            key = None
            value = None

            event_name = ""

            if event.type == pygame.JOYBUTTONUP:
                event_name = "joybutton_up"
                key = event.dict['button']
            elif event.type == pygame.JOYBUTTONDOWN:
                event_name = "joybutton_down"
                key = event.dict['button']
            elif event.type == pygame.JOYHATMOTION:
                event_name = "joyhat_motion"
                key = event.dict['hat']
                value = event.dict['value']
            elif event.type == pygame.JOYAXISMOTION:
                event_name = "joyaxis_motion"
                key = event.dict['axis']
                value = event.dict['value']

            if value is None:
                self.parent.emit(event_name, key)
            else:
                self.parent.emit(event_name, (key, value))

        if has_joy_event:
            self.activity_timestamp = time.time()
