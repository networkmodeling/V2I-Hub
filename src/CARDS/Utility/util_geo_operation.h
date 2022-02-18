#ifndef UTIL_GEO_OPERATION_H
#define UTIL_GEO_OPERATION_H
// #pragma once
#include "util_include_all.h"
// #include "util_point_operation.h"
// #include "util_unit_operation.h"
#include <math.h>

namespace utility_function
{
#define PI 3.14159265359
#define EARTH_RADIUS 6378137
#define MECATOR_CORRECT 1.001120232

    void convert_mecator_to_WGS84(double& x, double& y);
    void convert_matrix_to_mecator(double& x, double& y, double x_refNet, double y_refNet, double x_refMap, double y_refMap);

    // void convert_mecator_to_WGS84(double& x, double& y)
	// {
	// 	x = MECATOR_CORRECT * x / (PI / 180) / EARTH_RADIUS;
	// 	y = (2 * atan(exp(MECATOR_CORRECT * y / EARTH_RADIUS)) - PI / 2) / (PI / 180);
	// };

	// void convert_matrix_to_mecator(double& x, double& y, double x_refNet, double y_refNet, double x_refMap, double y_refMap)
	// {
	// 	double reference_latitude = (2 * atan(exp(y_refMap * MECATOR_CORRECT / EARTH_RADIUS)) - PI / 2) / (PI / 180);
	// 	double local_scale = 1 / cos(reference_latitude / (180 / PI));
	// 	x = (x - x_refNet) * local_scale + x_refMap;
	// 	y = (y - y_refNet) * local_scale + y_refMap;
	// };

}	
#endif