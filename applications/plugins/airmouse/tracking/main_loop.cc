#include "main_loop.h"

#include <furi.h>
#include <furi_hal.h>

#include "imu/imu.h"
#include "orientation_tracker.h"
#include "calibration_data.h"

#define TAG "tracker"

static const float CURSOR_SPEED = 1024.0 / (M_PI / 4);
static const float STABILIZE_BIAS = 16.0;

float g_yaw = 0;
float g_pitch = 0;
float g_dYaw = 0;
float g_dPitch = 0;
bool firstRead = true;
bool stabilize = true;
CalibrationData calibration;
cardboard::OrientationTracker tracker(10000000l); // 10 ms / 100 Hz
uint64_t ippms, ippms2;

static inline float clamp(float val)
{
    while (val <= -M_PI) {
        val += 2 * M_PI;
    }
    while (val >= M_PI) {
        val -= 2 * M_PI;
    }
    return val;
}

static inline float highpass(float oldVal, float newVal)
{
    if (!stabilize) {
        return newVal;
    }
    float delta = clamp(oldVal - newVal);
    float alpha = (float) std::max(0.0, 1 - std::pow(std::fabs(delta) * CURSOR_SPEED / STABILIZE_BIAS, 3.0));
    return newVal + alpha * delta;
}

void sendCurrentState(MouseMoveCallback mouse_move, void *context)
{
    float dX = g_dYaw * CURSOR_SPEED;
    float dY = g_dPitch * CURSOR_SPEED;

    // Scale the shift down to fit the protocol.
    if (dX > 127) {
        dY *= 127.0 / dX;
        dX = 127;
    }
    if (dX < -127) {
        dY *= -127.0 / dX;
        dX = -127;
    }
    if (dY > 127) {
        dX *= 127.0 / dY;
        dY = 127;
    }
    if (dY < -127) {
        dX *= -127.0 / dY;
        dY = -127;
    }

    const int8_t x = (int8_t)std::floor(dX + 0.5);
    const int8_t y = (int8_t)std::floor(dY + 0.5);

    mouse_move(x, y, context);

    // Only subtract the part of the error that was already sent.
    if (x != 0) {
        g_dYaw -= x / CURSOR_SPEED;
    }
    if (y != 0) {
        g_dPitch -= y / CURSOR_SPEED;
    }
}

void onOrientation(cardboard::Vector4& quaternion)
{
    float q1 = quaternion[0]; // X * sin(T/2)
    float q2 = quaternion[1]; // Y * sin(T/2)
    float q3 = quaternion[2]; // Z * sin(T/2)
    float q0 = quaternion[3]; // cos(T/2)

    float yaw = std::atan2(2 * (q0 * q3 - q1 * q2), (1 - 2 * (q1 * q1 + q3 * q3)));
    float pitch = std::asin(2 * (q0 * q1 + q2 * q3));
    // float roll = std::atan2(2 * (q0 * q2 - q1 * q3), (1 - 2 * (q1 * q1 + q2 * q2)));

    if (yaw == NAN || pitch == NAN) {
        // NaN case, skip it
        return;
    }

    if (firstRead) {
        g_yaw = yaw;
        g_pitch = pitch;
        firstRead = false;
    } else {
        const float newYaw = highpass(g_yaw, yaw);
        const float newPitch = highpass(g_pitch, pitch);

        float dYaw = clamp(g_yaw - newYaw);
        float dPitch = g_pitch - newPitch;
        g_yaw = newYaw;
        g_pitch = newPitch;

        // Accumulate the error locally.
        g_dYaw += dYaw;
        g_dPitch += dPitch;
    }
}

extern "C" {

void calibration_begin() {
    calibration.reset();
    FURI_LOG_I(TAG, "Calibrating");
}

bool calibration_step() {
    if (calibration.isComplete())
        return true;

    double vec[6];
    if (imu_read(vec) & GYR_DATA_READY) {
        cardboard::Vector3 data(vec[3], vec[4], vec[5]);
        furi_delay_ms(9); // Artificially limit to ~100Hz
        return calibration.add(data);
    }

    return false;
}

void calibration_end() {
    CalibrationMedian store;
    cardboard::Vector3 median = calibration.getMedian();
    store.x = median[0];
    store.y = median[1];
    store.z = median[2];
    CALIBRATION_DATA_SAVE(&store);
}

void tracking_begin() {
    CalibrationMedian store;
    cardboard::Vector3 median = calibration.getMedian();
    if (CALIBRATION_DATA_LOAD(&store)) {
        median[0] = store.x;
        median[1] = store.y;
        median[2] = store.z;
    }

    ippms = furi_hal_cortex_instructions_per_microsecond();
    ippms2 = ippms / 2;
    tracker.SetCalibration(median);
    tracker.Resume();
}

void tracking_step(MouseMoveCallback mouse_move, void *context) {
    double vec[6];
    int ret = imu_read(vec);
    if (ret != 0) {
        uint64_t t = (DWT->CYCCNT * 1000llu + ippms2) / ippms;
        if (ret & ACC_DATA_READY) {
            cardboard::AccelerometerData adata
                = { .system_timestamp = t, .sensor_timestamp_ns = t,
                    .data = cardboard::Vector3(vec[0], vec[1], vec[2]) };
            tracker.OnAccelerometerData(adata);
        }
        if (ret & GYR_DATA_READY) {
            cardboard::GyroscopeData gdata
                = { .system_timestamp = t, .sensor_timestamp_ns = t,
                    .data = cardboard::Vector3(vec[3], vec[4], vec[5]) };
            cardboard::Vector4 pose = tracker.OnGyroscopeData(gdata);
            onOrientation(pose);
            sendCurrentState(mouse_move, context);
        }
    }
}

void tracking_end() {
    tracker.Pause();
}

}
