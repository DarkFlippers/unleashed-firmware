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
#ifndef CARDBOARD_SDK_UTIL_MATRIX_3X3_H_
#define CARDBOARD_SDK_UTIL_MATRIX_3X3_H_

#include <array>
#include <cstring> // For memcpy().
#include <istream> // NOLINT
#include <ostream> // NOLINT

namespace cardboard {

// The Matrix3x3 class defines a square 3-dimensional matrix. Elements are
// stored in row-major order.
// TODO(b/135461889): Make this class consistent with Matrix4x4.
class Matrix3x3 {
public:
    // The default constructor zero-initializes all elements.
    Matrix3x3();

    // Dimension-specific constructors that are passed individual element values.
    Matrix3x3(
        double m00,
        double m01,
        double m02,
        double m10,
        double m11,
        double m12,
        double m20,
        double m21,
        double m22);

    // Constructor that reads elements from a linear array of the correct size.
    explicit Matrix3x3(const double array[3 * 3]);

    // Returns a Matrix3x3 containing all zeroes.
    static Matrix3x3 Zero();

    // Returns an identity Matrix3x3.
    static Matrix3x3 Identity();

    // Mutable element accessors.
    double& operator()(int row, int col) {
        return elem_[row][col];
    }
    std::array<double, 3>& operator[](int row) {
        return elem_[row];
    }

    // Read-only element accessors.
    const double& operator()(int row, int col) const {
        return elem_[row][col];
    }
    const std::array<double, 3>& operator[](int row) const {
        return elem_[row];
    }

    // Return a pointer to the data for interfacing with libraries.
    double* Data() {
        return &elem_[0][0];
    }
    const double* Data() const {
        return &elem_[0][0];
    }

    // Self-modifying multiplication operators.
    void operator*=(double s) {
        MultiplyScalar(s);
    }
    void operator*=(const Matrix3x3& m) {
        *this = Product(*this, m);
    }

    // Unary operators.
    Matrix3x3 operator-() const {
        return Negation();
    }

    // Binary scale operators.
    friend Matrix3x3 operator*(const Matrix3x3& m, double s) {
        return Scale(m, s);
    }
    friend Matrix3x3 operator*(double s, const Matrix3x3& m) {
        return Scale(m, s);
    }

    // Binary matrix addition.
    friend Matrix3x3 operator+(const Matrix3x3& lhs, const Matrix3x3& rhs) {
        return Addition(lhs, rhs);
    }

    // Binary matrix subtraction.
    friend Matrix3x3 operator-(const Matrix3x3& lhs, const Matrix3x3& rhs) {
        return Subtraction(lhs, rhs);
    }

    // Binary multiplication operator.
    friend Matrix3x3 operator*(const Matrix3x3& m0, const Matrix3x3& m1) {
        return Product(m0, m1);
    }

    // Exact equality and inequality comparisons.
    friend bool operator==(const Matrix3x3& m0, const Matrix3x3& m1) {
        return AreEqual(m0, m1);
    }
    friend bool operator!=(const Matrix3x3& m0, const Matrix3x3& m1) {
        return !AreEqual(m0, m1);
    }

private:
    // These private functions implement most of the operators.
    void MultiplyScalar(double s);
    Matrix3x3 Negation() const;
    static Matrix3x3 Addition(const Matrix3x3& lhs, const Matrix3x3& rhs);
    static Matrix3x3 Subtraction(const Matrix3x3& lhs, const Matrix3x3& rhs);
    static Matrix3x3 Scale(const Matrix3x3& m, double s);
    static Matrix3x3 Product(const Matrix3x3& m0, const Matrix3x3& m1);
    static bool AreEqual(const Matrix3x3& m0, const Matrix3x3& m1);

    std::array<std::array<double, 3>, 3> elem_;
};

} // namespace cardboard

#endif // CARDBOARD_SDK_UTIL_MATRIX_3X3_H_
