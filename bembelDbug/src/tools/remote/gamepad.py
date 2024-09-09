import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"
import pygame
from enum import Enum

class CMD(Enum):
    NONE = 0
    stand = 1
    kick = 2
    stand_up = 3
    walk_forward = 4
    walk_backward = 5
    turn_left = 6
    turn_right = 7
    strafe_left = 8
    strafe_right = 9
    head_sweep = 12
    head_static = 13


class Input(Enum):
    NONE = 0
    BUTTON = 1
    HAT = 2
    AXIS = 3

class Gamepad:
    DEAD_ZONE = 0.25 # ignore axis movement less than 25%

    """
    Gamepad config:
        Dict of strings, which are matched against the start of the joystick's
        name (using startswith())

        There are 3 possible input types:
            - Input.BUTTON for button presses
            - Input.AXIS for movement of an analog stick
            - Input.HAT for movement of a "hat" / digital pad

        Each input type consists of a dict with the button/axis/hat index
        as key and commands as values.
        
        Buttons only have one action, axes have a List of 2 commands (negative, positive)
        and hats have a List of 4 commands (-x, +x, -y, +y)
    """
    CONFIG = {
        'Xbox 360 Wireless Receiver': {
            Input.BUTTON: {
                0:  CMD.kick,
                1:  CMD.strafe_right,
                2:  CMD.strafe_left,
                3:  CMD.stand_up,
                4:  CMD.strafe_left,
                5:  CMD.strafe_right
            },
            Input.AXIS: {
                0:  (CMD.turn_left, CMD.turn_right),
                1:  (CMD.walk_forward, CMD.walk_backward)
            },
            Input.HAT: {
                0:  (
                    (CMD.turn_left, CMD.turn_right),
                    (CMD.walk_backward, CMD.walk_forward)
                )
            }
        },
        "Microsoft X-Box 360 pad": {
            Input.BUTTON: {
                0:  CMD.kick,
                1:  CMD.head_sweep,
                2:  CMD.head_static,
                3:  CMD.stand_up,
                4:  CMD.strafe_left,
                5:  CMD.strafe_right
            },
            Input.AXIS: {
                0:  (CMD.turn_left, CMD.turn_right),
                1:  (CMD.walk_forward, CMD.walk_backward),
                2:  (CMD.stand, CMD.kick),
                5:  (CMD.stand, CMD.kick)
            },
            Input.HAT: {
                0:  (
                    (CMD.turn_left, CMD.turn_right),
                    (CMD.walk_backward, CMD.walk_forward)
                )
            }
        },
        "Xbox 360 Controller": {
            Input.BUTTON: {
                0:  CMD.kick,
                1:  CMD.head_sweep,
                2:  CMD.head_static,
                3:  CMD.stand_up,
                4:  CMD.strafe_left,
                5:  CMD.strafe_right
            },
            Input.AXIS: {
                0:  (CMD.turn_left, CMD.turn_right),
                1:  (CMD.walk_forward, CMD.walk_backward),
                2:  (CMD.stand, CMD.strafe_left),
                5:  (CMD.stand, CMD.strafe_right)
            },
            Input.HAT: {
                0:  (
                    (CMD.turn_left, CMD.turn_right),
                    (CMD.walk_backward, CMD.walk_forward)
                )
            }
        },
        "Xbox Series X Controller": {
            Input.BUTTON: {
                0:  CMD.kick,
                1:  CMD.head_sweep,
                2:  CMD.head_static,
                3:  CMD.stand_up,
                4:  CMD.turn_left,
                5:  CMD.turn_right
            },
            Input.AXIS: {
                0:  (CMD.strafe_left, CMD.strafe_right),
                1:  (CMD.walk_forward, CMD.walk_backward),
                2:  (CMD.stand, CMD.kick),
                5:  (CMD.stand, CMD.kick)
            },
            Input.HAT: {
                0:  (
                    (CMD.strafe_left, CMD.strafe_right),
                    (CMD.walk_backward, CMD.walk_forward)
                )
            }
        },
        'MOCUTE-032X': {
            Input.BUTTON: {
                0:  CMD.strafe_left,
                1:  CMD.kick,
                2:  CMD.stand_up,
                3:  CMD.strafe_right
            },
            Input.AXIS: {
                0:  (CMD.walk_forward, CMD.walk_backward),
                1:  (CMD.turn_right, CMD.turn_left)
            }
        },
        # default config if no other string matches
        '__default__': {
            Input.BUTTON: {
                0:  CMD.kick,
                1:  CMD.stand_up,
                6:  CMD.turn_left,
                7:  CMD.turn_right
            },
            Input.HAT: {
                0:  (
                    (CMD.strafe_left, CMD.strafe_right),
                    (CMD.walk_backward, CMD.walk_forward)
                )
            }
        },

    }

    def __init__(self):
        # initialize pygame and get all joysticks
        self.joysticks = {}
        pygame.init()
        joysticks = [pygame.joystick.Joystick(x) for x in range(pygame.joystick.get_count())]

        for j in joysticks:
            # initialize joystick
            j.init()
            # set config
            cfg = '__default__'
            jsname = j.get_name()
            for key in self.CONFIG:
                if jsname.startswith(key):
                    cfg = key
                    break
            self.joysticks[j] = self.CONFIG[cfg]
            print("Gamepad: using config \xbb{}\xab for gamepad {}: \xbb{}\xab".format(cfg, j.get_id(), jsname))


    """
    process pygame events and returns command
    """
    def pollInput(self):
        cmd = CMD.NONE
        event = pygame.event.poll()
        while event:
            for j in self.joysticks.keys():
                # we don't need to check this joystick, if the event was not triggered by it
                if not "joy" in dir(event) or event.joy != j.get_id():
                    continue
                
                cfg = self.joysticks[j]

                # handle button press
                if event.type == pygame.JOYBUTTONDOWN and Input.BUTTON in cfg:
                    btns = cfg[Input.BUTTON]
                    if event.button in btns:
                        cmd = btns[event.button]

                # handle hat motion
                elif event.type == pygame.JOYHATMOTION and Input.HAT in cfg:
                    if event.hat in cfg[Input.HAT]:
                        hat = cfg[Input.HAT][event.hat]
                        x,y = event.value
                        if x < -self.DEAD_ZONE:
                            cmd = hat[0][0]
                        elif x > self.DEAD_ZONE:
                            cmd = hat[0][1]
                        elif y < -self.DEAD_ZONE:
                            cmd = hat[1][0]
                        elif y > self.DEAD_ZONE:
                            cmd = hat[1][1]
                        else:
                            cmd = CMD.stand

                # handle axis motion
                elif event.type == pygame.JOYAXISMOTION and Input.AXIS in cfg:
                    if event.axis in cfg[Input.AXIS]:
                        axis = cfg[Input.AXIS][event.axis]
                        if event.value < -self.DEAD_ZONE:
                            cmd = axis[0]
                        elif event.value > self.DEAD_ZONE:
                            cmd = axis[1]
                        else:
                            cmd = CMD.stand

                else:
                    cmd = CMD.stand

            event = pygame.event.poll()

        return cmd


# debug mode: run endless loop printing commands
if __name__ == '__main__':
    ctrl = Gamepad()
    while (True):
        c = ctrl.pollInput()
        if c != CMD.NONE:
            print(c)
            import time
            time.sleep(0.1)

        


