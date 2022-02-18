#include "util_geo_operation.h"

void utility_function::convert_mecator_to_WGS84(double& x, double& y)
{
    x = MECATOR_CORRECT * x / (PI / 180) / EARTH_RADIUS;
    y = (2 * atan(exp(MECATOR_CORRECT * y / EARTH_RADIUS)) - PI / 2) / (PI / 180);
};

void utility_function::convert_matrix_to_mecator(double& x, double& y, double x_refNet, double y_refNet, double x_refMap, double y_refMap)
{
    double reference_latitude = (2 * atan(exp(y_refMap * MECATOR_CORRECT / EARTH_RADIUS)) - PI / 2) / (PI / 180);
    double local_scale = 1 / cos(reference_latitude / (180 / PI));
    x = (x - x_refNet) * local_scale + x_refMap;
    y = (y - y_refNet) * local_scale + y_refMap;
};