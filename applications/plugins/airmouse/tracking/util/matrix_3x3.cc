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
#include "matrix_3x3.h"

namespace cardboard {

Matrix3x3::Matrix3x3(double m00, double m01, double m02, double m10, double m11, double m12,
    double m20, double m21, double m22)
    : elem_ { { { m00, m01, m02 }, { m10, m11, m12 }, { m20, m21, m22 } } }
{
}

Matrix3x3::Matrix3x3()
{
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            elem_[row][col] = 0;
    }
}

Matrix3x3 Matrix3x3::Zero()
{
    Matrix3x3 result;
    return result;
}

Matrix3x3 Matrix3x3::Identity()
{
    Matrix3x3 result;
    for (int row = 0; row < 3; ++row) {
        result.elem_[row][row] = 1;
    }
    return result;
}

void Matrix3x3::MultiplyScalar(double s)
{
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            elem_[row][col] *= s;
    }
}

Matrix3x3 Matrix3x3::Negation() const
{
    Matrix3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            result.elem_[row][col] = -elem_[row][col];
    }
    return result;
}

Matrix3x3 Matrix3x3::Scale(const Matrix3x3& m, double s)
{
    Matrix3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            result.elem_[row][col] = m.elem_[row][col] * s;
    }
    return result;
}

Matrix3x3 Matrix3x3::Addition(const Matrix3x3& lhs, const Matrix3x3& rhs)
{
    Matrix3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            result.elem_[row][col] = lhs.elem_[row][col] + rhs.elem_[row][col];
    }
    return result;
}

Matrix3x3 Matrix3x3::Subtraction(const Matrix3x3& lhs, const Matrix3x3& rhs)
{
    Matrix3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            result.elem_[row][col] = lhs.elem_[row][col] - rhs.elem_[row][col];
    }
    return result;
}

Matrix3x3 Matrix3x3::Product(const Matrix3x3& m0, const Matrix3x3& m1)
{
    Matrix3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            result.elem_[row][col] = 0;
            for (int i = 0; i < 3; ++i)
                result.elem_[row][col] += m0.elem_[row][i] * m1.elem_[i][col];
        }
    }
    return result;
}

bool Matrix3x3::AreEqual(const Matrix3x3& m0, const Matrix3x3& m1)
{
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (m0.elem_[row][col] != m1.elem_[row][col])
                return false;
        }
    }
    return true;
}

} // namespace cardboard
