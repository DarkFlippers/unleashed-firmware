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
#ifndef CARDBOARD_SDK_UTIL_ROTATION_H_
#define CARDBOARD_SDK_UTIL_ROTATION_H_

#include "matrix_3x3.h"
#include "vector.h"
#include "vectorutils.h"

namespace cardboard {

// The Rotation class represents a rotation around a 3-dimensional axis. It
// uses normalized quaternions internally to make the math robust.
class Rotation {
public:
    // Convenience typedefs for vector of the correct type.
    typedef Vector<3> VectorType;
    typedef Vector<4> QuaternionType;

    // The default constructor creates an identity Rotation, which has no effect.
    Rotation() {
        quat_.Set(0, 0, 0, 1);
    }

    // Returns an identity Rotation, which has no effect.
    static Rotation Identity() {
        return Rotation();
    }

    // Sets the Rotation from a quaternion (4D vector), which is first normalized.
    void SetQuaternion(const QuaternionType& quaternion) {
        quat_ = Normalized(quaternion);
    }

    // Returns the Rotation as a normalized quaternion (4D vector).
    const QuaternionType& GetQuaternion() const {
        return quat_;
    }

    // Sets the Rotation to rotate by the given angle around the given axis,
    // following the right-hand rule. The axis does not need to be unit
    // length. If it is zero length, this results in an identity Rotation.
    void SetAxisAndAngle(const VectorType& axis, double angle);

    // Returns the right-hand rule axis and angle corresponding to the
    // Rotation. If the Rotation is the identity rotation, this returns the +X
    // axis and an angle of 0.
    void GetAxisAndAngle(VectorType* axis, double* angle) const;

    // Convenience function that constructs and returns a Rotation given an axis
    // and angle.
    static Rotation FromAxisAndAngle(const VectorType& axis, double angle) {
        Rotation r;
        r.SetAxisAndAngle(axis, angle);
        return r;
    }

    // Convenience function that constructs and returns a Rotation given a
    // quaternion.
    static Rotation FromQuaternion(const QuaternionType& quat) {
        Rotation r;
        r.SetQuaternion(quat);
        return r;
    }

    // Convenience function that constructs and returns a Rotation given a
    // rotation matrix R with $R^\top R = I && det(R) = 1$.
    static Rotation FromRotationMatrix(const Matrix3x3& mat);

    // Convenience function that constructs and returns a Rotation given Euler
    // angles that are applied in the order of rotate-Z by roll, rotate-X by
    // pitch, rotate-Y by yaw (same as GetRollPitchYaw).
    static Rotation FromRollPitchYaw(double roll, double pitch, double yaw) {
        VectorType x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
        return FromAxisAndAngle(z, roll) * (FromAxisAndAngle(x, pitch) * FromAxisAndAngle(y, yaw));
    }

    // Convenience function that constructs and returns a Rotation given Euler
    // angles that are applied in the order of rotate-Y by yaw, rotate-X by
    // pitch, rotate-Z by roll (same as GetYawPitchRoll).
    static Rotation FromYawPitchRoll(double yaw, double pitch, double roll) {
        VectorType x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
        return FromAxisAndAngle(y, yaw) * (FromAxisAndAngle(x, pitch) * FromAxisAndAngle(z, roll));
    }

    // Constructs and returns a Rotation that rotates one vector to another along
    // the shortest arc. This returns an identity rotation if either vector has
    // zero length.
    static Rotation RotateInto(const VectorType& from, const VectorType& to);

    // The negation operator returns the inverse rotation.
    friend Rotation operator-(const Rotation& r) {
        // Because we store normalized quaternions, the inverse is found by
        // negating the vector part.
        return Rotation(-r.quat_[0], -r.quat_[1], -r.quat_[2], r.quat_[3]);
    }

    // Appends a rotation to this one.
    Rotation& operator*=(const Rotation& r) {
        const QuaternionType& qr = r.quat_;
        QuaternionType& qt = quat_;
        SetQuaternion(QuaternionType(
            qr[3] * qt[0] + qr[0] * qt[3] + qr[2] * qt[1] - qr[1] * qt[2],
            qr[3] * qt[1] + qr[1] * qt[3] + qr[0] * qt[2] - qr[2] * qt[0],
            qr[3] * qt[2] + qr[2] * qt[3] + qr[1] * qt[0] - qr[0] * qt[1],
            qr[3] * qt[3] - qr[0] * qt[0] - qr[1] * qt[1] - qr[2] * qt[2]));
        return *this;
    }

    // Binary multiplication operator - returns a composite Rotation.
    friend const Rotation operator*(const Rotation& r0, const Rotation& r1) {
        Rotation r = r0;
        r *= r1;
        return r;
    }

    // Multiply a Rotation and a Vector to get a Vector.
    VectorType operator*(const VectorType& v) const;

private:
    // Private constructor that builds a Rotation from quaternion components.
    Rotation(double q0, double q1, double q2, double q3)
        : quat_(q0, q1, q2, q3) {
    }

    // Applies a Rotation to a Vector to rotate the Vector. Method borrowed from:
    // http://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
    VectorType ApplyToVector(const VectorType& v) const {
        VectorType im(quat_[0], quat_[1], quat_[2]);
        VectorType temp = 2.0 * Cross(im, v);
        return v + quat_[3] * temp + Cross(im, temp);
    }

    // The rotation represented as a normalized quaternion. (Unit quaternions are
    // required for constructing rotation matrices, so it makes sense to always
    // store them that way.) The vector part is in the first 3 elements, and the
    // scalar part is in the last element.
    QuaternionType quat_;
};

} // namespace cardboard

#endif // CARDBOARD_SDK_UTIL_ROTATION_H_
