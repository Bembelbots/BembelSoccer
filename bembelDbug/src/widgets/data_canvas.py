from PyQt5 import QtGui
from PyQt5.QtCore import Qt

from PyQt5.QtWidgets import (
    QWidget,
)

from PyQt5.QtGui import (
    QImage,
    QPainter,
    QPixmap,
    QColor,
)

from bembelapi.nao import NaoCamImage

import consts
import numpy as np


class DataCanvas(QWidget):
    """
    Displays a data (such as NP data) as an image.

    """

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self._img = None
        self._data = None
        self._visionResultCanvas = None
        self._visionResults = []

    def setRawRGB(self, data):
        if data is None:
            self._data = None
            return
        if self._img is None:
            self._img = QImage()
        data = data.astype(np.uint8)
        width, height = data.shape[1], data.shape[0]
        bytes_per_line = 3*width

        self._img = QImage(data, width, height, bytes_per_line, QImage.Format_RGB888)
        self.update()

    def set2D(self, data):
        assert len(data.shape) == 2
        data = np.reshape(data, (data.shape[0],data.shape[1],1))
        rgb = np.concatenate([data,data,data], axis=2)
        self.setRawRGB(rgb)


    def paintEvent(self, ev):

        painter = QPainter(self)

        if self._img is None:
            painter.fillRect(self.rect(), QColor('black'))
            return

        painter.drawImage(self.rect(), self._img)

        for visionResult in self._visionResults:
           self.drawVisionResult(painter, visionResult)



