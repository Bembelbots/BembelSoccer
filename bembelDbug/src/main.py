import time

from bootstrap import bootstrap
from config import Config
from util import load_stylesheet, relative_path

bootstrap()

import argparse
import signal
import sys
from widgets.mainwindow import MainWindow, MainWindowWrapper
from widgets.patch_processor import PatchProcessor
from PyQt5.QtCore import QCoreApplication, QSettings
from PyQt5.QtWidgets import QApplication
from debugger import BembelDbugFactory
from tool_management.toolsregistry import ToolsRegistry, ToolTypes
from tool_management.toollauncher import ToolLauncher
from plugins import register_plugins
from tools import register_tools

import sys
import traceback

from collections import namedtuple

from config import APPLICATION_NAME, ORGANIZATION_NAME, ORGANIZATION_DOMAIN

"""
Added exception hook with thanks to: https://fman.io/blog/pyqt-excepthook/
"""

def excepthook(exc_type, exc_value, exc_tb):
    enriched_tb = _add_missing_frames(exc_tb) if exc_tb else exc_tb
    # Note: sys.__excepthook__(...) would not work here.
    # We need to use print_exception(...):
    traceback.print_exception(exc_type, exc_value, enriched_tb)

def _add_missing_frames(tb):
    result = fake_tb(tb.tb_frame, tb.tb_lasti, tb.tb_lineno, tb.tb_next)
    frame = tb.tb_frame.f_back
    while frame:
        result = fake_tb(frame, frame.f_lasti, frame.f_lineno, result)
        frame = frame.f_back
    return result

fake_tb = namedtuple(
    'fake_tb', ('tb_frame', 'tb_lasti', 'tb_lineno', 'tb_next')
)

sys.excepthook = excepthook


def main(argv):
    # set default app name / domain for QSettings()
    QCoreApplication.setApplicationName(APPLICATION_NAME)
    QCoreApplication.setOrganizationName(ORGANIZATION_NAME)
    QCoreApplication.setOrganizationDomain(ORGANIZATION_DOMAIN)
    
    parser = argparse.ArgumentParser()
    parser.set_defaults(func=default_action)

    register_tools()

    subparsers = parser.add_subparsers(title='commands',
                                       description='valid commands')

    parser.add_argument('-lt', '--list-tools', dest='list_tools', action='store_const', const=True)
    parser.add_argument('-ln', '--list-naos', dest='list_naos', action='store_const', const=True)
    parser.add_argument('-c', '--config', dest='config_path', action="store", type=str,
                        default=relative_path("../config/my_config.json"))
    parser.add_argument('-sw', '--single-window', dest='single_window', action='store_true', help='Start debugger in a single window')

    parser.add_argument('-p', '--profile', dest='profile', action="store", type=str, help='Store & read setting from/to alternative config file')

    debugger_parser = subparsers.add_parser("debugger")
    debugger_parser.add_argument('-n', '--nao', dest='nao', action="store", type=str)
    debugger_parser.add_argument('-t', '--tool', choices=ToolsRegistry.tools(ToolTypes.CONNECTION),
                                 dest='tool', action="store", type=str)
    debugger_parser.set_defaults(func=debugger_action)

    spl_monitor_parser = subparsers.add_parser("spl-monitor")
    spl_monitor_parser.set_defaults(func=spl_monitor_action)

    patches_parser = subparsers.add_parser("patches")
    patches_parser.set_defaults(func=patch_processor_action)

    args = parser.parse_args(argv[1:])
    
    forceMdi = QSettings().value("forceMdiMode")
    args.single_window = args.single_window or (forceMdi and bool(int(forceMdi)))

    if args.profile:
        QCoreApplication.setApplicationName(APPLICATION_NAME + '-' + args.profile)

    args.func(args)

    parser.print_help()


def default_action(args):
    debugger = BembelDbugFactory().get()

    if args.list_tools:
        tools = ToolsRegistry.tools(ToolTypes.CONNECTION)
        for tool in tools:
            print(tool)
        sys.exit()
    elif args.list_naos:
        debugger.start()
        time.sleep(2)
        naos = list(debugger.debugger.nao_list.keys)
        naos.sort()
        for nao in naos:
            print(nao)
        sys.exit()

    debugger_action(args, True)


def debugger_action(args, is_default_action=False):
    debugger = BembelDbugFactory().get()
    register_plugins(debugger)

    debugger.config = Config(relative_path(args.config_path), relative_path("../config/default.json"))

    app = QApplication(sys.argv)
    app.setStyleSheet(load_stylesheet(":/bembelDbug/styles/style.qss"))

    if not is_default_action and args.nao and args.tool:
        print("using {1} with {0}".format(args.nao, args.tool))
        ToolLauncher(args.nao, args.tool)
    else:
        main = MainWindowWrapper() if args.single_window else MainWindow()
        main.show()

    debugger.start()
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    sys.exit(app.exec_())

def spl_monitor_action(args):
    print("spl monitor")
    sys.exit()


def halt_and_catch_fire(signal=None, frame=None):
    print("\n   )\n  ) \\\n / ) (\n \\(_)/\n")
    sys.exit(0)

def patch_processor_action(args):
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    app = QApplication(sys.argv)
    main = PatchProcessor()
    main.show()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main(sys.argv)
