from PyQt5 import QtGui
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QFont
from PyQt5.QtGui import QPainter
from utils.parseFrameworkClasses import RobotData
from widgets.playing_field.field_objects.field_object import FieldObject

class GCInfos(FieldObject):

    def __init__(self, fieldconfig):
        super().__init__("GameController")
        self.information = None
        self.fieldconfig = fieldconfig

    def handleGCMessage(self, message):
        self.information = message
        self.available = True

    def drawWCSText(self, drawer):
        # use only the offset of the drawing method, because we dont want the text mirrored

        # draw times
        drawer.drawTextAtPoint(self.information.timeRemaining, 0, 0, (0,-(self.fieldconfig.field_width/2-200)), 240)

        if self.information.secondsRemainingSecondary != 0:
            drawer.drawTextAtPoint(self.information.timeRemainingSecondary, 0, 0, (0,-(self.fieldconfig.field_width/2-500)), 160)

        # calculate score positioning
        x = -0.33*self.fieldconfig.field_length
        y = -(self.fieldconfig.field_width/2-300)
        mboffset = 500
        if drawer.is_mirrored():
            x = -x
            mboffset = -mboffset

        pos1 = (x, y)
        pos2 = (-x, y)
        mbpos1 = (x-mboffset, y-150)
        mbpos2 = (-(x-mboffset), y-150)

        # draw scores
        drawer.drawTextAtPoint(str(self.information.teams[0].score), 0, 0, pos1, 500, self.information.teams[0].teamColor)
        drawer.drawTextAtPoint(str(self.information.teams[1].score), 0, 0, pos2, 500, self.information.teams[1].teamColor)
        
        # message budgets
        drawer.drawTextAtPoint(str(self.information.teams[0].messageBudget), 0, 0, mbpos1, 100)
        drawer.drawTextAtPoint(str(self.information.teams[1].messageBudget), 0, 0, mbpos2, 100)

        # draw gamestates
        pos = (-0.2*self.fieldconfig.field_length, -(self.fieldconfig.field_width/2-200))
        drawer.drawTextAtPoint(str(self.information.gameState), 0, 0, pos, 200)

        # display first/second half
        drawer.drawTextAtPoint(str(self.information.half)+ " Half", 0, 0, (0, self.fieldconfig.field_width/2-200), 200)

        if self.information.currentSetPlay != None:
            pos = (0.2*self.fieldconfig.field_length, -(self.fieldconfig.field_width/2-200))
            drawer.drawTextAtPoint(str(self.information.currentSetPlay), 0, 0, pos, 160)
        
        if self.information.gamePhase != None:
            pos = (0.2*self.fieldconfig.field_length, -(self.fieldconfig.field_width/2-200))
            drawer.drawTextAtPoint(str(self.information.gamePhase), 0, 0, pos, 160)

        
        if self.information.gameState == "Finished" and self.information.half == "First":
            pos = (0.2*self.fieldconfig.field_length, -(self.fieldconfig.field_width/2-200))
            drawer.drawTextAtPoint("Half-Time", 0, 0, pos, 160)
