#pragma once

#define V4L2_MT9M114_FADE_TO_BLACK V4L2_CID_PRIVATE_BASE
#define V4L2_MT9M114_AE_TARGET_AVERAGE_LUMA (V4L2_CID_PRIVATE_BASE+1)
#define V4L2_MT9M114_AE_TARGET_AVERAGE_LUMA_DARK (V4L2_CID_PRIVATE_BASE+2)
#define V4L2_MT9M114_AE_TARGET_GAIN (V4L2_CID_PRIVATE_BASE+3)
#define V4L2_MT9M114_AE_MIN_VIRT_GAIN (V4L2_CID_PRIVATE_BASE+4)
#define V4L2_MT9M114_AE_MAX_VIRT_GAIN (V4L2_CID_PRIVATE_BASE+5)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_0_0 (V4L2_CID_PRIVATE_BASE+6)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_0_1 (V4L2_CID_PRIVATE_BASE+7)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_0_2 (V4L2_CID_PRIVATE_BASE+8)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_0_3 (V4L2_CID_PRIVATE_BASE+9)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_0_4 (V4L2_CID_PRIVATE_BASE+10)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_1_0 (V4L2_CID_PRIVATE_BASE+11)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_1_1 (V4L2_CID_PRIVATE_BASE+12)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_1_2 (V4L2_CID_PRIVATE_BASE+13)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_1_3 (V4L2_CID_PRIVATE_BASE+14)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_1_4 (V4L2_CID_PRIVATE_BASE+15)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_2_0 (V4L2_CID_PRIVATE_BASE+16)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_2_1 (V4L2_CID_PRIVATE_BASE+17)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_2_2 (V4L2_CID_PRIVATE_BASE+18)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_2_3 (V4L2_CID_PRIVATE_BASE+19)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_2_4 (V4L2_CID_PRIVATE_BASE+20)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_3_0 (V4L2_CID_PRIVATE_BASE+21)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_3_1 (V4L2_CID_PRIVATE_BASE+22)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_3_2 (V4L2_CID_PRIVATE_BASE+23)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_3_3 (V4L2_CID_PRIVATE_BASE+24)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_3_4 (V4L2_CID_PRIVATE_BASE+25)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_4_0 (V4L2_CID_PRIVATE_BASE+26)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_4_1 (V4L2_CID_PRIVATE_BASE+27)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_4_2 (V4L2_CID_PRIVATE_BASE+28)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_4_3 (V4L2_CID_PRIVATE_BASE+29)
#define V4L2_MT9M114_AE_WEIGHT_TABLE_4_4 (V4L2_CID_PRIVATE_BASE+30)

#define NUM_CAMERA_OPTIONS 19

namespace CameraDefinitions {
enum CameraOption {
    AutoExposure = 0, /* 1: Use auto exposure, 0: disable auto exposure. */
    AutoExposureAlgorithm,
    BacklightCompensation, /* 0 - 4 */
    AutoWhiteBalance, /* 1: Use auto white balance, 0: disable auto white balance. */
    Contrast,   /* The contrast in range of [16 .. 64]. Gradients from 0.5 (16) to 2.0 (64).*/
    Exposure, /**< The exposure time in the range of [0 .. 1000]. Time is measured in increments of 100µs. */
    FadeToBlack, /**< Fade to black under low light conditions. 1: enabled, 0: disabled. */
    Gain, /**< The gain level in the range of [0 .. 255]. */
    Hue, /* The hue in range [-22 .. 22] */
    Saturation, /* The saturation in range of [0 .. 255] */
    Sharpness, /* The sharpness in range of [-7 .. 7] */
    WhiteBalance, /**< The white balance in Kelvin [2700 .. 6500] */
    Gamma, /* The gamma value in range [100, 280] */
    PowerLineFrequency, /* The local power frequency (1 = 50Hz, 2 = 60Hz) */
    TargetAverageLuma, /* The target average brightness [0 .. 255] */
    TargetAverageLumaDark, /* The target average brightness for dark [0 .. 255] */
    TargetGain, /* The target analog gain [0 .. 65535] */
    MinGain, /* The minimum value for the analog gain that AE Track is permitted to use [0 .. 65535] */
    MaxGain, /* The maximum value for the analog gain that AE Track is permitted to use [0 .. 65535] */
    Brightness,
	AutoHue,
	AutoFocus,
	Focus
};
};
