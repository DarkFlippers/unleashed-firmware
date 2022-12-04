#pragma once

#include <toolbox/saved_struct.h>
#include <storage/storage.h>
#include <vector>

#include "util/vector.h"

#define CALIBRATION_DATA_VER (1)
#define CALIBRATION_DATA_FILE_NAME ".calibration.data"
#define CALIBRATION_DATA_PATH INT_PATH(CALIBRATION_DATA_FILE_NAME)
#define CALIBRATION_DATA_MAGIC (0x23)

#define CALIBRATION_DATA_SAVE(x)   \
    saved_struct_save(             \
        CALIBRATION_DATA_PATH,     \
        (x),                       \
        sizeof(CalibrationMedian), \
        CALIBRATION_DATA_MAGIC,    \
        CALIBRATION_DATA_VER)

#define CALIBRATION_DATA_LOAD(x)   \
    saved_struct_load(             \
        CALIBRATION_DATA_PATH,     \
        (x),                       \
        sizeof(CalibrationMedian), \
        CALIBRATION_DATA_MAGIC,    \
        CALIBRATION_DATA_VER)

typedef struct {
    double x;
    double y;
    double z;
} CalibrationMedian;

typedef cardboard::Vector3 Vector;

/**
 * Helper class to gather some stats and store the calibration data. Right now it calculates a lot
 * more stats than actually needed. Some of them are used for logging the sensors quality (and
 * filing bugs), other may be required in the future, e.g. for bias.
 */
class CalibrationData {
public:
    /**
     * Check if the sensors were calibrated before.
     *
     * @return {@code true} if calibration data is available, or {@code false} otherwise.
     */
    bool isComplete() {
        return complete;
    }

    /** Prepare to collect new calibration data. */
    void reset();

    /**
     * Retrieve the median gyroscope readings.
     *
     * @return Three-axis median vector.
     */
    Vector getMedian() {
        return median;
    }

    /**
     * Retrieve the mean gyroscope readings.
     *
     * @return Three-axis mean vector.
     */
    Vector getMean() {
        return mean;
    }

    /**
     * Retrieve the standard deviation of gyroscope readings.
     *
     * @return Three-axis standard deviation vector.
     */
    Vector getSigma() {
        return sigma;
    }

    /**
     * Retrieve the confidence interval size of gyroscope readings.
     *
     * @return Three-axis confidence interval size vector.
     */
    Vector getDelta() {
        return delta;
    }

    /**
     * Add a new gyroscope reading to the stats.
     *
     * @param data gyroscope values vector.
     * @return {@code true} if we now have enough data for calibration, or {@code false} otherwise.
     */
    bool add(Vector& data);

private:
    // Calculates the confidence interval (mean +- delta) and some other related values, like
    // standard deviation, etc. See https://en.wikipedia.org/wiki/Student%27s_t-distribution
    void calcDelta();

    int count;
    bool complete;
    Vector sum;
    Vector sumSq;
    Vector mean;
    Vector median;
    Vector sigma;
    Vector delta;
    std::vector<double> xData;
    std::vector<double> yData;
    std::vector<double> zData;
};
