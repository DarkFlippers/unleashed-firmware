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
#include "mean_filter.h"

namespace cardboard {

MeanFilter::MeanFilter(size_t filter_size)
    : filter_size_(filter_size)
{
}

void MeanFilter::AddSample(const Vector3& sample)
{
    buffer_.push_back(sample);
    if (buffer_.size() > filter_size_) {
        buffer_.pop_front();
    }
}

bool MeanFilter::IsValid() const { return buffer_.size() == filter_size_; }

Vector3 MeanFilter::GetFilteredData() const
{
    // Compute mean of the samples stored in buffer_.
    Vector3 mean = Vector3::Zero();
    for (auto sample : buffer_) {
        mean += sample;
    }

    return mean / static_cast<double>(filter_size_);
}

} // namespace cardboard
