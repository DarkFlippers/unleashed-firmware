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
#pragma once

#include <array>
#include <memory>
#include <mutex> // NOLINT

#include "sensors/accelerometer_data.h"
#include "sensors/gyroscope_data.h"
#include "sensors/sensor_fusion_ekf.h"
#include "util/rotation.h"

namespace cardboard {

// OrientationTracker encapsulates pose tracking by connecting sensors
// to SensorFusion.
// This pose tracker reports poses in display space.
class OrientationTracker {
public:
    OrientationTracker(const long sampling_period_ns);

    void SetCalibration(const Vector3& calibration);

    // Pauses tracking and sensors.
    void Pause();

    // Resumes tracking ans sensors.
    void Resume();

    // Gets the predicted pose for a given timestamp.
    Vector4 GetPose(int64_t timestamp_ns) const;

    // Function called when receiving AccelerometerData.
    //
    // @param event sensor event.
    void OnAccelerometerData(const AccelerometerData& event);

    // Function called when receiving GyroscopeData.
    //
    // @param event sensor event.
    Vector4 OnGyroscopeData(const GyroscopeData& event);

private:
    long sampling_period_ns_;
    Vector3 calibration_;

    std::atomic<bool> is_tracking_;
    // Sensor Fusion object that stores the internal state of the filter.
    std::unique_ptr<SensorFusionEkf> sensor_fusion_;
    // Latest gyroscope data.
    GyroscopeData latest_gyroscope_data_;
};

} // namespace cardboard
