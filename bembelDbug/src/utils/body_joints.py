class _BodyJoint:
	def __init__(self, name, minValue, maxValue):
		self._name = name
		self._minValue = minValue
		self._maxValue = maxValue

	def name(self):
		return self._name

	def min(self):
		return self._minValue

	def max(self):
		return self._maxValue

class BodyJoints:
	l_shoulder_pitch = _BodyJoint("l_shoulder_pitch", -2.0857, 2.0857)
	r_shoulder_pitch = _BodyJoint("r_shoulder_pitch", -2.0857, 2.0857)
	l_shoulder_roll = _BodyJoint("l_shoulder_roll", -0.3142, 1.3265)
	r_shoulder_roll = _BodyJoint("r_shoulder_roll", -1.3265, 0.3142)
	l_elbow_yaw = _BodyJoint("l_elbow_yaw", -2.0875, 2.0875)
	r_elbow_yaw = _BodyJoint("r_elbow_yaw", -2.0875, 2.0875)
	l_elbow_roll = _BodyJoint("l_elbow_roll", -1.5446, -0.0349)
	r_elbow_roll = _BodyJoint("r_elbow_roll", 0.0349, 1.5446)

	hip_yaw_pitch = _BodyJoint("l_hip_yaw_pitch", -1.145303, 0.740810)

	l_hip_roll = _BodyJoint("l_hip_roll", -0.3794, 0.7904)
	r_hip_roll = _BodyJoint("r_hip_roll", -0.7904, 0.3794)
	l_hip_pitch = _BodyJoint("l_hip_pitch", -1.5358, 0.4840)
	r_hip_pitch = _BodyJoint("r_hip_pitch", -1.5358, 0.4849)
	l_knee_pitch = _BodyJoint("l_knee_pitch", -0.0923, 2.1125)
	r_knee_pitch = _BodyJoint("r_knee_pitch", -0.1030, 2.1201)
	l_ankle_pitch = _BodyJoint("l_ankle_pitch", -1.1895, 0.9227)
	r_ankle_pitch = _BodyJoint("r_ankle_pitch", -1.1864, 0.9320)
	l_ankle_roll = _BodyJoint("l_ankle_roll", -0.3978, 0.7690)
	r_ankle_roll = _BodyJoint("r_ankle_roll", -0.7689, 0.3979)


class ButtonManager:
	def __init__(self):
		self.buttonDict = {
			"lShoulderPitchStiffness":False,
			"lShoulderRollStiffness":False,
			"lElbowYawStiffness":False,
			"lElbowRollStiffness":False,
			"lWristYawStiffness":False,
			"rShoulderPitchStiffness":False,
			"rShoulderRollStiffness":False,
			"rElbowYawStiffness":False,
			"rElbowRollStiffness":False,
			"rWristYawStiffness":False,
			"lHipYawPitchStiffness":False,
			"lHipRollStiffness":False,
			"lHipPitchStiffness":False,
			"lKneePitchStiffness":False,
			"lAnklePitchStiffness":False,
			"lAnkleRollStiffness":False,
			"rHipRollStiffness":False,
			"rHipPitchStiffness":False,
			"rKneePitchStiffness":False,
			"rAnklePitchStiffness":False,
			"rAnkleRollStiffness":False,
		};
		return

	def setButtonCheckedState(self, buttonName, state):
		if(buttonName in self.buttonDict):
			self.buttonDict[buttonName] = state
		else:
			raise Exception("buttonName unknown!")

	def getButtonCheckedState(self, buttonName):
		# Check if labelName contained in labelDict
		if(buttonName in self.buttonDict):
			return self.buttonDict[buttonName]
		else:
			raise Exception("buttonName unknown!")

	def getIsAnyActive(self):
		isAnyActive = False
		for val in self.buttonDict.values():
			if(val):
				isAnyActive = True

		print("isAnyActive: ", isAnyActive)
		return isAnyActive
