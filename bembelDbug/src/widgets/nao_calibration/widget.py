from typing import Optional

from PyQt5.QtWidgets import QVBoxLayout
from PyQt5.QtWidgets import QWidget
from bembelapi.config import NaoEvent
from bembelapi.nao import NaoInfo

from debugger import BembelDbugFactory
from widgets.base.categorized_table.widget import CategorizedTableWidget


class NaoCalibrationWidget(QWidget):

    def __init__(self, nao: NaoInfo = None):
        super().__init__()
        self.nao = nao
        self.table = CategorizedTableWidget(self)
        self.setupui(self)
        events = BembelDbugFactory.get()
        events.add_listener(NaoEvent.NAO_UPDATE, self.handle_nao_update)
        self.update()

    def setupui(self, NaoCalibrationWidget):
        verticalLayout = QVBoxLayout()
        verticalLayout.setContentsMargins(0, 0, 0, 0)
        verticalLayout.setSpacing(0)
        self.setLayout(verticalLayout)
        self.layout().addWidget(self.table)
        self.table.data_applied.connect(self.handle_apply)

    def set_nao(self, nao: Optional[NaoInfo]):
        self.nao = nao
        self.table.clear()

        if nao is not None:
            self.table.data = nao.config.calibration

    def handle_nao_update(self, nao: NaoInfo):
        if nao != self.nao:
            return

        if self.table.model_list.rowCount() == 0:
            self.table.data = nao.config.calibration

    def handle_apply(self, data, _):
        if self.nao is None:
            return

        self.nao.config.setCalibration(data)
        self.nao.send_calibration()