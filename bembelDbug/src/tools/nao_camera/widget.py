from PyQt5.QtCore import *
from PyQt5.QtWidgets import (
        QWidget,
        QMessageBox
    )

from bembelapi.config import NaoEvent
from bembelapi.nao import NaoCamImage

import base64
import json

from struct import unpack_from

import consts
from debugger import BembelDbugFactory
from tool_management.toolsregistry import Tool, ConnectionTool
from ui.naocamera_ui import Ui_NaoCameraWidget

from widgets.aspect_ratio_widget import AspectRatioWidget
from widgets.image_canvas import ImageCanvas

from utils.camera_parameters import CameraParameter, _CameraParameter
from utils.vision_result import VisionResult
from utils.config_file import ConfigFile


class NaoCameraWidget(ConnectionTool):
    name = "nao_camera"
    button_text = "Nao Camera"

    def __init__(self):
        ConnectionTool.__init__(self, self.name, self.button_text)
        self.ui = Ui_NaoCameraWidget()
        self.ui.setupUi(self)
        self.setupUi()
        events = BembelDbugFactory.get()
        events.add_listener(NaoEvent.NAO_IMAGE, self.handle_nao_image)
        events.add_listener(NaoEvent.NAO_SAVE_BLACKBOARDS_OK, self.saveBlackboardsOk)
        events.add_listener(NaoEvent.NAO_SAVE_BLACKBOARDS_ERROR, self.saveBlackboardsError)

        # Map parameterName -> SetterFunction that should be called when the parameter
        # was changed.
        self._blackboards = ["VisionData", "CameraParameter", "cameraCalibrationTop", "cameraCalibrationBottom"]

        self._parameterListener = {}
        for i in self._blackboards:
            self._parameterListener[i] = {}

        self._params_cache = {}
        for blackboard in self._blackboards:
            self._params_cache[blackboard] = {}

    def setup(self):

        Tool.setup(self)

        self.connection.recv_images()

        self.setupUiConnections()

        self.register_parameter_listeners()

        events = BembelDbugFactory.get()
        events.add_listener(NaoEvent.NAO_SYMBOL_VALUES, self.handleSymbolValues)

    def register_parameter_listeners(self):
        for blackboard in self._parameterListener.keys():
            for paramName in self._parameterListener[blackboard].keys():
                self.connection.start_receiving_symbol(blackboard, paramName)
    
    def unregister_parameter_listeners(self):
        for blackboard in self._parameterListener.keys():
            for paramName in self._parameterListener[blackboard].keys():
                self.connection.stop_receiving_symbol(blackboard, paramName)

    def handleSymbolValues(self, values, _):
        params = {}
        for blackboard in self._parameterListener.keys():
            if values.get(blackboard):
                params[blackboard] = values[blackboard]

        for blackboard, symbols in params.items():
            for paramName, paramValue in symbols.items():

                if paramName not in self._parameterListener[blackboard].keys():
                    continue

                if self.param_cached(blackboard, paramName, paramValue):
                    continue

                for setter in self._parameterListener[blackboard][paramName]:
                    setter(paramValue)

        self._params_cache = params

    def saveBlackboards(self):
        self.connection.save_blackboards()

    def saveBlackboardsOk(self):
        print("Save blackboards ok!")

    def saveBlackboardsError(self):
        print("Save blackboards error!")

    def normalizeCheckboxState(self, state):
        if state != Qt.Unchecked:
            return 1
        else:
            return 0

    def param_cached(self, blackboard:str, param:str, param_value) -> bool:
        if len(self._params_cache) == 0:
            return False
        if blackboard not in self._params_cache:
            return False
        elif param not in self._params_cache[blackboard].keys():
            return False
        else:
            return param_value == self._params_cache[blackboard][param]

    def shutdown(self):
        self.connection.change_value("VisionData", "showPitchCorrectionBottom", 0)
        self.connection.change_value("VisionData", "showPitchCorrectionTop", 0)
        self.unregister_parameter_listeners()
        self.connection.stop_recv_images()

    def addParameterListener(self, blackboard, parameter, setter):
        parameter_name = parameter

        if isinstance(parameter, _CameraParameter):
            parameter_name = parameter.name()

        if self._parameterListener.get(blackboard) is None:
            return

        if parameter_name not in self._parameterListener[blackboard].keys():
            self._parameterListener[blackboard][parameter_name] = []

        self._parameterListener[blackboard][parameter_name].append(lambda val: setter(int(float(val))))

    def setupUiConnections(self):

        def updateParameterFactory(blackboard, param, obj):
            return lambda: self.update_parameter(blackboard, param, obj.value())

        def connectParameter(parameter, hslider, spinbox):
            minVal, maxVal = parameter.min(), parameter.max()
            hslider.setRange(minVal, maxVal)
            hslider.setSingleStep(parameter.step())
            spinbox.setRange(minVal, maxVal)
            spinbox.setSingleStep(parameter.step())

            hslider.sliderReleased.connect(updateParameterFactory("CameraParameter", parameter, hslider))
            spinbox.editingFinished.connect(updateParameterFactory("CameraParameter", parameter, spinbox))

            # setValue() only triggers valueChanged, if the new value is 
            # diffrent from the old one. So this wont create an endless loop. ;D
            hslider.valueChanged.connect(copyValue(spinbox, hslider))
            spinbox.editingFinished.connect(copyValue(hslider, spinbox))

            self.addParameterListener("CameraParameter", parameter, spinbox.setValue)
            self.addParameterListener("CameraParameter", parameter, hslider.setValue)

        def connectPitchCorrection(blackboard, parameter, spinbox):
            self.addParameterListener(blackboard, parameter, spinbox.setValue)
            spinbox.editingFinished.connect(updateParameterFactory(blackboard, parameter, spinbox))

        def copyValue(obj1, obj2):
            return lambda: obj1.setValue(obj2.value())

        def connectCheckbox(blackboard, symbol, checkbox, disable_controls=()):

            def checkboxHandler(state):
                if not checkbox.property("disableConnection"):
                    self.connection.change_value(blackboard, symbol, self.normalizeCheckboxState(state))
                for c in disable_controls:
                    c.setEnabled(state == Qt.Unchecked)

            checkbox.stateChanged.connect(checkboxHandler)

            if symbol not in self._parameterListener[blackboard]:
                self._parameterListener[blackboard][symbol] = []

            def connChangeHandler(state):
                checkbox.setProperty("disableConnection", True)
                newstate = Qt.Checked if int(state) == 1 else Qt.Unchecked
                checkbox.setCheckState(newstate)
                checkbox.setProperty("disableConnection", False)

            self._parameterListener[blackboard][symbol].append(connChangeHandler)            

        def connectCombobox(blackboard, symbol, combobox, labels):
            combobox.addItems(labels)
            combobox.setCurrentIndex(0)
            combobox.currentIndexChanged.connect(lambda val: self.connection.change_value(blackboard, symbol, val))
            if symbol not in self._parameterListener[blackboard]:
                self._parameterListener[blackboard][symbol] = []
            self._parameterListener[blackboard][symbol].append(lambda val: combobox.setCurrentIndex(int(val)))

        connectParameter(
            CameraParameter.exposureTop,
            self.ui.hslider_exposure_top,
            self.ui.spinbox_exposure_top
        )

        connectParameter(
            CameraParameter.exposureBottom,
            self.ui.hslider_exposure_bottom,
            self.ui.spinbox_exposure_bottom
        )

        connectParameter(
            CameraParameter.gain,
            self.ui.hslider_gain,
            self.ui.spinbox_gain
        )

        connectParameter(
            CameraParameter.brightnessTop,
            self.ui.hslider_brightnessTop,
            self.ui.spinbox_brightnessTop
        )

        connectParameter(
            CameraParameter.brightnessBottom,
            self.ui.hslider_brightnessBottom,
            self.ui.spinbox_brightnessBottom
        )

        connectParameter(
            CameraParameter.contrast,
            self.ui.hslider_contrast,
            self.ui.spinbox_contrast
        )

        connectParameter(
            CameraParameter.saturation,
            self.ui.hslider_saturation,
            self.ui.spinbox_saturation
        )

        connectParameter(
            CameraParameter.whiteBalance,
            self.ui.hslider_whitebalance,
            self.ui.spinbox_whitebalance
        )

        connectCheckbox(
            "CameraParameter",
            "autoExposure",
            self.ui.checkBox_autoexposure,
            disable_controls=(
                self.ui.label_exposure_top,
                self.ui.hslider_exposure_top,
                self.ui.spinbox_exposure_top,
                self.ui.label_exposure_bottom,
                self.ui.hslider_exposure_bottom,
                self.ui.spinbox_exposure_bottom,
                self.ui.label_gain,
                self.ui.hslider_gain,
                self.ui.spinbox_gain
            )
        )

        connectCheckbox(
            "CameraParameter",
            "autoWhiteBalancing",
            self.ui.checkBox_autowhitebalance,
            disable_controls=(
                self.ui.label_whitebalance,
                self.ui.hslider_whitebalance,
                self.ui.spinbox_whitebalance
            )
        )
    

        # camera pitch correction

        self.ui.tabWidget_camera_settings.currentChanged.connect(self.tab_camera_settings_changed)
        # connectPitchCorrection("cameraCalibrationTop", "pitchCorrection", self.ui.spinbox_pitch_correction_top)
        # connectPitchCorrection("cameraCalibrationBottom", "pitchCorrection", self.ui.spinbox_pitch_correction_bottom)

        connectPitchCorrection("VisionData", "ballDistanceMeasuredTop", self.ui.top_spinBox_ball_dist_measured)
        connectPitchCorrection("VisionData", "ballDistanceMeasuredBottom", self.ui.bottom_spinBox_ball_dist_measured)

        self.ui.pushButton_save_parameters.released.connect(self.saveBlackboards)
       
    def show_error_message(self, error_description):
        messagebox = QMessageBox(QMessageBox.Critical, 'Error', 
                error_description, QMessageBox.Ok)
        messagebox.show()

    def parameter_snapshot(self):

        parameters = {}

        parameters['exposureTop']      = self.ui.hslider_exposure_top.value()
        parameters['exposureBottom']   = self.ui.hslider_exposure_bottom.value()
        parameters['brightnessTop']    = self.ui.hslider_brightnessTop.value()
        parameters['brightnessBottom'] = self.ui.hslider_brightnessBottom.value()
        parameters['gain']             = self.ui.hslider_gain.value()
        parameters['contrast']         = self.ui.hslider_contrast.value()
        parameters['saturation']       = self.ui.hslider_saturation.value()
        parameters['sharpness']        = self.ui.hslider_sharpness.value()
        parameters['whiteBalance']     = self.ui.hslider_whitebalance.value()

        return parameters

    def save_config(self, for_all_robots: bool):
        """
        :returns: Returns if config file was saved succesfully.
        """

        calibration = ConfigFile.calibration_json()

        new_parameters = self.parameter_snapshot()

        edited_robots = []
        something_changed = False

        def set_new_parameters(config:dict) -> bool:
            value_changed = False
            for parameter, value in new_parameters.items():
                value_changed |= (config[parameter] != value)
                config[parameter] = value
            return value_changed

        if for_all_robots:
            for robot_name, robot_calibration in calibration.allRobots():
                edited_robots.append(robot_name)
                something_changed |= set_new_parameters(robot_calibration["camera"])
        else:
            curr_robot = self.connection.name
            robot_calibration = calibration.robot(curr_robot)
            edited_robots.append(curr_robot)
            something_changed |= set_new_parameters(robot_calibration["camera"])

        

        if something_changed:
            calibration.save()

        return True

    def tab_camera_settings_changed(self, index):
        tab = self.ui.tabWidget_camera_settings.currentWidget()  # type: QWidget

        if tab.objectName() == "tab_pitch_correction":
            self.connection.change_value("VisionData", "showPitchCorrectionBottom", 1)
            self.connection.change_value("VisionData", "showPitchCorrectionTop", 1)
        else:
            self.connection.change_value("VisionData", "showPitchCorrectionBottom", 0)
            self.connection.change_value("VisionData", "showPitchCorrectionTop", 0)

    def update_parameter(self, blackboard, param, value):
        parameter_name = param

        if isinstance(param, _CameraParameter):
            parameter_name = param.name()
            step = param.step()
            print(value, step)
            if step > 1:
                value = round(value/step) * step
                print("newval", value)

        self.connection.change_value(blackboard, parameter_name, value)

    # noinspection PyAttributeOutsideInit
    def setupUi(self):

        self._topImageCanvas = ImageCanvas()
        self._botImageCanvas = ImageCanvas()

        self.ui.tab_cameras.layout().addWidget(AspectRatioWidget(self._topImageCanvas, 4, 3))
        self.ui.tab_cameras.layout().addWidget(AspectRatioWidget(self._botImageCanvas, 4, 3))

    def handle_nao_image(self, img: NaoCamImage, addr):
        vr = [ VisionResult(result) for result in img.get_vision_results() ]
        for result in vr:
            if result.vtype == "ball":
                self.ui.display_ball_pos_rcs.setText("{:.2f};{:.2f}".format(result.rcs_x1, result.rcs_y1))

        if img.get_camera() == consts.TOP_CAMERA:
            self._topImageCanvas.updateImg(img)
            if vr:
                self._topImageCanvas.updateVisionResults(vr)        
        else:
            self._botImageCanvas.updateImg(img)
            if vr:
                self._botImageCanvas.updateVisionResults(vr)        
