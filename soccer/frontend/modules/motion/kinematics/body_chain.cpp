#include "body_chain.h"
#include <framework/math/rotation_matrices.h>

Eigen::Matrix4f BodyChain::DHTransform(const bbipc::Sensors &s, int i) {
    Eigen::Matrix4f transform;
    float q = dh[i].gamma;
    if (dh[i].q) q += s.joints.position[j[i]];
    
    transform <<    cosf(q),    -sinf(q) * cosf(dh[i].alpha),   sinf(q) * sinf(dh[i].alpha),    dh[i].a * cosf(q),
                    sinf(q),    cosf(q) * cosf(dh[i].alpha),    -cosf(q) * sinf(dh[i].alpha),   dh[i].a * sinf(q),
                    0.f,        sinf(dh[i].alpha),              cosf(dh[i].alpha),              dh[i].d,
                    0.f,        0.f,                            0.f,                            1.f;
    return transform;
}

Eigen::Matrix4f BodyChain::transformation(const bbipc::Sensors &sensors, int initial, int final) {
    if (initial == final) 
        return Eigen::Matrix4f::Identity();

    if (initial < final) {
        Eigen::Matrix4f TM = Eigen::Matrix4f::Identity();
        for (int i = initial; i < final; i++) {
            TM *= DHTransform(sensors, i);
        }
        return TM;
    } else {
        return RotMat::invert(transformation(sensors, final, initial));
    }
}

Eigen::Matrix4f BodyChain::transformation(const bbipc::Sensors &sensors, int dir) {
    Eigen::Matrix4f TM;

    if (dir == 1) {
        return transformation(sensors, 0, (int)dh.size());
    } else if (dir == -1) {
        return transformation(sensors, (int)dh.size(), 0);
    }
    else LOG_ERROR << "not a valid transformation direction: " << dir;

    return TM;
}
