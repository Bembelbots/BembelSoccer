#include "rotation_matrices.h"

const float RotMat::M_PI_3_4 = 3.f * M_PI_4;


Eigen::Matrix3f RotMat::rotateX(const float &alpha) {
    Eigen::Matrix3f rotate;
    rotate <<   1,  0,          0,
                0,  cosf(alpha), -sinf(alpha),
                0,  sinf(alpha), cosf(alpha);
    return rotate;
}

Eigen::Matrix3f RotMat::rotateY(const float &alpha) {
    Eigen::Matrix3f rotate;
    rotate <<   cosf(alpha), 0,  sinf(alpha),
                0,          1,  0,
                -sinf(alpha), 0, cosf(alpha);
    return rotate;
}


Eigen::Matrix3f RotMat::rotateZ(const float &alpha) {
    Eigen::Matrix3f rotate;
    rotate <<   cosf(alpha), -sinf(alpha),    0,
                sinf(alpha), cosf(alpha),     0,
                0,          0,              1;
    return rotate;
}

Eigen::Matrix3f RotMat::rotateXYZ(const float &alpha, const float &beta, const float &gamma) {
	return rotateX(alpha)*rotateY(beta)*rotateZ(gamma);
}

Eigen::Matrix3f RotMat::rotateXYZ(const Eigen::Vector3f &alpha) {
	return rotateX(alpha[0])*rotateY(alpha[1])*rotateZ(alpha[2]);
}

Eigen::Matrix3f RotMat::rotateZYX(const Eigen::Vector3f &alpha) {
	return rotateZ(alpha[2])*rotateY(alpha[1])*rotateX(alpha[0]);
}

Eigen::Matrix3f RotMat::rotateYZL(const float &alpha) {
    // M_PI_3_4 : angle between rotation-axis and z-axis
	return rotateX(M_PI_3_4)*rotateZ(-alpha)*rotateX(-M_PI_3_4);
}

Eigen::Matrix3f RotMat::rotateYZR(const float &alpha) {
    // -M_PI_3_4 : angle between rotation-axis and z-axis
    return rotateX(-M_PI_3_4)*rotateZ(alpha)*rotateX(M_PI_3_4);
}

Eigen::Matrix3f RotMat::revRotateX(const float &alpha) {
    Eigen::Matrix3f rotate;
    rotate <<   1,  0,          0,
                0,  cosf(-alpha), -sinf(-alpha),
                0,  sinf(-alpha), cosf(-alpha);
    return rotate;
}

Eigen::Matrix3f RotMat::revRotateY(const float &alpha) {
    Eigen::Matrix3f rotate;
    rotate <<   cosf(-alpha), 0,  sinf(-alpha),
                0,          1,  0,
                -sinf(-alpha), 0, cosf(-alpha);
    return rotate;
}

Eigen::Matrix3f RotMat::revRotateZ(const float &alpha) {
    Eigen::Matrix3f rotate;
    rotate <<   cosf(-alpha), -sinf(-alpha),    0,
                sinf(-alpha), cosf(-alpha),     0,
                0,          0,              1;
    return rotate;
}

Eigen::Matrix3f RotMat::revRotateYZL(const float &alpha) {
    // M_PI_3_4 : angle between rotation-axis and z-axis
	return rotateX(M_PI_3_4)*rotateZ(alpha)*rotateX(-M_PI_3_4);
}

Eigen::Matrix3f RotMat::revRotateYZR(const float &alpha) {
    // -M_PI_3_4 : angle between rotation-axis and z-axis
    return rotateX(-M_PI_3_4)*rotateZ(-alpha)*rotateX(M_PI_3_4);
}




// Homogene Variante:

Eigen::Matrix4f RotMat::rotateHX(const float &alpha) {
    Eigen::Matrix4f rotate;
    rotate <<   1,  0,          0,          0,
                0,  cosf(alpha), -sinf(alpha),0,
                0,  sinf(alpha), cosf(alpha), 0,
                0,  0,          0,          1;
    return rotate;
}

Eigen::Matrix4f RotMat::rotateHY(const float &alpha) {
    Eigen::Matrix4f rotate;
    rotate <<   cosf(alpha), 0,  sinf(alpha), 0,
                0,          1,  0,          0,
                -sinf(alpha),0,  cosf(alpha), 0,
                0,          0,  0,          1;
    return rotate;
}


Eigen::Matrix4f RotMat::rotateHZ(const float &alpha) {
    Eigen::Matrix4f rotate;
    rotate <<   cosf(alpha), -sinf(alpha),    0,  0,
                sinf(alpha), cosf(alpha),     0,  0,
                0,          0,              1,  0,
                0,          0,              0,  1;
    return rotate;
}

Eigen::Matrix4f RotMat::transformX(const float &alpha, const Eigen::Vector3f &tran) {
    Eigen::Matrix4f mat;
    mat <<  1,  0,          0,              tran[0],
            0,  cosf(alpha), -sinf(alpha),    tran[1],
            0,  sinf(alpha), cosf(alpha),     tran[2],
            0,  0,          0,              1;
    return mat;
}

Eigen::Matrix4f RotMat::transformY(const float &alpha, const Eigen::Vector3f &tran) {
    Eigen::Matrix4f mat;
    mat <<  cosf(alpha), 0,  sinf(alpha),     tran[0],
            0,          1,  0,              tran[1],
            -sinf(alpha),0,  cosf(alpha),     tran[2],
            0,          0,  0,              1;
    return mat;
}

Eigen::Matrix4f RotMat::transformZ(const float &alpha, const Eigen::Vector3f &tran) {
    Eigen::Matrix4f mat;
    mat <<  cosf(alpha), -sinf(alpha),    0,  tran[0],
            sinf(alpha), cosf(alpha),     0,  tran[1],
            0,          0,              1,  tran[2],
            0,          0,              0,  1;
    return mat;
}

Eigen::Matrix4f RotMat::translate(const Eigen::Vector3f &tran) {
    Eigen::Matrix4f mat;
    mat <<  1,  0,  0,  tran[0],
            0,  1,  0,  tran[1],
            0,  0,  1,  tran[2],
            0,  0,  0,  1;
    return mat;
}

Eigen::Matrix4f RotMat::translate(float x, float y, float z) {
    Eigen::Matrix4f mat;
    mat <<  1,  0,  0,  x,
            0,  1,  0,  y,
            0,  0,  1,  z,
            0,  0,  0,  1;
    return mat;
}

Eigen::Matrix4f RotMat::invert(const Eigen::Matrix4f &M) {
        Eigen::Matrix3f R = M.block<3,3>(0,0);
        R.transposeInPlace();
        Eigen::Vector3f r = M.block<3,1>(0,3);

        r = - R * r; 
        
        Eigen::Matrix4f T = Eigen::Matrix4f::Identity();
        T.block<3,3>(0,0) = R;
        T.block<3,1>(0,3) = r;

        return T;
    }

Eigen::Matrix4f RotMat::rotateHYZL(const float &alpha) {
    // M_PI_3_4 : angle between rotation-axis and z-axis
	return rotateHX(M_PI_3_4)*rotateHZ(-alpha)*rotateHX(-M_PI_3_4);
}

Eigen::Matrix4f RotMat::rotateHYZR(const float &alpha) {
    // -M_PI_3_4 : angle between rotation-axis and z-axis
    return rotateHX(-M_PI_3_4)*rotateHZ(alpha)*rotateHX(M_PI_3_4);
}

Eigen::Matrix4f RotMat::rotateRPY(const Eigen::Vector3f &rot) {
    return rotateHZ(rot[2]) * ( rotateHY(rot[1]) * rotateHX(rot[0]) );
}

Eigen::Matrix4f RotMat::rotateRPY(const float &alpha, const float &beta, const float &gamma) {
    return rotateHZ(gamma) * ( rotateHY(beta) * rotateHX(alpha) );
}

Eigen::Matrix4f RotMat::transformXYZ(const Eigen::Vector3f &rot, const Eigen::Vector3f &tran) {
    return transformX(rot[0], tran) * transformY(rot[1], tran) * transformZ(rot[2], tran);
}

Eigen::Matrix4f RotMat::transformXYZ(const float &alpha, const float &beta, const float &gamma, const Eigen::Vector3f &tran) {
    return transformX(alpha, tran) * transformY(beta, tran) * transformZ(gamma, tran);
}

Eigen::Vector4f RotMat::homogenous(const Eigen::Vector3f &vec) {
    Eigen::Vector4f v;
    v << vec[0], vec[1], vec[2], 1.f;
    return v;
}

Eigen::Vector3f RotMat::dehomogenous(const Eigen::Vector4f &vec) {
    Eigen::Vector3f v;
    v << vec[0], vec[1], vec[2];
    return v;
}
