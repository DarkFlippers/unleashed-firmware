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
#ifndef CARDBOARD_SDK_SENSORS_GYROSCOPE_BIAS_ESTIMATOR_H_
#define CARDBOARD_SDK_SENSORS_GYROSCOPE_BIAS_ESTIMATOR_H_

#include <chrono> // NOLINT
#include <cstdint>
#include <list>
#include <memory>
#include <vector>

#include "lowpass_filter.h"
#include "mean_filter.h"
#include "median_filter.h"
#include "../util/vector.h"

namespace cardboard {

// Class that attempts to estimate the gyroscope's bias.
// Its main idea is that it averages the gyroscope values when the phone is
// considered stationary.
// Usage: A client should call the ProcessGyroscope and ProcessAccelerometer
// methods for every accelerometer and gyroscope sensor sample. This class
// expects these calls to be frequent, i.e., at least at 10 Hz. The client can
// then call GetGyroBias to retrieve the current estimate of the gyroscope bias.
// For best results, the fastest available delay option should be used when
// registering to sensors. Note that this class is not thread-safe.
//
// The filtering applied to the accelerometer to estimate a rotation
// from it follows :
// Baptiste Delporte, Laurent Perroton, Thierry Grandpierre, Jacques Trichet.
// Accelerometer and Magnetometer Based Gyroscope Emulation on Smart Sensor
// for a Virtual Reality Application. Sensor and Transducers Journal, 2012.
//
// which is a combination of a IIR filter, a median and a mean filter.
class GyroscopeBiasEstimator {
public:
    GyroscopeBiasEstimator();
    virtual ~GyroscopeBiasEstimator();

    // Updates the estimator with a gyroscope event.
    //
    // @param gyroscope_sample the angular speed around the x, y, z axis in
    //     radians/sec.
    // @param timestamp_ns the nanosecond at which the event occurred. Only
    //     guaranteed to be comparable with timestamps from other PocessGyroscope
    //     invocations.
    virtual void ProcessGyroscope(const Vector3& gyroscope_sample, uint64_t timestamp_ns);

    // Processes accelerometer samples to estimate if device is
    // stable or not.
    //
    // First we filter the accelerometer. This is done with 3 filters.
    //  - A IIR low-pass filter
    //  - A median filter
    //  - A mean filter.
    // Then a rotation is computed between consecutive filtered accelerometer
    // samples.
    // Finally this is converted to a velocity to emulate a gyroscope.
    //
    // @param accelerometer_sample the acceleration (including gravity) on the x,
    //     y, z axis in meters/s^2.
    // @param timestamp_ns the nanosecond at which the event occurred. Only
    //     guaranteed to be comparable with timestamps from other
    //     ProcessAccelerometer invocations.
    virtual void ProcessAccelerometer(const Vector3& accelerometer_sample, uint64_t timestamp_ns);

    // Returns the estimated gyroscope bias.
    //
    // @return Estimated gyroscope bias. A vector with zeros is returned if no
    //     estimate has been computed.
    virtual Vector3 GetGyroscopeBias() const;

    // Resets the estimator state.
    void Reset();

    // Returns true if the current estimate returned by GetGyroscopeBias is
    // correct. The device (measured using the sensors) has to be static for this
    // function to return true.
    virtual bool IsCurrentEstimateValid() const;

private:
    // A helper class to keep track of whether some signal can be considered
    // static over specified number of frames.
    class IsStaticCounter;

    // Updates gyroscope bias estimation.
    //
    // @return false if the current sample is too large.
    bool UpdateGyroscopeBias(const Vector3& gyroscope_sample, uint64_t timestamp_ns);

    // Returns device angular velocity (rad/s) from the latest accelerometer data.
    //
    // @param timestep in seconds between the last two samples.
    // @return rotation velocity from latest accelerometer. This can be
    // interpreted as an gyroscope.
    Vector3 ComputeAngularVelocityFromLatestAccelerometer(double timestep) const;

    LowpassFilter accelerometer_lowpass_filter_;
    LowpassFilter simulated_gyroscope_from_accelerometer_lowpass_filter_;
    LowpassFilter gyroscope_lowpass_filter_;
    LowpassFilter gyroscope_bias_lowpass_filter_;

    std::unique_ptr<IsStaticCounter> accelerometer_static_counter_;
    std::unique_ptr<IsStaticCounter> gyroscope_static_counter_;

    // Sum of the weight of sample used for gyroscope filtering.
    float current_accumulated_weights_gyroscope_bias_;

    // Set of filters for accelerometer data to estimate a rotation
    // based only on accelerometer.
    MeanFilter mean_filter_;
    MedianFilter median_filter_;

    // Last computed filter accelerometer value used for finite differences.
    Vector3 last_mean_filtered_accelerometer_value_;
};

} // namespace cardboard

#endif // CARDBOARD_SDK_SENSORS_GYROSCOPE_BIAS_ESTIMATOR_H_
