from tools.blackboard.widget import BlackboardWidget
from tools.nao_camera.widget import NaoCameraWidget
from tools.playing_field.widget import PlayingFieldWidget
from tools.remote.widget import RemoteWidget


def register_tools():
    NaoCameraWidget.register()
    BlackboardWidget.register()
    PlayingFieldWidget.register()
    RemoteWidget.register()
