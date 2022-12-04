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
#ifndef CARDBOARD_SDK_UTIL_MATRIXUTILS_H_
#define CARDBOARD_SDK_UTIL_MATRIXUTILS_H_

//
// This file contains operators and free functions that define generic Matrix
// operations.
//

#include "matrix_3x3.h"
#include "rotation.h"
#include "vector.h"

namespace cardboard {

// Returns the transpose of a matrix.
Matrix3x3 Transpose(const Matrix3x3& m);

// Multiplies a Matrix and a column Vector of the same Dimension to produce
// another column Vector.
Vector3 operator*(const Matrix3x3& m, const Vector3& v);

// Returns the determinant of the matrix. This function is defined for all the
// typedef'ed Matrix types.
double Determinant(const Matrix3x3& m);

// Returns the adjugate of the matrix, which is defined as the transpose of the
// cofactor matrix. This function is defined for all the typedef'ed Matrix
// types.  The determinant of the matrix is computed as a side effect, so it is
// returned in the determinant parameter if it is not null.
Matrix3x3 AdjugateWithDeterminant(const Matrix3x3& m, double* determinant);

// Returns the inverse of the matrix. This function is defined for all the
// typedef'ed Matrix types.  The determinant of the matrix is computed as a
// side effect, so it is returned in the determinant parameter if it is not
// null. If the determinant is 0, the returned matrix has all zeroes.
Matrix3x3 InverseWithDeterminant(const Matrix3x3& m, double* determinant);

// Returns the inverse of the matrix. This function is defined for all the
// typedef'ed Matrix types. If the determinant of the matrix is 0, the returned
// matrix has all zeroes.
Matrix3x3 Inverse(const Matrix3x3& m);

// Returns a 3x3 Matrix representing a 3D rotation. This creates a Matrix that
// does not work with homogeneous coordinates, so the function name ends in
// "NH".
Matrix3x3 RotationMatrixNH(const Rotation& r);

} // namespace cardboard

#endif // CARDBOARD_SDK_UTIL_MATRIXUTILS_H_
