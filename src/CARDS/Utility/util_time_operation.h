#pragma once
#include "util_include_all.h"
#include "util_string_operation.h"
#include <chrono>
#include <sstream>
#ifdef LINUX
    #include <sys/time.h>
	#include <tr1/regex>
	#define regex std::tr1::regex
	#include <tr1/unordered_map>
	#define unordered_map std::tr1::unordered_map
	#include <tr1/unordered_set>
	#define unordered_set std::tr1::unordered_set
#else
    #include <Windows.h>
	#include <regex>
#endif
namespace utility_function
{ 
	int convert_hhmmss_to_seconds(string hhmmss);
	int convert_hhmm_to_seconds(string hhmm);
	string convert_seconds_to_hhmmss(int time_in_seconds);
	string convert_seconds_to_hhmm(int time_in_seconds);
	string convert_seconds_to_hhmmss(float time_in_seconds);
	double get_current_cpu_time_in_seconds();
    
	long long get_current_cpu_time_in_nanoseconds();
	double get_current_cpu_time_in_milliseconds();

	string get_current_time_string(string format);
	string get_current_time_string_with_ms_precision(string format);
}