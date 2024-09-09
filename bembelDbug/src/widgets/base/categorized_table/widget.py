import queue
from typing import Optional

from PyQt5.QtCore import QItemSelection
from PyQt5.QtCore import QModelIndex
from PyQt5.QtCore import Qt
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtGui import QStandardItem
from PyQt5.QtGui import QStandardItemModel
from PyQt5.QtWidgets import QTableView
from PyQt5.QtWidgets import QWidget

from ui.categorizedtable_ui import Ui_CategorizedTableWidget

# TODO: move knowledge of table structure out of this widget to allow for tables of various sizes


class CategorizedTableWidget(QWidget):
    display_update = pyqtSignal(QTableView, QStandardItemModel)
    data_applied = pyqtSignal(dict, dict)

    def __init__(self, parent=None, item_modifier=None):
        super().__init__(parent)
        self._item_modifier = item_modifier
        self._selected_item = None  # type: Optional[str]
        self._data = {}
        self._changed_items = {}
        self._index = {}
        self.model_list = QStandardItemModel()
        self.table_models = {}
        self.table_models_reverse_map = {}

        self.ui = Ui_CategorizedTableWidget()
        self.ui.setupUi(self)
        self.ui.btn_apply.clicked.connect(self.handle_apply)
        self.ui.btn_revert.clicked.connect(self.handle_revert)

        self.ui.listView.setModel(self.model_list)
        self.ui.listView.selectionModel().selectionChanged.connect(self.handle_selection_changed)

        self.ui.tableView.setSortingEnabled(True)

        self.clear()

    @property
    def table_view(self) -> QTableView:
        return self.ui.tableView

    @property
    def selected_item(self) -> str:
        return self._selected_item

    @property
    def current_model(self) -> QStandardItemModel:
        return self.table_models.get(self._selected_item)

    @property
    def data(self) -> dict:
        return self._data

    @data.setter
    def data(self, data: dict):
        self.clear()
        self.update_models(data)

    def update_data(self, data: dict):
        self.update_models(data)

    def handle_selection_changed(self, selection: QItemSelection):
        indexes = selection.indexes()

        if len(indexes) == 0:
            self._selected_item = None
            self.redraw()
            return

        item = self.model_list.itemFromIndex(indexes[0])  # type: QStandardItem
        self._selected_item = item.text()
        self.redraw()

    def handle_item_changed(self, item: QStandardItem):
        model = item.model()
        index = item.index()  # type: QModelIndex

        category = self.table_models_reverse_map[model]
        key = model.item(index.row(), 0).text()
        value = item.text()

        if self.data[category][key] == value or key == value:
            return

        changed_items = self._changed_items.get(category, {})
        changed_items[key] = item.text()
        self._changed_items[self._selected_item] = changed_items

    def handle_revert(self):
        self._changed_items = {}
        self.update_models(self._data)
        self.ui.btn_revert.clearFocus()

    def handle_apply(self):
        self.ui.btn_apply.clearFocus()

        for category, changed_items in self._changed_items.items():
            for key, value in changed_items.items():
                self._data[category][key] = value

        self.data_applied.emit(self._data, self._changed_items)
        self._changed_items = {}

    def clear(self):
        self.model_list.clear()
        self._selected_item = None
        table_model = self.table_models.get(self._selected_item)
        self.ui.tableView.setModel(table_model)
        self.table_models = {}
        self._changed_items = {}
        self._index = {}

    def redraw(self):
        table_model = self.table_models.get(self._selected_item)
        self.ui.tableView.setModel(table_model)
        self.ui.tableView.sortByColumn(0, Qt.AscendingOrder)
        self.ui.tableView.resizeColumnsToContents()
        self.ui.tableView.resizeRowsToContents()
        self.display_update.emit(self.ui.tableView, table_model)

    def update_models(self, data: dict):
        """
        Creates an index from nested dictionaries using a depth first traversal
        inserting new data into the widgets.nao_config.models and updating old data
        :param data: dict
        :return: None
        """

        if data is None:
            data = {}

        flagged = set()
        _stack = queue.LifoQueue()

        keys = list(data.keys())
        keys.sort(reverse=True)

        for key in keys:
            _stack.put((data, self._index, key, None))

        while not _stack.empty():
            data_offset, index_offset, current, parent = _stack.get_nowait()

            if (current, parent) in flagged:
                continue

            flagged.add((current, parent))
            current_value = data_offset.get(current)
            index = index_offset.get(current, {"_key": None, "_value": None})

            has_children = isinstance(current_value, dict)
            new_entry = index["_key"] is None or index["_value"] is None

            if new_entry:
                index["_key"] = QStandardItem(current)

            if new_entry and has_children:
                index["_value"] = QStandardItem(current)

                self.model_list.appendRow(index["_value"])

                model = QStandardItemModel()
                model.setHorizontalHeaderLabels(["Key", "Value"])
                model.itemChanged.connect(self.handle_item_changed)

                self.table_models[current] = model
                self.table_models_reverse_map[model] = current

                self._data[current] = {}

            if new_entry and not has_children:
                index["_value"] = QStandardItem(current_value)

                if self._item_modifier is not None:
                    self._item_modifier(index["_key"], index["_value"], parent)

                self.table_models[parent].appendRow([index["_key"], index["_value"]])

                self._data[parent][current] = current_value

            if not new_entry and not has_children:
                    self._data[parent][current] = current_value
                    changed_parent = self._changed_items.get(parent, {})
                    changed_item = changed_parent.get(current)
                    if not changed_item:
                        index["_value"].setText(current_value)

            index_offset[current] = index

            if not has_children:
                continue

            for child in current_value.keys():
                _stack.put((data_offset[current], index_offset[current], child, current))

        if len(keys) > 0:
            list_view = self.ui.listView
            list_view.updateGeometry()
            list_view.setMinimumWidth(100)
            list_view.setMaximumWidth(list_view.sizeHintForColumn(0) + list_view.verticalScrollBar().width())
