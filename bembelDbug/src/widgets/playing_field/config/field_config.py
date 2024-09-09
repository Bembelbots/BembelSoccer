from utils.constants import Constants
from utils.coords import DirectedCoord
import math
from PyQt5 import QtSvg

class FieldConfig:

    def __init__(self):
        self.svg_name = None
        self.penalties = {}
    
    # the field config keeps track of the penalty spots,
    # hands them out and frees them again
    def getNextPenaltySpot(self, team):

        if team not in self.penalties:
            self.penalties[team] = [False]*Constants.NUM_PLAYERS

        # find next free penaltyspot
        spot = -1
        for i in range(Constants.NUM_PLAYERS):
            if self.penalties[team][i] == False:
                self.penalties[team][i] = True
                spot = i
                break

        if spot == -1:
            print("Too many penaltyspots needed?!")

        return spot, DirectedCoord(-self.field_length/2 + 0.7*spot, -self.field_width/2 - 0.35, -math.pi/2)

    def freePenaltySpot(self, team, spot):        

        if team not in self.penalties:
            self.penalties[team] = [False]*Constants.NUM_PLAYERS
            return

        self.penalties[team][spot] = False

    def getSize(self):
        svg = QtSvg.QSvgRenderer()
        svg.load(self.svg_name)
        s = svg.viewBox().size()
        return (s.width(), s.height())
        

    @staticmethod
    def fromID(number):
        
        mapping = {
            0: JRLFieldConfig(),
            1: SPLFieldConfig(),
            2: HTWKFieldConfig(),
            3: TinyFieldConfig(),
            255: JsonFieldConfig(),
        }

        if number not in mapping:
            print("No such field ID!")
            return None

        return mapping[number]


class JRLFieldConfig(FieldConfig):

    def __init__(self):
        super().__init__()
        self.svg_name = "resources/images/jrl-field.svg"
        self.field_length, self.field_width = self.getSize()


class SPLFieldConfig(FieldConfig):

    def __init__(self):
        super().__init__()
        self.svg_name = "resources/images/spl-field.svg"
        self.field_length, self.field_width = self.getSize()


class HTWKFieldConfig(FieldConfig):

    def __init__(self):
        super().__init__()
        self.svg_name = "resources/images/htwk-field.svg"
        self.field_length, self.field_width = self.getSize()


class TinyFieldConfig(FieldConfig):

    def __init__(self):
        super().__init__()
        self.svg_name = "resources/images/tiny-field.svg"
        self.field_length, self.field_width = self.getSize()


class JsonFieldConfig(FieldConfig):

    def __init__(self):
        super().__init__()
        #self.svg_name = "resources/images/json-field.svg"
        self.svg_name = "resources/images/spl-field.svg" # assume json dimensions are identical to SPL field
        self.field_length, self.field_width = self.getSize()
