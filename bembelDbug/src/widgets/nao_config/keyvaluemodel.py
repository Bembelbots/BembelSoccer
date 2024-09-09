from PyQt5.QtCore import QAbstractTableModel
from PyQt5.QtCore import QVariant
from PyQt5.QtCore import Qt


class KeyValueModel(QAbstractTableModel):
    def __init__(self, parent=None):
        QAbstractTableModel.__init__(self, parent)
        self.h_header = []
        self.v_header = []
        self.values = []

    @property
    def dict(self):
        return dict(zip(self.v_header, self.values))

    @dict.setter
    def dict(self, data: dict):
        if data is None:
            data = {}

        self.layoutAboutToBeChanged.emit()

        items = list(data.items())
        items.sort(key=lambda x: x[0])

        self.h_header = []
        self.v_header = []
        self.values = []

        self.h_header = ["value"]
        for key, value in items:
            self.v_header.append(key)
            self.values.append(value)

        self.layoutChanged.emit()

    def rowCount(self, parent=None, **kwargs):
        return len(self.v_header)

    def columnCount(self, parent=None, **kwargs):
        return len(self.h_header)

    def data(self, index, role=None):
        if not index.isValid():
            return QVariant()
        elif role != Qt.DisplayRole:
            return QVariant()
        return QVariant(self.values[index.row()])

    def headerData(self, col, orientation, role=None):
        if role != Qt.DisplayRole:
            return QVariant()

        if len(self.h_header) != 0 and orientation == Qt.Horizontal:
            return QVariant(self.h_header[col])
        elif len(self.v_header) != 0 and orientation == Qt.Vertical:
            return QVariant(self.v_header[col])

        return QVariant()

    def setData(self, index, value, role=None):
        data = self.values[index.row()]
        tmp = value.strip()
        if tmp == "" and data[0] != "":
            return False
        self.values[index.row()] = tmp
        return True

    def flags(self, index):
        if index.column() == 0:
            return Qt.ItemIsEditable | Qt.ItemIsEnabled | Qt.ItemIsSelectable
        return Qt.ItemIsEnabled

    def clear(self):
        self.layoutAboutToBeChanged.emit()
        self.h_header = []
        self.v_header = []
        self.values = []
        self.layoutChanged.emit()
