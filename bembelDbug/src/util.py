import os
from os.path import dirname
from typing import Optional

from PyQt5.QtCore import QFile
from PyQt5.QtCore import QTextStream


def read_file(path: str) -> str:
    f = QFile(path)
    if not f.exists():
        raise FileNotFoundError

    f.open(QFile.ReadOnly | QFile.Text)
    ts = QTextStream(f)
    result = ts.readAll()
    f.close()
    return result


def write_file(path: str, data: str):
    f = QFile(path)
    f.open(QFile.WriteOnly | QFile.Text)
    f.write(data.encode("utf-8"))
    f.close()


def load_stylesheet(path: str) -> str:
    return read_file(path)


def relative_path(path: str):
    return os.path.join(dirname(__file__), path)

def find_git_root(start_dir: str) -> Optional[str]:
    """
    Traverses the file tree upwards until a '.git' folder is found.

    :start_dir: Directory where the traversal is started.
    :returns: The (absolute) path of the git root.
    """

    git_root = start_dir

    while True:

        git_folder = os.path.join(git_root, '.git')

        if os.path.isdir(git_folder):
            break

        git_root = os.path.join(git_root, '..')
        git_root = os.path.normpath(git_root)

        # Assume that fs root is never a git...
        if os.path.samefile(git_root, '/'):
            git_root = None
            break

    return git_root


def get_robocupag_root() -> str:
    robocupag_root = find_git_root(os.path.abspath(__file__))

    if robocupag_root == None:
        raise Exception("Could not find robocupag root!")

    return robocupag_root

