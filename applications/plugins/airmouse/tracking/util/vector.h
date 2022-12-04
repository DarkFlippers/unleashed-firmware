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
#ifndef CARDBOARD_SDK_UTIL_VECTOR_H_
#define CARDBOARD_SDK_UTIL_VECTOR_H_

#include <array>

namespace cardboard {

// Geometric N-dimensional Vector class.
template <int Dimension>
class Vector {
public:
    // The default constructor zero-initializes all elements.
    Vector();

    // Dimension-specific constructors that are passed individual element values.
    constexpr Vector(double e0, double e1, double e2);
    constexpr Vector(double e0, double e1, double e2, double e3);

    // Constructor for a Vector of dimension N from a Vector of dimension N-1 and
    // a scalar of the correct type, assuming N is at least 2.
    // constexpr Vector(const Vector<Dimension - 1>& v, double s);

    void Set(double e0, double e1, double e2); // Only when Dimension == 3.
    void Set(double e0, double e1, double e2,
             double e3); // Only when Dimension == 4.

    // Mutable element accessor.
    double& operator[](int index) {
        return elem_[index];
    }

    // Element accessor.
    double operator[](int index) const {
        return elem_[index];
    }

    // Returns a Vector containing all zeroes.
    static Vector Zero();

    // Self-modifying operators.
    void operator+=(const Vector& v) {
        Add(v);
    }
    void operator-=(const Vector& v) {
        Subtract(v);
    }
    void operator*=(double s) {
        Multiply(s);
    }
    void operator/=(double s) {
        Divide(s);
    }

    // Unary negation operator.
    Vector operator-() const {
        return Negation();
    }

    // Binary operators.
    friend Vector operator+(const Vector& v0, const Vector& v1) {
        return Sum(v0, v1);
    }
    friend Vector operator-(const Vector& v0, const Vector& v1) {
        return Difference(v0, v1);
    }
    friend Vector operator*(const Vector& v, double s) {
        return Scale(v, s);
    }
    friend Vector operator*(double s, const Vector& v) {
        return Scale(v, s);
    }
    friend Vector operator*(const Vector& v, const Vector& s) {
        return Product(v, s);
    }
    friend Vector operator/(const Vector& v, double s) {
        return Divide(v, s);
    }

    // Self-modifying addition.
    void Add(const Vector& v);
    // Self-modifying subtraction.
    void Subtract(const Vector& v);
    // Self-modifying multiplication by a scalar.
    void Multiply(double s);
    // Self-modifying division by a scalar.
    void Divide(double s);

    // Unary negation.
    Vector Negation() const;

    // Binary component-wise multiplication.
    static Vector Product(const Vector& v0, const Vector& v1);
    // Binary component-wise addition.
    static Vector Sum(const Vector& v0, const Vector& v1);
    // Binary component-wise subtraction.
    static Vector Difference(const Vector& v0, const Vector& v1);
    // Binary multiplication by a scalar.
    static Vector Scale(const Vector& v, double s);
    // Binary division by a scalar.
    static Vector Divide(const Vector& v, double s);

private:
    std::array<double, Dimension> elem_;
};
//------------------------------------------------------------------------------

template <int Dimension>
Vector<Dimension>::Vector() {
    for(int i = 0; i < Dimension; i++) {
        elem_[i] = 0;
    }
}

template <int Dimension>
constexpr Vector<Dimension>::Vector(double e0, double e1, double e2)
    : elem_{e0, e1, e2} {
}

template <int Dimension>
constexpr Vector<Dimension>::Vector(double e0, double e1, double e2, double e3)
    : elem_{e0, e1, e2, e3} {
}
/*
template <>
constexpr Vector<4>::Vector(const Vector<3>& v, double s)
    : elem_{v[0], v[1], v[2], s} {}
*/
template <int Dimension>
void Vector<Dimension>::Set(double e0, double e1, double e2) {
    elem_[0] = e0;
    elem_[1] = e1;
    elem_[2] = e2;
}

template <int Dimension>
void Vector<Dimension>::Set(double e0, double e1, double e2, double e3) {
    elem_[0] = e0;
    elem_[1] = e1;
    elem_[2] = e2;
    elem_[3] = e3;
}

template <int Dimension>
Vector<Dimension> Vector<Dimension>::Zero() {
    Vector<Dimension> v;
    return v;
}

template <int Dimension>
void Vector<Dimension>::Add(const Vector& v) {
    for(int i = 0; i < Dimension; i++) {
        elem_[i] += v[i];
    }
}

template <int Dimension>
void Vector<Dimension>::Subtract(const Vector& v) {
    for(int i = 0; i < Dimension; i++) {
        elem_[i] -= v[i];
    }
}

template <int Dimension>
void Vector<Dimension>::Multiply(double s) {
    for(int i = 0; i < Dimension; i++) {
        elem_[i] *= s;
    }
}

template <int Dimension>
void Vector<Dimension>::Divide(double s) {
    for(int i = 0; i < Dimension; i++) {
        elem_[i] /= s;
    }
}

template <int Dimension>
Vector<Dimension> Vector<Dimension>::Negation() const {
    Vector<Dimension> ret;
    for(int i = 0; i < Dimension; i++) {
        ret.elem_[i] = -elem_[i];
    }
    return ret;
}

template <int Dimension>
Vector<Dimension> Vector<Dimension>::Product(const Vector& v0, const Vector& v1) {
    Vector<Dimension> ret;
    for(int i = 0; i < Dimension; i++) {
        ret.elem_[i] = v0[i] * v1[i];
    }
    return ret;
}

template <int Dimension>
Vector<Dimension> Vector<Dimension>::Sum(const Vector& v0, const Vector& v1) {
    Vector<Dimension> ret;
    for(int i = 0; i < Dimension; i++) {
        ret.elem_[i] = v0[i] + v1[i];
    }
    return ret;
}

template <int Dimension>
Vector<Dimension> Vector<Dimension>::Difference(const Vector& v0, const Vector& v1) {
    Vector<Dimension> ret;
    for(int i = 0; i < Dimension; i++) {
        ret.elem_[i] = v0[i] - v1[i];
    }
    return ret;
}

template <int Dimension>
Vector<Dimension> Vector<Dimension>::Scale(const Vector& v, double s) {
    Vector<Dimension> ret;
    for(int i = 0; i < Dimension; i++) {
        ret.elem_[i] = v[i] * s;
    }
    return ret;
}

template <int Dimension>
Vector<Dimension> Vector<Dimension>::Divide(const Vector& v, double s) {
    Vector<Dimension> ret;
    for(int i = 0; i < Dimension; i++) {
        ret.elem_[i] = v[i] / s;
    }
    return ret;
}

typedef Vector<3> Vector3;
typedef Vector<4> Vector4;

} // namespace cardboard

#endif // CARDBOARD_SDK_UTIL_VECTOR_H_
