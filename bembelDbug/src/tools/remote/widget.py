
if __name__ == '__main__':
    import bootstrap

    bootstrap.bootstrap()

from PyQt5 import QtGui, QtWidgets, QtCore
from ui.remote_ui import Ui_NaoRemoteWidget
from ui.remote_keydialog_ui import Ui_Dialog
from bembelapi.naoconnection import NaoConnection
from bembelapi.naomodule import NaoPlugin

from debugger import BembelDbugFactory

from tool_management.toolsregistry import ConnectionTool

from tools.remote.gamepad import CMD, Gamepad


class RemoteWidget(ConnectionTool):
    name = "remote"
    button_text = "Remote"
    _blackboard = "BehaviorBlackboard"

    def __init__(self):
        ConnectionTool.__init__(self, self.name, self.button_text)
        self.debugger = BembelDbugFactory.get()
        self.ui = Ui_NaoRemoteWidget()        
        self.commands = self.debugger.config.data["controller_commands"];
        self.ui.setupUi(self)
        self.setupUi(self)
        self.activecommands = []

        self.last_gamepad_command = CMD.NONE
        self.gamepad = Gamepad()

        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.gamepadEvent)
        self.timer.start(50)
        
    def setup(self):
        super().setup()
        self.connection.change_value(self._blackboard, "test_mode", 1)
        self.connection.change_value(self._blackboard, "test_start", 1)
        self.connection.change_value(self._blackboard, "test_active", "test_remote_walk")

    def shutdown(self):
        super().shutdown()
        self.connection.change_value(self._blackboard, "test_mode", 0)
        self.connection.change_value(self._blackboard, "test_start", 0)
        if self.timer != None:
            self.timer.stop()

    # noinspection PyAttributeOutsideInit
    def setupUi(self, widget):       
        self.ui.forward_button.released.connect(lambda: self.send_command("walk_forward"))
        self.ui.backward_button.released.connect(lambda: self.send_command("walk_backward"))
        self.ui.left_button.released.connect(lambda: self.send_command("turn_left"))
        self.ui.right_button.released.connect(lambda: self.send_command("turn_right"))
        self.ui.strafe_left_button.released.connect(lambda: self.send_command("strafe_left"))
        self.ui.strafe_right_button.released.connect(lambda: self.send_command("strafe_right"))
        self.ui.stop_button.released.connect(lambda: self.send_command("stand"))
        self.ui.kick_button.released.connect(lambda: self.send_command("kick"))
        self.ui.standup_button.released.connect(lambda: self.send_command("stand_up"))
        self.ui.head_sweep_button.released.connect(lambda: self.send_command("head_sweep"))
        self.ui.head_static_button.released.connect(lambda: self.send_command("head_static"))
        self.ui.say_button.released.connect(lambda: self.say())

        # setup table for keybindings
        self.ui.keybindingsTable.setHorizontalHeaderLabels(["Command","Keybinding"])
        self.updateKeybindingTable()
        self.ui.keybindingsTable.doubleClicked.connect(self.onKeybindingEdit)

        # setup table for keybindings
        self.ui.gamepadTable.setHorizontalHeaderLabels(["Command","Gamepadbinding"])

    	
    def say(self):
        text = str(self.ui.say_text.text());
        text = text.replace(" ", "_");
        self.connection.change_value(self._blackboard, "nao_say", text);
        print("Remote: say " + text)
       

    def send_command(self,  name):
        if name not in self.commands.keys():
            return
        command = self.commands[name]["value"]
        print("Command: ", command)
        self.connection.change_value(self._blackboard, "remote_comm", command)
        print("Remote: " + name)

    def keyPressEvent(self, event):
        # dont process key events if the lineedit for saying text is in focus or if it is an autorepeated event
        if((self.ui.say_text == self.focusWidget()) or (event.isAutoRepeat())):
            return
        key = QtGui.QKeySequence(event.modifiers()|event.key()).toString()
        for name,cmd in self.commands.items():

            # make sure the shortcut is formatted the same to compare them
            shortcut = QtGui.QKeySequence(cmd["keybinding"]).toString()
            if shortcut == key:
                self.activecommands.append(name)
                self.send_command(name)
                event.accept()
                break
        event.ignore()

    def keyReleaseEvent(self, event):                
        # dont process key events if the lineedit for saying text is in focus or if it is an autorepeated event
        if((self.ui.say_text == self.focusWidget()) or (event.isAutoRepeat())):
            return
        key = QtGui.QKeySequence(event.modifiers()|event.key()).toString()
        for name,cmd in self.commands.items():

            # make sure the shortcut is formatted the same to compare them
            shortcut = QtGui.QKeySequence(cmd["keybinding"]).toString()
            if shortcut == key:
                self.activecommands.remove(name)
                if len(self.activecommands) == 0:
                    self.send_command("stand")
                else:
                    self.send_command(self.activecommands[-1])
                event.accept()
                break
        event.ignore()

    def onKeybindingEdit(self):
        if len(self.ui.keybindingsTable.selectedItems()) == 0:
            return
        cmdname = self.ui.keybindingsTable.selectedItems()[0].text()
        self.releaseKeyboard()
        dialog = KeybindingDialog(self, cmdname, self.commands[cmdname]["keybinding"], self.saveNewKeyBinding)

    def onGamepadBindingEdit(self):
        if len(self.ui.gamepadTable.selectedItems()) == 0:
            return
        cmdname = self.ui.gamepadTable.selectedItems()[0].text()
        dialog = KeybindingDialog(self, cmdname, self.commands[cmdname]["gamepad"], self.saveNewGamepadBinding)

    def saveNewKeyBinding(self, name, binding):
        self.commands[name]["keybinding"] = binding
        self.updateKeybindingTable()
        self.debugger.config.save()

    def saveNewGamepadBinding(self, name, binding):
        self.commands[name]["gamepad"] = binding
        self.updateGamepadTable()
        self.debugger.config.save()

    def updateKeybindingTable(self):
        # clear the table
        self.ui.keybindingsTable.setRowCount(0)

        # write keybindings again
        count = 0
        for name,cmd in self.commands.items():
            self.ui.keybindingsTable.insertRow(count)
            self.ui.keybindingsTable.setItem(count, 0, QtWidgets.QTableWidgetItem(name))
            self.ui.keybindingsTable.setItem(count, 1, QtWidgets.QTableWidgetItem(cmd["keybinding"]))
            count = count + 1

    def updateGamepadTable(self):
        # clear the table
        self.ui.gamepadTable.setRowCount(0)

        # write gamepadbindings in table again
        count = 0
        for name,cmd in self.commands.items():
            self.ui.gamepadTable.insertRow(count)
            self.ui.gamepadTable.setItem(count, 0, QtWidgets.QTableWidgetItem(name))
            self.ui.gamepadTable.setItem(count, 1, QtWidgets.QTableWidgetItem(cmd["gamepad"]))
            count = count + 1
    
    def gamepadEvent(self):
        """ runs frequently for testing pygame events coming up """
        cmd = self.gamepad.pollInput()
        if cmd != cmd.NONE and cmd != self.last_gamepad_command:
            self.last_gamepad_command = cmd
            self.send_command(cmd.name)



class KeybindingDialog(QtWidgets.QDialog):
    def __init__(self, parent, name, initial, callback):
        super().__init__(parent)
        self.callback = callback
        self.name = name
        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.ui.keybindingEdit.setFocus()
        # FIXME: Keybindingedit-Widget should be of type QKeySequenceEdit but this causes a bug in pyuic.
        self.ui.keybindingEdit.setKeySequence(QtGui.QKeySequence(initial))  
        self.exec_()

    def accept(self):
        self.callback(self.name, self.ui.keybindingEdit.keySequence().toString())
        super().accept()

    def reject(self):
        super().reject()

if __name__ == '__main__':
    import sys
    from PyQt5.QtWidgets import QApplication
    from widgets.nao.widget import NaoWidget
    from bembelapi.nao import NaoInfo
    from config import Config
    from util import relative_path

    debugger = BembelDbugFactory.get()
    debugger.config = Config(relative_path("../config/default.json"))

    app = QApplication(sys.argv)
    remote = RemoteWidget()
    remote.nao_widget = NaoWidget(NaoInfo("Unknown", None, None))
    remote.show()
    sys.exit(app.exec_())
