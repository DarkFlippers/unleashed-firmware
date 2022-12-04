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
#ifndef CARDBOARD_SDK_SENSORS_POSE_PREDICTION_H_
#define CARDBOARD_SDK_SENSORS_POSE_PREDICTION_H_

#include <cstdint>

#include "pose_state.h"
#include "../util/rotation.h"

namespace cardboard {
namespace pose_prediction {

// Returns a rotation matrix based on the integration of the gyroscope_value
// over the timestep_s in seconds.
// TODO(pfg): Document the space better here.
//
// @param gyroscope_value gyroscope sensor values.
// @param timestep_s integration period in seconds.
// @return Integration of the gyroscope value the rotation is from Start to
//         Sensor Space.
Rotation GetRotationFromGyroscope(const Vector3& gyroscope_value, double timestep_s);

// Gets a predicted pose for a given time in the future (e.g. rendering time)
// based on a linear prediction model. This uses the system current state
// (position, velocity, etc) from the past to extrapolate a position in the
// future.
//
// @param requested_pose_timestamp time at which you want the pose.
// @param current_state current state that stores the pose and linear model at a
//        given time prior to requested_pose_timestamp_ns.
// @return pose from Start to Sensor Space.
Rotation PredictPose(int64_t requested_pose_timestamp, const PoseState& current_state);

// Equivalent to PredictPose, but for use with poses relative to Start Space
// rather than sensor space.
Rotation PredictPoseInv(int64_t requested_pose_timestamp, const PoseState& current_state);

} // namespace pose_prediction
} // namespace cardboard

#endif // CARDBOARD_SDK_SENSORS_POSE_PREDICTION_H_
