import consts
import util

import os
import json


class ConfigFile:
    """
    Wrapper class around bembelbots config files.

    Use the functions Configfile.calibration_json() and
    Configfile.bembelbots_json() to load the files calibration.json / bembelbots.json
    from inside our git.
    """

    @staticmethod
    def bembelbots_json():
        """
        :returns: An instance that loads the bembelbots.json from robocupag git
        """
        return ConfigFile._load_config_in_git(consts.BEMBELBOTS_JSON)

    @staticmethod
    def calibration_json():
        """
        :returns: An instance that loads the bembelbots.json from robocupag git
        """
        return ConfigFile._load_config_in_git(consts.CALIBRATION_JSON)

    @staticmethod
    def _load_config_in_git(rel_file_path:str):
        robocupag_root = util.get_robocupag_root()
        config_file_name = os.path.join(robocupag_root, 
                rel_file_path)
        return ConfigFile(config_file_name)


    def __init__(self, config_file:str):
        """
        :param config_file: Path to json file.
        """
        self._config_file = config_file

        with open(self._config_file, 'r') as f:
            self._config = json.load(f)

    def save(self):
        """
        Write the current configuration to disk.
        """
        with open(self._config_file, 'w') as f:
            json.dump(self._config, f, indent=4, sort_keys=True)

    def robot(self, robotName:str) -> dict:
        """
        :param robotName: Name of the robot.
        :returns: The current configuration of a robot as a dict.
        """
        return self._config[robotName]

    def allRobots(self) -> [(str, dict)]:
        """
        :returns: An iterator containing tupels with form
        (robot_name, robot_config) for all robots.
        """
        return self._config.items()
