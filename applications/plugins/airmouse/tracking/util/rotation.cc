/*
 * Copyright 2019 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "rotation.h"

#include <cmath>
#include <limits>

#include "vectorutils.h"

namespace cardboard {

void Rotation::SetAxisAndAngle(const VectorType& axis, double angle)
{
    VectorType unit_axis = axis;
    if (!Normalize(&unit_axis)) {
        *this = Identity();
    } else {
        double a = angle / 2;
        const double s = sin(a);
        const VectorType v(unit_axis * s);
        SetQuaternion(QuaternionType(v[0], v[1], v[2], cos(a)));
    }
}

Rotation Rotation::FromRotationMatrix(const Matrix3x3& mat)
{
    static const double kOne = 1.0;
    static const double kFour = 4.0;

    const double d0 = mat(0, 0), d1 = mat(1, 1), d2 = mat(2, 2);
    const double ww = kOne + d0 + d1 + d2;
    const double xx = kOne + d0 - d1 - d2;
    const double yy = kOne - d0 + d1 - d2;
    const double zz = kOne - d0 - d1 + d2;

    const double max = std::max(ww, std::max(xx, std::max(yy, zz)));
    if (ww == max) {
        const double w4 = sqrt(ww * kFour);
        return Rotation::FromQuaternion(QuaternionType((mat(2, 1) - mat(1, 2)) / w4,
            (mat(0, 2) - mat(2, 0)) / w4, (mat(1, 0) - mat(0, 1)) / w4, w4 / kFour));
    }

    if (xx == max) {
        const double x4 = sqrt(xx * kFour);
        return Rotation::FromQuaternion(QuaternionType(x4 / kFour, (mat(0, 1) + mat(1, 0)) / x4,
            (mat(0, 2) + mat(2, 0)) / x4, (mat(2, 1) - mat(1, 2)) / x4));
    }

    if (yy == max) {
        const double y4 = sqrt(yy * kFour);
        return Rotation::FromQuaternion(QuaternionType((mat(0, 1) + mat(1, 0)) / y4, y4 / kFour,
            (mat(1, 2) + mat(2, 1)) / y4, (mat(0, 2) - mat(2, 0)) / y4));
    }

    // zz is the largest component.
    const double z4 = sqrt(zz * kFour);
    return Rotation::FromQuaternion(QuaternionType((mat(0, 2) + mat(2, 0)) / z4,
        (mat(1, 2) + mat(2, 1)) / z4, z4 / kFour, (mat(1, 0) - mat(0, 1)) / z4));
}

void Rotation::GetAxisAndAngle(VectorType* axis, double* angle) const
{
    VectorType vec(quat_[0], quat_[1], quat_[2]);
    if (Normalize(&vec)) {
        *angle = 2 * acos(quat_[3]);
        *axis = vec;
    } else {
        *axis = VectorType(1, 0, 0);
        *angle = 0.0;
    }
}

Rotation Rotation::RotateInto(const VectorType& from, const VectorType& to)
{
    static const double kTolerance = std::numeric_limits<double>::epsilon() * 100;

    // Directly build the quaternion using the following technique:
    // http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
    const double norm_u_norm_v = sqrt(LengthSquared(from) * LengthSquared(to));
    double real_part = norm_u_norm_v + Dot(from, to);
    VectorType w;
    if (real_part < kTolerance * norm_u_norm_v) {
        // If |from| and |to| are exactly opposite, rotate 180 degrees around an
        // arbitrary orthogonal axis. Axis normalization can happen later, when we
        // normalize the quaternion.
        real_part = 0.0;
        w = (abs(from[0]) > abs(from[2])) ? VectorType(-from[1], from[0], 0)
                                          : VectorType(0, -from[2], from[1]);
    } else {
        // Otherwise, build the quaternion the standard way.
        w = Cross(from, to);
    }

    // Build and return a normalized quaternion.
    // Note that Rotation::FromQuaternion automatically performs normalization.
    return Rotation::FromQuaternion(QuaternionType(w[0], w[1], w[2], real_part));
}

Rotation::VectorType Rotation::operator*(const Rotation::VectorType& v) const
{
    return ApplyToVector(v);
}

} // namespace cardboard
