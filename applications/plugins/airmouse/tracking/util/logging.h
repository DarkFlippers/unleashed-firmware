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
#ifndef CARDBOARD_SDK_UTIL_LOGGING_H_
#define CARDBOARD_SDK_UTIL_LOGGING_H_

#include <furi.h>
#include <furi_hal.h>

#if defined(__ANDROID__)

#include <android/log.h>

// Uncomment these to enable debug logging from native code

#define CARDBOARD_LOGI(...) // __android_log_print(ANDROID_LOG_INFO, "CardboardSDK", __VA_ARGS__)
#define CARDBOARD_LOGE(...) // __android_log_print(ANDROID_LOG_ERROR, "CardboardSDK", __VA_ARGS__)

#else

#define CARDBOARD_LOGI(...) // FURI_LOG_I("CardboardSDK", __VA_ARGS__)
#define CARDBOARD_LOGE(...) // FURI_LOG_E("CardboardSDK", __VA_ARGS__)

#endif

#endif // CARDBOARD_SDK_UTIL_LOGGING_H_
