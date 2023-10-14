#pragma once

#include <Eigen/Core>
#define _USE_MATH_DEFINES

class RotMat
{
private:
    static const float M_PI_3_4;

public:
    static Eigen::Matrix3f rotateX(const float &alpha);
    static Eigen::Matrix3f rotateY(const float &alpha);
    static Eigen::Matrix3f rotateZ(const float &alpha);
    static Eigen::Matrix3f rotateYZL(const float &alpha);
    static Eigen::Matrix3f rotateYZR(const float &alpha);
    
    static Eigen::Matrix3f revRotateX(const float &alpha);
    static Eigen::Matrix3f revRotateY(const float &alpha);
    static Eigen::Matrix3f revRotateZ(const float &alpha);
    static Eigen::Matrix3f revRotateYZL(const float &alpha);
    static Eigen::Matrix3f revRotateYZR(const float &alpha);

    static Eigen::Matrix3f rotateXYZ(const float &alpha, const float &beta, const float &gamma);
    static Eigen::Matrix3f rotateXYZ(const Eigen::Vector3f &alpha);
    static Eigen::Matrix3f rotateZYX(const Eigen::Vector3f &alpha);



    // homogenous variant:
    static Eigen::Matrix4f rotateHX(const float &alpha);
    static Eigen::Matrix4f rotateHY(const float &alpha);
    static Eigen::Matrix4f rotateHZ(const float &alpha);

    static Eigen::Matrix4f transformX(const float &alpha, const Eigen::Vector3f &tran);
    static Eigen::Matrix4f transformY(const float &alpha, const Eigen::Vector3f &tran);
    static Eigen::Matrix4f transformZ(const float &alpha, const Eigen::Vector3f &tran);
    static Eigen::Matrix4f transformXYZ(const Eigen::Vector3f &rot, const Eigen::Vector3f &tran);
    static Eigen::Matrix4f transformXYZ(const float &alpha, const float &beta, const float &gamma, const Eigen::Vector3f &tran);

    static Eigen::Matrix4f translate(const Eigen::Vector3f &tran);
    static Eigen::Matrix4f translate(float x, float y, float z);

    static Eigen::Matrix4f invert(const Eigen::Matrix4f &M);

    static Eigen::Matrix4f rotateHYZL(const float &alpha);
    static Eigen::Matrix4f rotateHYZR(const float &alpha);

    static Eigen::Matrix4f rotateRPY(const Eigen::Vector3f &rot);
    static Eigen::Matrix4f rotateRPY(const float &alpha, const float &beta, const float &gamma);

    static Eigen::Vector4f homogenous(const Eigen::Vector3f &vec);
    static Eigen::Vector3f dehomogenous(const Eigen::Vector4f &vec);
};

