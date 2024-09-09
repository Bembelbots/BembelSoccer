from typing import Optional

from PyQt5.QtWidgets import QWidget
from bembelapi.config import NaoEvent
from bembelapi.nao import NaoInfo

from debugger import BembelDbugFactory
from ui.naoconfiguration_ui import Ui_NaoConfigWidget
from widgets.nao_config.keyvaluemodel import KeyValueModel


class NaoConfigWidget(Ui_NaoConfigWidget, QWidget):

    def __init__(self, nao: NaoInfo = None):
        super().__init__()
        self.nao = nao  # type: Optional[NaoInfo]
        events = BembelDbugFactory.get()
        events.add_listener(NaoEvent.NAO_UPDATE, self.handle_nao_update)
        self.setupUi(self)
        self._reset_models()
        self.update()

    def setupUi(self, MainWindow):
        super().setupUi(MainWindow)
        self.btn_apply.clicked.connect(self.handle_apply)
        self.btn_revert.clicked.connect(self.handle_revert)

    def set_nao(self, nao: Optional[NaoInfo]):
        self.nao = nao
        self._reset_models()
        self.update()

    def _reset_models(self):
        # TODO: update to use QStandardItemModel
        self.model = KeyValueModel()
        self.config_table.setModel(self.model)

    def handle_nao_update(self, nao: NaoInfo):
        if nao != self.nao:
            return

        if len(self.model.values) == 0:
            self.update()

    def handle_revert(self):
        self.update()
        self.btn_revert.clearFocus()

    def handle_apply(self):
        self.btn_apply.clearFocus()
        if self.nao is None:
            return
        self.nao.config.setConfiguration(self.model.dict)
        self.nao.send_config()

    def update(self):
        if self.nao is None:
            return

        config = self.nao.config

        if config.configuration is None:
            return
        self.model.dict = config.configuration
        self.config_table.resizeColumnsToContents()
        self.config_table.resizeRowsToContents()
