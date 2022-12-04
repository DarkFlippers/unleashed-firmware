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
#include "matrixutils.h"

#include "vectorutils.h"

namespace cardboard {

namespace {

    // Returns true if the cofactor for a given row and column should be negated.
    static bool IsCofactorNegated(int row, int col)
    {
        // Negated iff (row + col) is odd.
        return ((row + col) & 1) != 0;
    }

    static double CofactorElement3(const Matrix3x3& m, int row, int col)
    {
        static const int index[3][2] = { { 1, 2 }, { 0, 2 }, { 0, 1 } };
        const int i0 = index[row][0];
        const int i1 = index[row][1];
        const int j0 = index[col][0];
        const int j1 = index[col][1];
        const double cofactor = m(i0, j0) * m(i1, j1) - m(i0, j1) * m(i1, j0);
        return IsCofactorNegated(row, col) ? -cofactor : cofactor;
    }

    // Multiplies a matrix and some type of column vector to
    // produce another column vector of the same type.
    Vector3 MultiplyMatrixAndVector(const Matrix3x3& m, const Vector3& v)
    {
        Vector3 result = Vector3::Zero();
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col)
                result[row] += m(row, col) * v[col];
        }
        return result;
    }

    // Sets the upper 3x3 of a Matrix to represent a 3D rotation.
    void RotationMatrix3x3(const Rotation& r, Matrix3x3* matrix)
    {
        //
        // Given a quaternion (a,b,c,d) where d is the scalar part, the 3x3 rotation
        // matrix is:
        //
        //   a^2 - b^2 - c^2 + d^2         2ab - 2cd               2ac + 2bd
        //         2ab + 2cd        -a^2 + b^2 - c^2 + d^2         2bc - 2ad
        //         2ac - 2bd               2bc + 2ad        -a^2 - b^2 + c^2 + d^2
        //
        const Vector<4>& quat = r.GetQuaternion();
        const double aa = quat[0] * quat[0];
        const double bb = quat[1] * quat[1];
        const double cc = quat[2] * quat[2];
        const double dd = quat[3] * quat[3];

        const double ab = quat[0] * quat[1];
        const double ac = quat[0] * quat[2];
        const double bc = quat[1] * quat[2];

        const double ad = quat[0] * quat[3];
        const double bd = quat[1] * quat[3];
        const double cd = quat[2] * quat[3];

        Matrix3x3& m = *matrix;
        m[0][0] = aa - bb - cc + dd;
        m[0][1] = 2 * ab - 2 * cd;
        m[0][2] = 2 * ac + 2 * bd;
        m[1][0] = 2 * ab + 2 * cd;
        m[1][1] = -aa + bb - cc + dd;
        m[1][2] = 2 * bc - 2 * ad;
        m[2][0] = 2 * ac - 2 * bd;
        m[2][1] = 2 * bc + 2 * ad;
        m[2][2] = -aa - bb + cc + dd;
    }

} // anonymous namespace

Vector3 operator*(const Matrix3x3& m, const Vector3& v) { return MultiplyMatrixAndVector(m, v); }

Matrix3x3 CofactorMatrix(const Matrix3x3& m)
{
    Matrix3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            result(row, col) = CofactorElement3(m, row, col);
    }
    return result;
}

Matrix3x3 AdjugateWithDeterminant(const Matrix3x3& m, double* determinant)
{
    const Matrix3x3 cofactor_matrix = CofactorMatrix(m);
    if (determinant) {
        *determinant = m(0, 0) * cofactor_matrix(0, 0) + m(0, 1) * cofactor_matrix(0, 1)
            + m(0, 2) * cofactor_matrix(0, 2);
    }
    return Transpose(cofactor_matrix);
}

// Returns the transpose of a matrix.
Matrix3x3 Transpose(const Matrix3x3& m)
{
    Matrix3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            result(row, col) = m(col, row);
    }
    return result;
}

Matrix3x3 InverseWithDeterminant(const Matrix3x3& m, double* determinant)
{
    // The inverse is the adjugate divided by the determinant.
    double det;
    Matrix3x3 adjugate = AdjugateWithDeterminant(m, &det);
    if (determinant)
        *determinant = det;
    if (det == 0)
        return Matrix3x3::Zero();
    else
        return adjugate * (1 / det);
}

Matrix3x3 Inverse(const Matrix3x3& m) { return InverseWithDeterminant(m, nullptr); }

Matrix3x3 RotationMatrixNH(const Rotation& r)
{
    Matrix3x3 m;
    RotationMatrix3x3(r, &m);
    return m;
}

} // namespace cardboard
