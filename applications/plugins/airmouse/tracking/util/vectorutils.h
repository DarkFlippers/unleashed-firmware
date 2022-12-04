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
#ifndef CARDBOARD_SDK_UTIL_VECTORUTILS_H_
#define CARDBOARD_SDK_UTIL_VECTORUTILS_H_

//
// This file contains free functions that operate on Vector instances.
//

#include <cmath>

#include "vector.h"

namespace cardboard {

// Returns the dot (inner) product of two Vectors.
double Dot(const Vector<3>& v0, const Vector<3>& v1);

// Returns the dot (inner) product of two Vectors.
double Dot(const Vector<4>& v0, const Vector<4>& v1);

// Returns the 3-dimensional cross product of 2 Vectors. Note that this is
// defined only for 3-dimensional Vectors.
Vector<3> Cross(const Vector<3>& v0, const Vector<3>& v1);

// Returns the square of the length of a Vector.
template <int Dimension>
double LengthSquared(const Vector<Dimension>& v) {
    return Dot(v, v);
}

// Returns the geometric length of a Vector.
template <int Dimension>
double Length(const Vector<Dimension>& v) {
    return sqrt(LengthSquared(v));
}

// the Vector untouched and returns false.
template <int Dimension>
bool Normalize(Vector<Dimension>* v) {
    const double len = Length(*v);
    if(len == 0) {
        return false;
    } else {
        (*v) /= len;
        return true;
    }
}

// Returns a unit-length version of a Vector. If the given Vector has no
// length, this returns a Zero() Vector.
template <int Dimension>
Vector<Dimension> Normalized(const Vector<Dimension>& v) {
    Vector<Dimension> result = v;
    if(Normalize(&result))
        return result;
    else
        return Vector<Dimension>::Zero();
}

} // namespace cardboard

#endif // CARDBOARD_SDK_UTIL_VECTORUTILS_H_
