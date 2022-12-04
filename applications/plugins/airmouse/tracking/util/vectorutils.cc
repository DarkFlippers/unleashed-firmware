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
#include "vectorutils.h"

namespace cardboard {

// Returns the dot (inner) product of two Vectors.
double Dot(const Vector<3>& v0, const Vector<3>& v1)
{
    return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

// Returns the dot (inner) product of two Vectors.
double Dot(const Vector<4>& v0, const Vector<4>& v1)
{
    return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2] + v0[3] * v1[3];
}

// Returns the 3-dimensional cross product of 2 Vectors. Note that this is
// defined only for 3-dimensional Vectors.
Vector<3> Cross(const Vector<3>& v0, const Vector<3>& v1)
{
    return Vector<3>(v0[1] * v1[2] - v0[2] * v1[1], v0[2] * v1[0] - v0[0] * v1[2],
        v0[0] * v1[1] - v0[1] * v1[0]);
}

} // namespace cardboard
