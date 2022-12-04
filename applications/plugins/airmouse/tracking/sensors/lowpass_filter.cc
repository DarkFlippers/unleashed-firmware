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
#include "lowpass_filter.h"

#include <cmath>

namespace {

const double kSecondsFromNanoseconds = 1.0e-9;

// Minimum time step between sensor updates. This corresponds to 1000 Hz.
const double kMinTimestepS = 0.001f;

// Maximum time step between sensor updates. This corresponds to 1 Hz.
const double kMaxTimestepS = 1.00f;

} // namespace

namespace cardboard {

LowpassFilter::LowpassFilter(double cutoff_freq_hz)
    : cutoff_time_constant_(1 / (2 * (double)M_PI * cutoff_freq_hz))
    , initialized_(false)
{
    Reset();
}

void LowpassFilter::AddSample(const Vector3& sample, uint64_t timestamp_ns)
{
    AddWeightedSample(sample, timestamp_ns, 1.0);
}

void LowpassFilter::AddWeightedSample(const Vector3& sample, uint64_t timestamp_ns, double weight)
{
    if (!initialized_) {
        // Initialize filter state
        filtered_data_ = { sample[0], sample[1], sample[2] };
        timestamp_most_recent_update_ns_ = timestamp_ns;
        initialized_ = true;
        return;
    }

    if (timestamp_ns < timestamp_most_recent_update_ns_) {
        timestamp_most_recent_update_ns_ = timestamp_ns;
        return;
    }

    const double delta_s = static_cast<double>(timestamp_ns - timestamp_most_recent_update_ns_)
        * kSecondsFromNanoseconds;
    if (delta_s <= kMinTimestepS || delta_s > kMaxTimestepS) {
        timestamp_most_recent_update_ns_ = timestamp_ns;
        return;
    }

    const double weighted_delta_secs = weight * delta_s;

    const double alpha = weighted_delta_secs / (cutoff_time_constant_ + weighted_delta_secs);

    for (int i = 0; i < 3; ++i) {
        filtered_data_[i] = (1 - alpha) * filtered_data_[i] + alpha * sample[i];
    }
    timestamp_most_recent_update_ns_ = timestamp_ns;
}

void LowpassFilter::Reset()
{
    initialized_ = false;
    filtered_data_ = { 0, 0, 0 };
}

} // namespace cardboard
