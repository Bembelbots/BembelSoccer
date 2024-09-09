import bbapi
from bbapi import RobotRole, RobotName

class Constants:
    NUM_PLAYERS = 7
    TEAM_ID = 3
    # Using indexing only works when no values are skipped in the (flat buffer) enum (must be a continues range of ints).
    ROBOT_ROLES = list(bbapi.RobotRole.RobotRole.__dict__)[1:-3]
    ROBOT_NAMES = list(bbapi.RobotName.RobotName.__dict__)[1:-3]