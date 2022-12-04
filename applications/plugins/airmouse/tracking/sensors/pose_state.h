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
#ifndef CARDBOARD_SDK_SENSORS_POSE_STATE_H_
#define CARDBOARD_SDK_SENSORS_POSE_STATE_H_

#include "../util/rotation.h"
#include "../util/vector.h"

namespace cardboard {

enum {
    kPoseStateFlagInvalid = 1U << 0,
    kPoseStateFlagInitializing = 1U << 1,
    kPoseStateFlagHas6DoF = 1U << 2,
};

// Stores a head pose pose plus derivatives. This can be used for prediction.
struct PoseState {
    // System wall time.
    int64_t timestamp;

    // Rotation from Sensor Space to Start Space.
    Rotation sensor_from_start_rotation;

    // First derivative of the rotation.
    Vector3 sensor_from_start_rotation_velocity;

    // Current gyroscope bias in rad/s.
    Vector3 bias;

    // The position of the headset.
    Vector3 position = Vector3(0, 0, 0);

    // In the same coordinate frame as the position.
    Vector3 velocity = Vector3(0, 0, 0);

    // Flags indicating the status of the pose.
    uint64_t flags = 0U;
};

} // namespace cardboard

#endif // CARDBOARD_SDK_SENSORS_POSE_STATE_H_
