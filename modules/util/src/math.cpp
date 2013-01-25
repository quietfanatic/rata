
#include "../inc/math.h"
#include "../inc/honestly.h"

float ang_diff (float a, float b) {
    float d1 = fabs(a - b);
    return d1 > PI ? 2*PI - d1 : d1;
}

