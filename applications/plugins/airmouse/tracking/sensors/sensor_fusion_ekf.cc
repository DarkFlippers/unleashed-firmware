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
#include "sensor_fusion_ekf.h"

#include <algorithm>
#include <cmath>

#include "accelerometer_data.h"
#include "gyroscope_data.h"
#include "pose_prediction.h"
#include "../util/matrixutils.h"

namespace cardboard {

namespace {

    const double kFiniteDifferencingEpsilon = 1.0e-7;
    const double kEpsilon = 1.0e-15;
    // Default gyroscope frequency. This corresponds to 100 Hz.
    const double kDefaultGyroscopeTimestep_s = 0.01f;
    // Maximum time between gyroscope before we start limiting the integration.
    const double kMaximumGyroscopeSampleDelay_s = 0.04f;
    // Compute a first-order exponential moving average of changes in accel norm per
    // frame.
    const double kSmoothingFactor = 0.5;
    // Minimum and maximum values used for accelerometer noise covariance matrix.
    // The smaller the sigma value, the more weight is given to the accelerometer
    // signal.
    const double kMinAccelNoiseSigma = 0.75;
    const double kMaxAccelNoiseSigma = 7.0;
    // Initial value for the diagonal elements of the different covariance matrices.
    const double kInitialStateCovarianceValue = 25.0;
    const double kInitialProcessCovarianceValue = 1.0;
    // Maximum accelerometer norm change allowed before capping it covariance to a
    // large value.
    const double kMaxAccelNormChange = 0.15;
    // Timestep IIR filtering coefficient.
    const double kTimestepFilterCoeff = 0.95;
    // Minimum number of sample for timestep filtering.
    const int kTimestepFilterMinSamples = 10;

    // Z direction in start space.
    const Vector3 kCanonicalZDirection(0.0, 0.0, 1.0);

    // Computes an axis-angle rotation from the input vector.
    // angle = norm(a)
    // axis = a.normalized()
    // If norm(a) == 0, it returns an identity rotation.
    static inline Rotation RotationFromVector(const Vector3& a)
    {
        const double norm_a = Length(a);
        if (norm_a < kEpsilon) {
            return Rotation::Identity();
        }
        return Rotation::FromAxisAndAngle(a / norm_a, norm_a);
    }

} // namespace

SensorFusionEkf::SensorFusionEkf()
    : execute_reset_with_next_accelerometer_sample_(false)
    , bias_estimation_enabled_(true)
    , gyroscope_bias_estimate_({ 0, 0, 0 })
{
    ResetState();
}

void SensorFusionEkf::Reset() { execute_reset_with_next_accelerometer_sample_ = true; }

void SensorFusionEkf::ResetState()
{
    current_state_.sensor_from_start_rotation = Rotation::Identity();
    current_state_.sensor_from_start_rotation_velocity = Vector3::Zero();

    current_gyroscope_sensor_timestamp_ns_ = 0;
    current_accelerometer_sensor_timestamp_ns_ = 0;

    state_covariance_ = Matrix3x3::Identity() * kInitialStateCovarianceValue;
    process_covariance_ = Matrix3x3::Identity() * kInitialProcessCovarianceValue;
    accelerometer_measurement_covariance_
        = Matrix3x3::Identity() * kMinAccelNoiseSigma * kMinAccelNoiseSigma;
    innovation_covariance_ = Matrix3x3::Identity();

    accelerometer_measurement_jacobian_ = Matrix3x3::Zero();
    kalman_gain_ = Matrix3x3::Zero();
    innovation_ = Vector3::Zero();
    accelerometer_measurement_ = Vector3::Zero();
    prediction_ = Vector3::Zero();
    control_input_ = Vector3::Zero();
    state_update_ = Vector3::Zero();

    moving_average_accelerometer_norm_change_ = 0.0;

    is_timestep_filter_initialized_ = false;
    is_gyroscope_filter_valid_ = false;
    is_aligned_with_gravity_ = false;

    // Reset biases.
    gyroscope_bias_estimator_.Reset();
    gyroscope_bias_estimate_ = { 0, 0, 0 };
}

// Here I am doing something wrong relative to time stamps. The state timestamps
// always correspond to the gyrostamps because it would require additional
// extrapolation if I wanted to do otherwise.
PoseState SensorFusionEkf::GetLatestPoseState() const { return current_state_; }

void SensorFusionEkf::ProcessGyroscopeSample(const GyroscopeData& sample)
{
    // Don't accept gyroscope sample when waiting for a reset.
    if (execute_reset_with_next_accelerometer_sample_) {
        return;
    }

    // Discard outdated samples.
    if (current_gyroscope_sensor_timestamp_ns_ >= sample.sensor_timestamp_ns) {
        current_gyroscope_sensor_timestamp_ns_ = sample.sensor_timestamp_ns;
        return;
    }

    // Checks that we received at least one gyroscope sample in the past.
    if (current_gyroscope_sensor_timestamp_ns_ != 0) {
        double current_timestep_s = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::nanoseconds(
                sample.sensor_timestamp_ns - current_gyroscope_sensor_timestamp_ns_))
                                        .count();
        if (current_timestep_s > kMaximumGyroscopeSampleDelay_s) {
            if (is_gyroscope_filter_valid_) {
                // Replaces the delta timestamp by the filtered estimates of the delta time.
                current_timestep_s = filtered_gyroscope_timestep_s_;
            } else {
                current_timestep_s = kDefaultGyroscopeTimestep_s;
            }
        } else {
            FilterGyroscopeTimestep(current_timestep_s);
        }

        if (bias_estimation_enabled_) {
            gyroscope_bias_estimator_.ProcessGyroscope(sample.data, sample.sensor_timestamp_ns);

            if (gyroscope_bias_estimator_.IsCurrentEstimateValid()) {
                // As soon as the device is considered to be static, the bias estimator
                // should have a precise estimate of the gyroscope bias.
                gyroscope_bias_estimate_ = gyroscope_bias_estimator_.GetGyroscopeBias();
            }
        }

        // Only integrate after receiving an accelerometer sample.
        if (is_aligned_with_gravity_) {
            const Rotation rotation_from_gyroscope = pose_prediction::GetRotationFromGyroscope(
                { sample.data[0] - gyroscope_bias_estimate_[0],
                    sample.data[1] - gyroscope_bias_estimate_[1],
                    sample.data[2] - gyroscope_bias_estimate_[2] },
                current_timestep_s);
            current_state_.sensor_from_start_rotation
                = rotation_from_gyroscope * current_state_.sensor_from_start_rotation;
            UpdateStateCovariance(RotationMatrixNH(rotation_from_gyroscope));
            state_covariance_ = state_covariance_
                + ((current_timestep_s * current_timestep_s) * process_covariance_);
        }
    }

    // Saves gyroscope event for future prediction.
    current_state_.timestamp = sample.system_timestamp;
    current_gyroscope_sensor_timestamp_ns_ = sample.sensor_timestamp_ns;
    current_state_.sensor_from_start_rotation_velocity.Set(
        sample.data[0] - gyroscope_bias_estimate_[0], sample.data[1] - gyroscope_bias_estimate_[1],
        sample.data[2] - gyroscope_bias_estimate_[2]);
}

Vector3 SensorFusionEkf::ComputeInnovation(const Rotation& pose)
{
    const Vector3 predicted_down_direction = pose * kCanonicalZDirection;

    const Rotation rotation
        = Rotation::RotateInto(predicted_down_direction, accelerometer_measurement_);
    Vector3 axis;
    double angle;
    rotation.GetAxisAndAngle(&axis, &angle);
    return axis * angle;
}

void SensorFusionEkf::ComputeMeasurementJacobian()
{
    for (int dof = 0; dof < 3; dof++) {
        Vector3 delta = Vector3::Zero();
        delta[dof] = kFiniteDifferencingEpsilon;

        const Rotation epsilon_rotation = RotationFromVector(delta);
        const Vector3 delta_rotation
            = ComputeInnovation(epsilon_rotation * current_state_.sensor_from_start_rotation);

        const Vector3 col = (innovation_ - delta_rotation) / kFiniteDifferencingEpsilon;
        accelerometer_measurement_jacobian_(0, dof) = col[0];
        accelerometer_measurement_jacobian_(1, dof) = col[1];
        accelerometer_measurement_jacobian_(2, dof) = col[2];
    }
}

void SensorFusionEkf::ProcessAccelerometerSample(const AccelerometerData& sample)
{
    // Discard outdated samples.
    if (current_accelerometer_sensor_timestamp_ns_ >= sample.sensor_timestamp_ns) {
        current_accelerometer_sensor_timestamp_ns_ = sample.sensor_timestamp_ns;
        return;
    }

    // Call reset state if required.
    if (execute_reset_with_next_accelerometer_sample_.exchange(false)) {
        ResetState();
    }

    accelerometer_measurement_.Set(sample.data[0], sample.data[1], sample.data[2]);
    current_accelerometer_sensor_timestamp_ns_ = sample.sensor_timestamp_ns;

    if (bias_estimation_enabled_) {
        gyroscope_bias_estimator_.ProcessAccelerometer(sample.data, sample.sensor_timestamp_ns);
    }

    if (!is_aligned_with_gravity_) {
        // This is the first accelerometer measurement so it initializes the
        // orientation estimate.
        current_state_.sensor_from_start_rotation
            = Rotation::RotateInto(kCanonicalZDirection, accelerometer_measurement_);
        is_aligned_with_gravity_ = true;

        previous_accelerometer_norm_ = Length(accelerometer_measurement_);
        return;
    }

    UpdateMeasurementCovariance();

    innovation_ = ComputeInnovation(current_state_.sensor_from_start_rotation);
    ComputeMeasurementJacobian();

    // S = H * P * H' + R
    innovation_covariance_ = accelerometer_measurement_jacobian_ * state_covariance_
            * Transpose(accelerometer_measurement_jacobian_)
        + accelerometer_measurement_covariance_;

    // K = P * H' * S^-1
    kalman_gain_ = state_covariance_ * Transpose(accelerometer_measurement_jacobian_)
        * Inverse(innovation_covariance_);

    // x_update = K*nu
    state_update_ = kalman_gain_ * innovation_;

    // P = (I - K * H) * P;
    state_covariance_ = (Matrix3x3::Identity() - kalman_gain_ * accelerometer_measurement_jacobian_)
        * state_covariance_;

    // Updates pose and associate covariance matrix.
    const Rotation rotation_from_state_update = RotationFromVector(state_update_);

    current_state_.sensor_from_start_rotation
        = rotation_from_state_update * current_state_.sensor_from_start_rotation;
    UpdateStateCovariance(RotationMatrixNH(rotation_from_state_update));
}

void SensorFusionEkf::UpdateStateCovariance(const Matrix3x3& motion_update)
{
    state_covariance_ = motion_update * state_covariance_ * Transpose(motion_update);
}

void SensorFusionEkf::FilterGyroscopeTimestep(double gyroscope_timestep_s)
{
    if (!is_timestep_filter_initialized_) {
        // Initializes the filter.
        filtered_gyroscope_timestep_s_ = gyroscope_timestep_s;
        num_gyroscope_timestep_samples_ = 1;
        is_timestep_filter_initialized_ = true;
        return;
    }

    // Computes the IIR filter response.
    filtered_gyroscope_timestep_s_ = kTimestepFilterCoeff * filtered_gyroscope_timestep_s_
        + (1 - kTimestepFilterCoeff) * gyroscope_timestep_s;
    ++num_gyroscope_timestep_samples_;

    if (num_gyroscope_timestep_samples_ > kTimestepFilterMinSamples) {
        is_gyroscope_filter_valid_ = true;
    }
}

void SensorFusionEkf::UpdateMeasurementCovariance()
{
    const double current_accelerometer_norm = Length(accelerometer_measurement_);
    // Norm change between current and previous accel readings.
    const double current_accelerometer_norm_change
        = std::abs(current_accelerometer_norm - previous_accelerometer_norm_);
    previous_accelerometer_norm_ = current_accelerometer_norm;

    moving_average_accelerometer_norm_change_ = kSmoothingFactor * current_accelerometer_norm_change
        + (1 - kSmoothingFactor) * moving_average_accelerometer_norm_change_;

    // If we hit the accel norm change threshold, we use the maximum noise sigma
    // for the accel covariance. For anything below that, we use a linear
    // combination between min and max sigma values.
    const double norm_change_ratio
        = moving_average_accelerometer_norm_change_ / kMaxAccelNormChange;
    const double accelerometer_noise_sigma = std::min(kMaxAccelNoiseSigma,
        kMinAccelNoiseSigma + norm_change_ratio * (kMaxAccelNoiseSigma - kMinAccelNoiseSigma));

    // Updates the accel covariance matrix with the new sigma value.
    accelerometer_measurement_covariance_
        = Matrix3x3::Identity() * accelerometer_noise_sigma * accelerometer_noise_sigma;
}

bool SensorFusionEkf::IsBiasEstimationEnabled() const { return bias_estimation_enabled_; }

void SensorFusionEkf::SetBiasEstimationEnabled(bool enable)
{
    if (bias_estimation_enabled_ != enable) {
        bias_estimation_enabled_ = enable;
        gyroscope_bias_estimate_ = { 0, 0, 0 };
        gyroscope_bias_estimator_.Reset();
    }
}

} // namespace cardboard
