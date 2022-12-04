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
#ifndef CARDBOARD_SDK_UTIL_MATRIX_4X4_H_
#define CARDBOARD_SDK_UTIL_MATRIX_4X4_H_

#include <array>

namespace cardboard {

class Matrix4x4 {
public:
    static Matrix4x4 Identity();
    static Matrix4x4 Zeros();
    static Matrix4x4 Translation(float x, float y, float z);
    static Matrix4x4 Perspective(const std::array<float, 4>& fov, float zNear, float zFar);
    void ToArray(float* array) const;

private:
    std::array<std::array<float, 4>, 4> m;
};

} // namespace cardboard

#endif // CARDBOARD_SDK_UTIL_MATRIX4X4_H_
