#include "lightmeter_helper.h"
#include "lightmeter_config.h"

extern const float aperture_numbers[];
extern const float speed_numbers[];

float lux2ev(float lux) {
    return log2(lux / 2.5);
}

float getMinDistance(float x, float v1, float v2) {
    if(x - v1 > v2 - x) {
        return v2;
    }

    return v1;
}

float normalizeAperture(float a) {
    for(int i = 0; i < AP_NUM; i++) {
        float a1 = aperture_numbers[i];
        float a2 = aperture_numbers[i + 1];

        if(a1 < a && a2 >= a) {
            return getMinDistance(a, a1, a2);
        }
    }

    return 0;
}

float normalizeTime(float a) {
    for(int i = 0; i < SPEED_NUM; i++) {
        float a1 = speed_numbers[i];
        float a2 = speed_numbers[i + 1];

        if(a1 < a && a2 >= a) {
            return getMinDistance(a, a1, a2);
        }
    }

    return 0;
}
