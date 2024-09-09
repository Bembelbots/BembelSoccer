# automatically generated by the FlatBuffers compiler, do not modify

# namespace: 

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Joints(object):
    __slots__ = ['_tab']

    @classmethod
    def SizeOf(cls):
        return 200

    # Joints
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Joints
    def Temperatures(self, j = None):
        if j is None:
            return [self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0 + i * 4)) for i in range(self.TemperaturesLength())]
        elif j >= 0 and j < self.TemperaturesLength():
            return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0 + j * 4))
        else:
            return None

    # Joints
    def TemperaturesAsNumpy(self):
        return self._tab.GetArrayAsNumpy(flatbuffers.number_types.Float32Flags, self._tab.Pos + 0, self.TemperaturesLength())

    # Joints
    def TemperaturesLength(self):
        return 25

    # Joints
    def TemperaturesIsNone(self):
        return False

    # Joints
    def Status(self, j = None):
        if j is None:
            return [self._tab.Get(flatbuffers.number_types.Int32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(100 + i * 4)) for i in range(self.StatusLength())]
        elif j >= 0 and j < self.StatusLength():
            return self._tab.Get(flatbuffers.number_types.Int32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(100 + j * 4))
        else:
            return None

    # Joints
    def StatusAsNumpy(self):
        return self._tab.GetArrayAsNumpy(flatbuffers.number_types.Int32Flags, self._tab.Pos + 100, self.StatusLength())

    # Joints
    def StatusLength(self):
        return 25

    # Joints
    def StatusIsNone(self):
        return False


def CreateJoints(builder, temperatures, status):
    builder.Prep(4, 200)
    for _idx0 in range(25 , 0, -1):
        builder.PrependInt32(status[_idx0-1])
    for _idx0 in range(25 , 0, -1):
        builder.PrependFloat32(temperatures[_idx0-1])
    return builder.Offset()

try:
    from typing import List
except:
    pass

class JointsT(object):

    # JointsT
    def __init__(self):
        self.temperatures = None  # type: List[float]
        self.status = None  # type: List[int]

    @classmethod
    def InitFromBuf(cls, buf, pos):
        joints = Joints()
        joints.Init(buf, pos)
        return cls.InitFromObj(joints)

    @classmethod
    def InitFromPackedBuf(cls, buf, pos=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, pos)
        return cls.InitFromBuf(buf, pos+n)

    @classmethod
    def InitFromObj(cls, joints):
        x = JointsT()
        x._UnPack(joints)
        return x

    def __eq__(self, other):
        return type(self) == type(other) and \
            self.temperatures == other.temperatures and \
            self.status == other.status

    # JointsT
    def _UnPack(self, joints):
        if joints is None:
            return
        if not joints.TemperaturesIsNone():
            if np is None:
                self.temperatures = []
                for i in range(joints.TemperaturesLength()):
                    self.temperatures.append(joints.Temperatures(i))
            else:
                self.temperatures = joints.TemperaturesAsNumpy()
        if not joints.StatusIsNone():
            if np is None:
                self.status = []
                for i in range(joints.StatusLength()):
                    self.status.append(joints.Status(i))
            else:
                self.status = joints.StatusAsNumpy()

    # JointsT
    def Pack(self, builder):
        return CreateJoints(builder, self.temperatures, self.status)
