import time

from PyQt5.QtCore import QFile
from PyQt5.QtCore import QTextStream
from PyQt5.QtCore import QThread
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtGui import QStandardItem
from PyQt5.QtGui import QStandardItemModel
from PyQt5.QtWidgets import QWidget


class PyQtWait(QThread):
    finished = pyqtSignal()

    def __init__(self, seconds, callback):
        super().__init__()
        self.seconds = seconds
        self.finished.connect(callback)

    def run(self):
        time.sleep(self.seconds)
        self.finished.emit()


def dict_to_item_model(data: dict, index: dict=None,
                       item_modifier=None, model: QStandardItemModel=None) -> QStandardItemModel:
    if data is None:
        data = {}

    if model is None:
        model = QStandardItemModel()
    else:
        model.clear()

    model.setHorizontalHeaderLabels(["Key", "Value"])

    items = list(data.items())
    # items.sort(key=lambda x: x[0])

    for key, value in items:
        key_item = QStandardItem(key)
        value_item = QStandardItem(value)

        key_item.setEditable(False)
        value_item.setEditable(True)

        # create an index to use for updates
        if index is not None:
            index[key_item] = value_item

        if item_modifier is not None:
            item_modifier(key_item, value_item)

        model.appendRow([key_item, value_item])

    return model


# https://wiki.qt.io/Dynamic_Properties_and_Stylesheets#Limitations
# http://dgovil.com/blog/2017/02/24/qt_stylesheets/
# https://www.mail-archive.com/pyqt@riverbankcomputing.com/msg12322.html
# you had one job qt
def qt_apply_styles(widget: QWidget):
    widget.setStyle(widget.style())
