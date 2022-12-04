#include <furi.h>
#include <furi_hal.h>

#define TAG "tracker"

#include "calibration_data.h"

#include <cmath>
#include <algorithm>

// Student's distribution T value for 95% (two-sided) confidence interval.
static const double Tn = 1.960;

// Number of samples (degrees of freedom) for the corresponding T values.
static const int Nn = 200;

void CalibrationData::reset()
{
    complete = false;
    count = 0;
    sum = Vector::Zero();
    sumSq = Vector::Zero();
    mean = Vector::Zero();
    median = Vector::Zero();
    sigma = Vector::Zero();
    delta = Vector::Zero();
    xData.clear();
    yData.clear();
    zData.clear();
}

bool CalibrationData::add(Vector& data)
{
    if (complete) {
        return true;
    }

    xData.push_back(data[0]);
    yData.push_back(data[1]);
    zData.push_back(data[2]);

    sum += data;
    sumSq += data * data;
    count++;

    if (count >= Nn) {
        calcDelta();
        complete = true;
    }

    return complete;
}

static inline double medianOf(std::vector<double>& list)
{
    std::sort(list.begin(), list.end());
    int count = list.size();
    int middle = count / 2;
    return (count % 2 == 1) ? list[middle] : (list[middle - 1] + list[middle]) / 2.0l;
}

void CalibrationData::calcDelta()
{
    median.Set(medianOf(xData), medianOf(yData), medianOf(zData));

    mean = sum / count;
    Vector m2 = mean * mean;
    Vector d = sumSq / count - m2;
    Vector s2 = (d * count) / (count - 1);
    sigma = Vector(std::sqrt(d[0]), std::sqrt(d[1]), std::sqrt(d[2]));
    Vector s = Vector(std::sqrt(s2[0]), std::sqrt(s2[1]), std::sqrt(s2[2]));
    delta = s * Tn / std::sqrt((double)count);
    Vector low = mean - delta;
    Vector high = mean + delta;

    FURI_LOG_I(TAG,
        "M[x] = { %f ... %f }  //  median = %f  //  avg = %f  //  delta = %f  //  sigma = %f",
        low[0], high[0], median[0], mean[0], delta[0], sigma[0]);
    FURI_LOG_I(TAG,
        "M[y] = { %f ... %f }  //  median = %f  //  avg = %f  //  delta = %f  //  sigma = %f",
        low[1], high[1], median[1], mean[1], delta[1], sigma[1]);
    FURI_LOG_I(TAG,
        "M[z] = { %f ... %f }  //  median = %f  //  avg = %f  //  delta = %f  //  sigma = %f",
        low[2], high[2], median[2], mean[2], delta[2], sigma[2]);
}