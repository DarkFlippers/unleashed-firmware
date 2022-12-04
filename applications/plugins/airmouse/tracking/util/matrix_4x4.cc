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
#include "matrix_4x4.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace cardboard {

Matrix4x4 Matrix4x4::Identity()
{
    Matrix4x4 ret;
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            ret.m[j][i] = (i == j) ? 1 : 0;
        }
    }

    return ret;
}

Matrix4x4 Matrix4x4::Zeros()
{
    Matrix4x4 ret;
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            ret.m[j][i] = 0;
        }
    }

    return ret;
}

Matrix4x4 Matrix4x4::Translation(float x, float y, float z)
{
    Matrix4x4 ret = Matrix4x4::Identity();
    ret.m[3][0] = x;
    ret.m[3][1] = y;
    ret.m[3][2] = z;

    return ret;
}

Matrix4x4 Matrix4x4::Perspective(const std::array<float, 4>& fov, float zNear, float zFar)
{
    Matrix4x4 ret = Matrix4x4::Zeros();

    const float xLeft = -std::tan(fov[0] * M_PI / 180.0f) * zNear;
    const float xRight = std::tan(fov[1] * M_PI / 180.0f) * zNear;
    const float yBottom = -std::tan(fov[2] * M_PI / 180.0f) * zNear;
    const float yTop = std::tan(fov[3] * M_PI / 180.0f) * zNear;

    const float X = (2 * zNear) / (xRight - xLeft);
    const float Y = (2 * zNear) / (yTop - yBottom);
    const float A = (xRight + xLeft) / (xRight - xLeft);
    const float B = (yTop + yBottom) / (yTop - yBottom);
    const float C = (zNear + zFar) / (zNear - zFar);
    const float D = (2 * zNear * zFar) / (zNear - zFar);

    ret.m[0][0] = X;
    ret.m[2][0] = A;
    ret.m[1][1] = Y;
    ret.m[2][1] = B;
    ret.m[2][2] = C;
    ret.m[3][2] = D;
    ret.m[2][3] = -1;

    return ret;
}

void Matrix4x4::ToArray(float* array) const { std::memcpy(array, &m[0][0], 16 * sizeof(float)); }

} // namespace cardboard
