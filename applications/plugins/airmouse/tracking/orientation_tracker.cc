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
#include "orientation_tracker.h"

#include "sensors/pose_prediction.h"
#include "util/logging.h"
#include "util/vector.h"
#include "util/vectorutils.h"

namespace cardboard {

OrientationTracker::OrientationTracker(const long sampling_period_ns)
    : sampling_period_ns_(sampling_period_ns)
    , calibration_(Vector3::Zero())
    , is_tracking_(false)
    , sensor_fusion_(new SensorFusionEkf())
    , latest_gyroscope_data_({ 0, 0, Vector3::Zero() })
{
    sensor_fusion_->SetBiasEstimationEnabled(/*kGyroBiasEstimationEnabled*/ true);
}

void OrientationTracker::SetCalibration(const Vector3& calibration) {
    calibration_ = calibration;
}

void OrientationTracker::Pause()
{
    if (!is_tracking_) {
        return;
    }

    // Create a gyro event with zero velocity. This effectively stops the prediction.
    GyroscopeData event = latest_gyroscope_data_;
    event.data = Vector3::Zero();

    OnGyroscopeData(event);

    is_tracking_ = false;
}

void OrientationTracker::Resume() { is_tracking_ = true; }

Vector4 OrientationTracker::GetPose(int64_t timestamp_ns) const
{
    Rotation predicted_rotation;
    const PoseState pose_state = sensor_fusion_->GetLatestPoseState();
    if (sensor_fusion_->IsFullyInitialized()) {
        predicted_rotation = pose_state.sensor_from_start_rotation;
    } else {
        CARDBOARD_LOGI("Tracker not fully initialized yet. Using pose prediction only.");
        predicted_rotation = pose_prediction::PredictPose(timestamp_ns, pose_state);
    }

    return (-predicted_rotation).GetQuaternion();
}

void OrientationTracker::OnAccelerometerData(const AccelerometerData& event)
{
    if (!is_tracking_) {
        return;
    }
    sensor_fusion_->ProcessAccelerometerSample(event);
}

Vector4 OrientationTracker::OnGyroscopeData(const GyroscopeData& event)
{
    if (!is_tracking_) {
        return Vector4();
    }

    const GyroscopeData data = { .system_timestamp = event.system_timestamp,
        .sensor_timestamp_ns = event.sensor_timestamp_ns,
        .data = event.data - calibration_ };

    latest_gyroscope_data_ = data;

    sensor_fusion_->ProcessGyroscopeSample(data);

    return OrientationTracker::GetPose(data.sensor_timestamp_ns + sampling_period_ns_);
}

} // namespace cardboard
