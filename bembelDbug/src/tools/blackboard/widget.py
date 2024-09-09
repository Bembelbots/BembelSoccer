from PyQt5.QtCore import QEvent
from PyQt5.QtCore import QThread
from PyQt5.QtCore import Qt
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtGui import QStandardItem
from PyQt5.QtGui import QStandardItemModel
from PyQt5.QtWidgets import QTableView
from PyQt5.QtWidgets import QVBoxLayout
from PyQt5.QtWidgets import QWidget
from bembelapi.config import NaoEvent

from debugger import BembelDbugFactory
from tool_management.toolsregistry import ConnectionTool
from widgets.base.categorized_table.widget import CategorizedTableWidget


class CheckboxController(QThread):
    header_check = pyqtSignal(QStandardItemModel, int, int)
    item_check = pyqtSignal(QStandardItem)

    def __init__(self, col_indexes, header):
        super().__init__()
        self.header = header
        self.col_indexes = col_indexes
        self.header_check.connect(self.handle_header_check)
        self.item_check.connect(self.handle_item_check)

    def handle_header_check(self, model: QStandardItemModel, index: int, state: int):
        for i in range(model.rowCount()):
            item = model.item(i, index)
            item.setCheckState(Qt.Checked if state else Qt.Unchecked)

    def handle_item_check(self, item: QStandardItemModel):
        model = item.model()
        for i in self.col_indexes:
            checked = 0
            unchecked = 0
            for j in range(model.rowCount()):
                if model.item(j, i).checkState() == Qt.Checked:
                    checked += 1
                elif model.item(j, i).checkState() == Qt.Unchecked:
                    unchecked += 1

            if checked and unchecked:
                self.header.updateCheckState(i, 2)
            elif checked:
                self.header.updateCheckState(i, 1)
            else:
                self.header.updateCheckState(i, 0)


class BlackboardWidget(ConnectionTool):
    name = "blackboard"
    button_text = "Edit Blackboard"

    def __init__(self):
        ConnectionTool.__init__(self, self.name, self.button_text)
        self.table = CategorizedTableWidget(self, self.handle_item_insert)
        self.setupui(self)
        self.models = {}
        self.symbol_names = {}
        # self.last_updated = time.time()

        # self.check_col_indexes = [0]
        # self.header = CheckBoxHeader(self.check_col_indexes, parent=self.table)
        # self.header.clicked.connect(self.handle_header_check)
        # self.checkbox_controller = CheckboxController(self.check_col_indexes, self.header)
        # self.checkbox_controller.start()

        events = BembelDbugFactory.get()
        events.add_listener(NaoEvent.NAO_SYMBOL_VALUES, self.handle_nao_symbol_values)
        self.update()

    def setupui(self, BlackboardWidget):
        self.resize(800, 600)
        verticalLayout = QVBoxLayout()
        verticalLayout.setContentsMargins(0, 0, 0, 0)
        verticalLayout.setSpacing(0)
        self.setLayout(verticalLayout)
        self.layout().addWidget(self.table)
        self.table.data_applied.connect(self.handle_apply)
        self.table.display_update.connect(self.handle_display_update)
        self.table.table_view.installEventFilter(self)

    def setup(self):
        super().setup()
        self.symbol_names = self.connection.symbol_names
        self.table.data = self.connection.symbol_names

        self.table.ui.btn_save.released.connect(self.connection.save_blackboards)
        events = BembelDbugFactory.get()
        events.add_listener(NaoEvent.NAO_SAVE_BLACKBOARDS_OK, lambda: print("Save blackboards ok!"))
        events.add_listener(NaoEvent.NAO_SAVE_BLACKBOARDS_ERROR, lambda: print("Save blackboards error!"))

    def handle_nao_symbol_values(self, data: dict, _):
        # now = time.time()
        # if now - self.last_updated < 0.01:
        #    return
        # print(now - self.last_updated)
        # self.last_updated = now
        self.table.update_data(data)

    def handle_apply(self, data, partial):
        for blackboard, values in partial.items():
            for key, value in values.items():
                self.connection.change_value(blackboard, key, value)

    def handle_item_insert(self, key: QStandardItem, value: QStandardItem, category: str):
        key.setEditable(False)
        key.setCheckable(True)

        symbols = self.symbol_names.get(category, {})
        if symbols.get(key.text(), 0) == '1':
            key.setCheckState(Qt.Checked)

    def handle_display_update(self, table_view: QTableView, model: QStandardItemModel):
        # table_view.setHorizontalHeader(self.header)
        if model is not None and self.models.get(model) is None:
            self.models[model] = 0
            model.itemChanged.connect(self.handle_item_changed)

    # def handle_header_check(self, index, state):
    #    self.checkbox_controller.header_check.emit(self.table.current_model, index, state)

    def handle_item_changed(self, item: QStandardItem):
        if item.column() != 0:
            return

        key = item.text()

        if item.checkState() == Qt.Checked:
            self.connection.start_receiving_symbol(self.table.selected_item, key)
        elif item.checkState() == Qt.Unchecked:
            self.connection.stop_receiving_symbol(self.table.selected_item, key)

        # self.checkbox_controller.item_check.emit(item)

    def eventFilter(self, widget: QWidget, event) -> bool:
        if event.type() == QEvent.KeyPress:
            if event.key() == Qt.Key_Return or event.key() == Qt.Key_Enter:
                table = widget  # type: QTableView
                model = table.model()  # type: QStandardItemModel
                item = model.itemFromIndex(table.currentIndex()) # type: QStandardItem
                if item.checkState() == Qt.Checked:
                    item.setCheckState(Qt.Unchecked)
                elif item.checkState() == Qt.Unchecked:
                    item.setCheckState(Qt.Checked)
                self.handle_item_changed(item)
            else:
                return widget.eventFilter(widget, event)
            return True
        else:
            return widget.eventFilter(widget, event)
