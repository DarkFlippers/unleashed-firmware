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
#include "gyroscope_bias_estimator.h"

#include <algorithm>
#include <chrono> // NOLINT

#include "../util/rotation.h"
#include "../util/vector.h"

namespace {

// Cutoff frequencies in Hertz applied to our various signals, and their
// corresponding filters.
const float kAccelerometerLowPassCutOffFrequencyHz = 1.0f;
const float kRotationVelocityBasedAccelerometerLowPassCutOffFrequencyHz = 0.15f;
const float kGyroscopeLowPassCutOffFrequencyHz = 1.0f;
const float kGyroscopeBiasLowPassCutOffFrequencyHz = 0.15f;

// Note that MEMS IMU are not that precise.
const double kEpsilon = 1.0e-8;

// Size of the filtering window for the mean and median filter. The larger the
// windows the larger the filter delay.
const int kFilterWindowSize = 5;

// Threshold used to compare rotation computed from the accelerometer and the
// gyroscope bias.
const double kRatioBetweenGyroBiasAndAccel = 1.5;

// The minimum sum of weights we need to acquire before returning a bias
// estimation.
const float kMinSumOfWeightsGyroBiasThreshold = 25.0f;

// Amount of change in m/s^3 we allow on the smoothed accelerometer values to
// consider the phone static.
const double kAccelerometerDeltaStaticThreshold = 0.5;

// Amount of change in radians/s^2 we allow on the smoothed gyroscope values to
// consider the phone static.
const double kGyroscopeDeltaStaticThreshold = 0.03;

// If the gyroscope value is above this threshold, don't update the gyroscope
// bias estimation. This threshold is applied to the magnitude of gyroscope
// vectors in radians/s.
const float kGyroscopeForBiasThreshold = 0.30f;

// Used to monitor if accelerometer and gyroscope have been static for a few
// frames.
const int kStaticFrameDetectionThreshold = 50;

// Minimum time step between sensor updates.
const double kMinTimestep = 1; // std::chrono::nanoseconds(1);
} // namespace

namespace cardboard {

// A helper class to keep track of whether some signal can be considered static
// over specified number of frames.
class GyroscopeBiasEstimator::IsStaticCounter {
public:
    // Initializes a counter with the number of consecutive frames we require
    // the signal to be static before IsRecentlyStatic returns true.
    //
    // @param min_static_frames_threshold number of consecutive frames we
    //     require the signal to be static before IsRecentlyStatic returns true.
    explicit IsStaticCounter(int min_static_frames_threshold)
        : min_static_frames_threshold_(min_static_frames_threshold)
        , consecutive_static_frames_(0)
    {
    }

    // Specifies whether the current frame is considered static.
    //
    // @param is_static static flag for current frame.
    void AppendFrame(bool is_static)
    {
        if (is_static) {
            ++consecutive_static_frames_;
        } else {
            consecutive_static_frames_ = 0;
        }
    }

    // Returns if static movement is assumed.
    bool IsRecentlyStatic() const
    {
        return consecutive_static_frames_ >= min_static_frames_threshold_;
    }
    // Resets counter.
    void Reset() { consecutive_static_frames_ = 0; }

private:
    const int min_static_frames_threshold_;
    int consecutive_static_frames_;
};

GyroscopeBiasEstimator::GyroscopeBiasEstimator()
    : accelerometer_lowpass_filter_(kAccelerometerLowPassCutOffFrequencyHz)
    , simulated_gyroscope_from_accelerometer_lowpass_filter_(
          kRotationVelocityBasedAccelerometerLowPassCutOffFrequencyHz)
    , gyroscope_lowpass_filter_(kGyroscopeLowPassCutOffFrequencyHz)
    , gyroscope_bias_lowpass_filter_(kGyroscopeBiasLowPassCutOffFrequencyHz)
    , accelerometer_static_counter_(new IsStaticCounter(kStaticFrameDetectionThreshold))
    , gyroscope_static_counter_(new IsStaticCounter(kStaticFrameDetectionThreshold))
    , current_accumulated_weights_gyroscope_bias_(0.f)
    , mean_filter_(kFilterWindowSize)
    , median_filter_(kFilterWindowSize)
    , last_mean_filtered_accelerometer_value_({ 0, 0, 0 })
{
    Reset();
}

GyroscopeBiasEstimator::~GyroscopeBiasEstimator() { }

void GyroscopeBiasEstimator::Reset()
{
    accelerometer_lowpass_filter_.Reset();
    gyroscope_lowpass_filter_.Reset();
    gyroscope_bias_lowpass_filter_.Reset();
    accelerometer_static_counter_->Reset();
    gyroscope_static_counter_->Reset();
}

void GyroscopeBiasEstimator::ProcessGyroscope(
    const Vector3& gyroscope_sample, uint64_t timestamp_ns)
{
    // Update gyroscope and gyroscope delta low-pass filters.
    gyroscope_lowpass_filter_.AddSample(gyroscope_sample, timestamp_ns);

    const auto smoothed_gyroscope_delta
        = gyroscope_sample - gyroscope_lowpass_filter_.GetFilteredData();

    gyroscope_static_counter_->AppendFrame(
        Length(smoothed_gyroscope_delta) < kGyroscopeDeltaStaticThreshold);

    // Only update the bias if the gyroscope and accelerometer signals have been
    // relatively static recently.
    if (gyroscope_static_counter_->IsRecentlyStatic()
        && accelerometer_static_counter_->IsRecentlyStatic()) {
        // Reset static counter when updating the bias fails.
        if (!UpdateGyroscopeBias(gyroscope_sample, timestamp_ns)) {
            // Bias update fails because of large motion, thus reset the static
            // counter.
            gyroscope_static_counter_->AppendFrame(false);
        }
    } else {
        // Reset weights, if not static.
        current_accumulated_weights_gyroscope_bias_ = 0;
    }
}

void GyroscopeBiasEstimator::ProcessAccelerometer(
    const Vector3& accelerometer_sample, uint64_t timestamp_ns)
{
    // Get current state of the filter.
    const uint64_t previous_accel_timestamp_ns
        = accelerometer_lowpass_filter_.GetMostRecentTimestampNs();
    const bool is_low_pass_filter_init = accelerometer_lowpass_filter_.IsInitialized();

    // Update accel and accel delta low-pass filters.
    accelerometer_lowpass_filter_.AddSample(accelerometer_sample, timestamp_ns);

    const auto smoothed_accelerometer_delta
        = accelerometer_sample - accelerometer_lowpass_filter_.GetFilteredData();

    accelerometer_static_counter_->AppendFrame(
        Length(smoothed_accelerometer_delta) < kAccelerometerDeltaStaticThreshold);

    // Rotation from accel cannot be differentiated with only one sample.
    if (!is_low_pass_filter_init) {
        simulated_gyroscope_from_accelerometer_lowpass_filter_.AddSample({ 0, 0, 0 }, timestamp_ns);
        return;
    }

    // No need to update the simulated gyroscope at this point because the motion
    // is too large.
    if (!accelerometer_static_counter_->IsRecentlyStatic()) {
        return;
    }

    median_filter_.AddSample(accelerometer_lowpass_filter_.GetFilteredData());

    // This processing can only be started if the buffer is fully initialized.
    if (!median_filter_.IsValid()) {
        mean_filter_.AddSample(accelerometer_lowpass_filter_.GetFilteredData());

        // Update the last filtered accelerometer value.
        last_mean_filtered_accelerometer_value_ = accelerometer_lowpass_filter_.GetFilteredData();
        return;
    }

    mean_filter_.AddSample(median_filter_.GetFilteredData());

    // Compute a mock gyroscope value from accelerometer.
    const int64_t diff = timestamp_ns - previous_accel_timestamp_ns;
    const double timestep = static_cast<double>(diff);

    simulated_gyroscope_from_accelerometer_lowpass_filter_.AddSample(
        ComputeAngularVelocityFromLatestAccelerometer(timestep), timestamp_ns);
    last_mean_filtered_accelerometer_value_ = mean_filter_.GetFilteredData();
}

Vector3 GyroscopeBiasEstimator::ComputeAngularVelocityFromLatestAccelerometer(double timestep) const
{
    if (timestep < kMinTimestep) {
        return { 0, 0, 0 };
    }

    const auto mean_of_median = mean_filter_.GetFilteredData();

    // Compute an incremental rotation between the last state and the current
    // state.
    //
    // Note that we switch to double precision here because of precision problem
    // with small rotation.
    const auto incremental_rotation = Rotation::RotateInto(
        Vector3(last_mean_filtered_accelerometer_value_[0],
            last_mean_filtered_accelerometer_value_[1], last_mean_filtered_accelerometer_value_[2]),
        Vector3(mean_of_median[0], mean_of_median[1], mean_of_median[2]));

    // We use axis angle here because this is how gyroscope values are stored.
    Vector3 incremental_rotation_axis;
    double incremental_rotation_angle;
    incremental_rotation.GetAxisAndAngle(&incremental_rotation_axis, &incremental_rotation_angle);

    incremental_rotation_axis *= incremental_rotation_angle / timestep;

    return { static_cast<float>(incremental_rotation_axis[0]),
        static_cast<float>(incremental_rotation_axis[1]),
        static_cast<float>(incremental_rotation_axis[2]) };
}

bool GyroscopeBiasEstimator::UpdateGyroscopeBias(
    const Vector3& gyroscope_sample, uint64_t timestamp_ns)
{
    // Gyroscope values that are too big are potentially dangerous as they could
    // originate from slow and steady head rotations.
    //
    // Therefore we compute an update weight which:
    // * favors gyroscope values that are closer to 0
    // * is set to zero if gyroscope values are greater than a threshold.
    //
    // This way, the gyroscope bias estimation converges faster if the phone is
    // flat on a table, as opposed to held up somewhat stationary in the user's
    // hands.

    // If magnitude is too big, don't update the filter at all so that we don't
    // artificially increase the number of samples accumulated by the filter.
    const float gyroscope_sample_norm2 = Length(gyroscope_sample);
    if (gyroscope_sample_norm2 >= kGyroscopeForBiasThreshold) {
        return false;
    }

    float update_weight
        = std::max(0.0f, 1 - gyroscope_sample_norm2 / kGyroscopeForBiasThreshold);
    update_weight *= update_weight;
    gyroscope_bias_lowpass_filter_.AddWeightedSample(
        gyroscope_lowpass_filter_.GetFilteredData(), timestamp_ns, update_weight);

    // This counter is only partially valid as the low pass filter drops large
    // samples.
    current_accumulated_weights_gyroscope_bias_ += update_weight;

    return true;
}

Vector3 GyroscopeBiasEstimator::GetGyroscopeBias() const
{
    return gyroscope_bias_lowpass_filter_.GetFilteredData();
}

bool GyroscopeBiasEstimator::IsCurrentEstimateValid() const
{
    // Remove any bias component along the gravity because they cannot be
    // evaluated from accelerometer.
    const auto current_gravity_dir = Normalized(last_mean_filtered_accelerometer_value_);
    const auto gyro_bias_lowpass = gyroscope_bias_lowpass_filter_.GetFilteredData();

    const auto off_gravity_gyro_bias
        = gyro_bias_lowpass - current_gravity_dir * Dot(gyro_bias_lowpass, current_gravity_dir);

    // Checks that the current bias estimate is not correlated with the
    // rotation computed from accelerometer.
    const auto gyro_from_accel
        = simulated_gyroscope_from_accelerometer_lowpass_filter_.GetFilteredData();
    const bool isGyroscopeBiasCorrelatedWithSimulatedGyro
        = (Length(gyro_from_accel) * kRatioBetweenGyroBiasAndAccel
            > (Length(off_gravity_gyro_bias) + kEpsilon));
    const bool hasEnoughSamples
        = current_accumulated_weights_gyroscope_bias_ > kMinSumOfWeightsGyroBiasThreshold;
    const bool areCountersStatic = gyroscope_static_counter_->IsRecentlyStatic()
        && accelerometer_static_counter_->IsRecentlyStatic();

    const bool isStatic
        = hasEnoughSamples && areCountersStatic && !isGyroscopeBiasCorrelatedWithSimulatedGyro;
    return isStatic;
}

} // namespace cardboard
