# http://stackoverflow.com/a/30938728

from PyQt5.QtCore import QRect
from PyQt5.QtCore import Qt
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtWidgets import QHeaderView
from PyQt5.QtWidgets import QStyle
from PyQt5.QtWidgets import QStyleOptionButton


class CheckBoxHeader(QHeaderView):
    clicked = pyqtSignal(int, bool)

    _x_offset = 3
    _y_offset = 0  # This value is calculated later, based on the height of the paint rect
    _width = 20
    _height = 20

    def __init__(self, column_indices, orientation=Qt.Horizontal, parent=None):
        super(CheckBoxHeader, self).__init__(orientation, parent)
        self.setMinimumSectionSize(100)  # TODO: find a way to dynamically allocate column size to fit checkbox
        self.setSectionsClickable(True)

        if isinstance(column_indices, list) or isinstance(column_indices, tuple):
            self.column_indices = column_indices
        elif isinstance(column_indices, (int, int)):
            self.column_indices = [column_indices]
        else:
            raise RuntimeError('column_indices must be a list, tuple or integer')

        self.isChecked = {}
        for column in self.column_indices:
            self.isChecked[column] = 0

    def paintSection(self, painter, rect, logicalIndex):
        painter.save()
        super().paintSection(painter, rect, logicalIndex)
        painter.restore()

        self._y_offset = int((rect.height() - self._width) / 2.)

        if logicalIndex not in self.column_indices:
            return

        option = QStyleOptionButton()
        option.rect = QRect(rect.x() + self._x_offset, rect.y() + self._y_offset, self._width, self._height)
        option.state = QStyle.State_Enabled | QStyle.State_Active
        if self.isChecked[logicalIndex] == 2:
            option.state |= QStyle.State_NoChange
        elif self.isChecked[logicalIndex]:
            option.state |= QStyle.State_On
        else:
            option.state |= QStyle.State_Off

        self.style().drawControl(QStyle.CE_CheckBox, option, painter)

    def updateCheckState(self, index, state):
        self.isChecked[index] = state
        self.viewport().update()

    def mousePressEvent(self, event):
        index = self.logicalIndexAt(event.pos())

        if index not in self.column_indices:
            return

        if 0 <= index < self.count():
            x = self.sectionPosition(index)
            if x + self._x_offset < event.pos().x() < x + self._x_offset + self._width \
                    and self._y_offset < event.pos().y() < self._y_offset + self._height:
                if self.isChecked[index] == 1:
                    self.isChecked[index] = 0
                else:
                    self.isChecked[index] = 1

                self.clicked.emit(index, self.isChecked[index])
                self.viewport().update()
            else:
                super(CheckBoxHeader, self).mousePressEvent(event)
        else:
            super(CheckBoxHeader, self).mousePressEvent(event)
