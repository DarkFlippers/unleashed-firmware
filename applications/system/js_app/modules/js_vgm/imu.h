#pragma once

typedef struct Imu Imu;

Imu* imu_alloc(void);

void imu_free(Imu* imu);

bool imu_present(Imu* imu);

float imu_pitch_get(Imu* imu);

float imu_roll_get(Imu* imu);

float imu_yaw_get(Imu* imu);
