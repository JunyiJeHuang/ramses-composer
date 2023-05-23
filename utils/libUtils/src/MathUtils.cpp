/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "utils/MathUtils.h"

#include <algorithm>
#include <cmath>
#include <glm/detail/type_half.hpp>
#include <glm/glm.hpp>
#include <math.h>
#define PI 3.1415926

namespace raco::utils::math {

float wrapPi(float theta) {
    if (fabs(theta) <= PI) {
        const float TWOPI = 2.0f * PI;
        float revolutions = floor((theta + PI) * (1.0f / TWOPI));
        theta -= revolutions * TWOPI;
    }
    return theta;
}

float wrapAngle(float angle) {
    if (fabs(angle) > 180) {
        float revolutions = floor((angle + 180) * (1.0f / 360));
        angle -= revolutions * 360;
    }
    return angle;
}

float keepFourPrecision(float v) {
    float temp = floor(v * 10000.0000f + 0.5);
    v = temp / 10000.0000f;
    return v;
}

std::array<double, 3> eulerAngle(double lastX, double lastY, double lastZ, double curX, double curY, double curZ) {
    double eulerX = wrapAngle(curX - lastX) + lastX;
    double eulerY = wrapAngle(curY - lastY) + lastY;
    double eulerZ = wrapAngle(curZ - lastZ) + lastZ;
    return {eulerX, eulerY, eulerZ};
}

std::array<double, 3> quaternionToZYXDegrees(double qX, double qY, double qZ, double qW) {
    // Compute 3x3 rotation matrix as intermediate representation
    const float x2 = qX + qX;
    const float y2 = qY + qY;
    const float z2 = qZ + qZ;
    const float xx = qX * x2;
    const float xy = qX * y2;
    const float xz = qX * z2;
    const float yy = qY * y2;
    const float yz = qY * z2;
    const float zz = qZ * z2;
    const float wx = qW * x2;
    const float wy = qW * y2;
    const float wz = qW * z2;

    const float m11 = (1 - (yy + zz));
//      const float m12 = (xy - wz);
    const float m21 = (xy + wz);
    const float m22 = (1 - (xx + zz));
    const float m31 = (xz - wy);
    const float m32 = (yz + wx);
//      const float m13 = (xz + wy);
    const float m23 = (yz - wx);
    const float m33 = (1 - (xx + yy));

    float eulerX = 0.f;
    float eulerY = 0.f;
    float eulerZ = 0.f;
    double cy = sqrtf(m11 * m11 + m21 * m21);
    if (cy > 16 * FLT_EPSILON) {
        eulerX = atan2f(m32, m33);
        eulerY = atan2f(-m31, cy);
        eulerZ = atan2f(m21, m11);
    } else {
        eulerX = atan2f(-m23, m22);
        eulerY = atan2f(-m31, cy);
        eulerZ = 0;
    }

    return {glm::degrees(eulerX), glm::degrees(eulerY), glm::degrees(eulerZ)};
}

std::array<double, 3> quaternionToXYZDegrees(double qX, double qY, double qZ, double qW) {
    // algorithm copied from ramses-logic (RotationUtils.cpp)
    // to align our quaternion calculation with ramses-logic's when using quaternion links

    // Compute 3x3 rotation matrix as intermediate representation
    const float x2 = qX + qX;
    const float y2 = qY + qY;
    const float z2 = qZ + qZ;
    const float xx = qX * x2;
    const float xy = qX * y2;
    const float xz = qX * z2;
    const float yy = qY * y2;
    const float yz = qY * z2;
    const float zz = qZ * z2;
    const float wx = qW * x2;
    const float wy = qW * y2;
    const float wz = qW * z2;

    const float m11 = (1 - (yy + zz));
    const float m12 = (xy - wz);
    const float m22 = (1 - (xx + zz));
    const float m32 = (yz + wx);
    const float m13 = (xz + wy);
    const float m23 = (yz - wx);
    const float m33 = (1 - (xx + yy));

    // Compute euler XYZ angles from matrix values
    float eulerX = 0.f;
    const float eulerY = std::asin(std::clamp(m13, -1.f, 1.f));
    float eulerZ = 0.f;
    if (std::abs(m13) < 1.f) {
        eulerX = std::atan2(-m23, m33);
        eulerZ = std::atan2(-m12, m11);
    } else {
        eulerX = std::atan2(m32, m22);
    }

    return {glm::degrees(eulerX), glm::degrees(eulerY), glm::degrees(eulerZ)};
}

uint16_t twoBytesToHalfFloat(unsigned char firstByte, unsigned char secondByte) {
	float twoBytes = (firstByte << 8) | (secondByte & 0xff);
	float bytesNormalized = (twoBytes / (std::numeric_limits<uint16_t>::max()));

	return glm::detail::toFloat16(bytesNormalized);
}


}  // namespace raco::utils::path
