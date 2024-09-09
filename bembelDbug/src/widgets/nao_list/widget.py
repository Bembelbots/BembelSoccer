from typing import Optional

from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, QTimer
from PyQt5.QtWidgets import QLayout
from PyQt5.QtWidgets import QWidget

from bembelapi.config import NaoEvent
from bembelapi.nao import NaoInfo
from bembelapi.naoconnection import NaoConnection

from config import NaoStatusColors
from debugger import BembelDbugFactory
from ui.naolist.naolist_ui import Ui_NaoListWidget
from widgets.nao.widget import NaoWidget
from widgets.util import PyQtWait, qt_apply_styles


class NaoListWidget(Ui_NaoListWidget, QWidget):
    nao_focused = pyqtSignal(NaoWidget)
    nao_found = pyqtSignal(NaoWidget)
    nao_updated = pyqtSignal(NaoWidget)
    nao_lost = pyqtSignal(NaoWidget)
    nao_connected = pyqtSignal(NaoWidget)
    nao_disconnected = pyqtSignal(NaoWidget)
    sync_frontend_signal = pyqtSignal(NaoInfo)
    sync_all_signal = pyqtSignal(NaoInfo)

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.contents = self.scrollAreaWidgetContents.layout()  # type: QLayout
        self.naos = {}
        self.register_events()
        self._t = QTimer()
        self._t.timeout.connect(self.update_nao_widgets)
        self._t.start(500)
        self._current_widget = None   # type: Optional[NaoWidget]

    def retranslateUi(self, window) -> None:
        super().retranslateUi(window)

    def register_events(self) -> None:
        events = BembelDbugFactory.get()
        events.add_listener(NaoEvent.NAO_FOUND, self.handle_nao_found)
        events.add_listener(NaoEvent.NAO_LOST, self.handle_nao_lost)
        events.add_listener(NaoEvent.NAO_UPDATE, self.handle_nao_update)
        events.add_listener(NaoEvent.NAO_CONNECTED, self.handle_nao_connected)
        events.add_listener(NaoEvent.NAO_DISCONNECTED, self.handle_nao_disconnected)

    def handle_nao_found(self, nao: NaoInfo) -> None:
        nao_widget = NaoWidget(self, nao)
        nao_widget.installEventFilter(self)
        nao_widget.sync_all_signal.connect(self.sync_all_signal.emit)
        nao_widget.sync_frontend_signal.connect(self.sync_frontend_signal.emit)
        nao_widget.selected_nao.connect(self.handle_nao_selected)

        self.naos[nao.name] = nao_widget
        self.contents.addWidget(nao_widget)
        self.update_nao_widget(nao_widget)
        if(self._current_widget is None):
            self.focusNao(nao_widget)
        self.nao_found.emit(self.naos[nao.name])

    def handle_nao_selected(self, nao: NaoInfo, selected: bool):
        pass

    def handle_nao_lost(self, nao: NaoInfo) -> None:
        nao_widget = self.naos.get(nao.name)
        if nao_widget is None:
            return
        nao_widget.hide()
        self.contents.removeWidget(nao_widget)
        nao_widget.connection = None
        self.nao_lost.emit(nao_widget)
        del self.naos[nao.name]

    def handle_nao_update(self, nao: NaoInfo) -> None:
        nao_widget = self.naos.get(nao.name)
        if nao_widget is None:
            return
        self.update_nao_widget(nao_widget)
        self.nao_updated.emit(nao_widget)

    def handle_nao_connected(self, connection: NaoConnection):
        nao_widget = self.naos.get(connection.name)
        if nao_widget is None:
            return
        nao_widget.connection = connection
        self.update_nao_widget(nao_widget)
        self.nao_connected.emit(nao_widget)

    def handle_nao_disconnected(self, connection: NaoConnection):
        nao_widget = self.naos.get(connection.name)
        if nao_widget is None:
            return
        nao_widget.connection = None
        self.update_nao_widget(nao_widget)
        self.nao_disconnected.emit(nao_widget)

    def focusNao(self, naoWidget: NaoWidget):
        if naoWidget is self._current_widget:
            return

        if self._current_widget is not None:
            old_widget = self._current_widget
            old_widget.removeFocus()
            self.update_nao_widget(old_widget)

        if naoWidget is not None:
            self._current_widget = naoWidget
            self._current_widget.setFocus()
            self.update_nao_widget(self._current_widget)
            self.nao_focused.emit(self._current_widget)


    def eventFilter(self, nao_widget: NaoWidget, event) -> bool:
        assert isinstance(nao_widget, NaoWidget)

        # not focus event or mouse button press
        # do not filter out right mouse button presses here
        # right-click-to-sync currently relies on the selected nao being set by right clicking
        if event.type() != QtCore.QEvent.FocusIn and event.type() != QtCore.QEvent.MouseButtonPress:
            return False

        self.focusNao(nao_widget)

        return False

    def update_nao_widget(self, nao_widget: NaoWidget):
        nao_widget.update()

    def update_nao_widgets(self):
        for nao in self.naos.values():
            nao.update()

    def resizeEvent(self, event):
        super().resizeEvent(event)
