class _CameraParameter:
    def __init__(self, name, minValue, maxValue, step=1):
        self._name = name
        self._minValue = minValue
        self._maxValue = maxValue
        self._step = step

    def name(self):
        return self._name

    def min(self):
        return self._minValue

    def max(self):
        return self._maxValue

    def step(self):
        return self._step


class CameraParameter:
    exposureTop = _CameraParameter("exposureTop", 1, 35000) # clamped to useful range, valid range is much higher
    exposureBottom = _CameraParameter("exposureBottom", 1, 35000)
    gain = _CameraParameter("gain", 0, 1023)
    brightnessTop = _CameraParameter("brightnessTop", -255, 254)
    brightnessBottom = _CameraParameter("brightnessBottom", -255, 254)
    contrast = _CameraParameter("contrast", 0, 255)
    saturation = _CameraParameter("saturation", 0, 255)
    whiteBalance = _CameraParameter("whiteBalance", 2500, 6500, step=500)
