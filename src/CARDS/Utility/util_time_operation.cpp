#include "util_time_operation.h"

int utility_function::convert_hhmmss_to_seconds(string hhmmss)
{
	string hh, mm, ss;
	//int pos = (int) hhmmss.find(":");
	int pos = 2;
	hh = left(hhmmss, pos);
	hhmmss = right(hhmmss, pos + 1);

	//pos = (int) hhmmss.find(":");
	pos = 2;
	mm = left(hhmmss, pos);
	ss = right(hhmmss, pos + 1);
	int h = stoi(hh);
	int m = stoi(mm);
	int s = stoi(ss);

	int time_in_seconds = h * 3600 + m * 60 + s;

	return time_in_seconds;
}
int utility_function::convert_hhmm_to_seconds(string hhmm)
{
	string hh, mm;

	int pos = 2;
	hh = left(hhmm, pos);
	mm = right(hhmm, pos + 1);

	int h = stoi(hh);
	int m = stoi(mm);

	int time_in_seconds = h * 3600 + m * 60;

	return time_in_seconds;
}
string utility_function::convert_seconds_to_hhmmss(int time_in_seconds)
{
	{
		long long h = time_in_seconds / 3600;
		time_in_seconds = time_in_seconds % 3600;
		long long m = time_in_seconds / 60;
		time_in_seconds = time_in_seconds % 60;
		long long s = time_in_seconds;
		string hh = to_string(h);
		if (hh.size() == 1)
		{
			hh = "0" + hh;
		}
		string mm = to_string(m);
		if (mm.size() == 1)
		{
			mm = "0" + mm;
		}

		string ss = to_string(s);
		if (ss.size() == 1)
		{
			ss = "0" + ss;
		}

		string hhmmss = hh + ":" + mm + ":" + ss;
		return hhmmss;
	};
}
string utility_function::convert_seconds_to_hhmm(int time_in_seconds)
{
	long long h = time_in_seconds / 3600;
	time_in_seconds = time_in_seconds % 3600;
	long long m = time_in_seconds / 60;
	time_in_seconds = time_in_seconds % 60;
	long long s = time_in_seconds;
	string hh = to_string(h);
	if (hh.size() == 1)
	{
		hh = "0" + hh;
	}
	string mm = to_string(m);
	if (mm.size() == 1)
	{
		mm = "0" + mm;
	}

	string ss = to_string(s);
	if (ss.size() == 1)
	{
		ss = "0" + ss;
	}

	string hhmm = hh + ":" + mm;
	return hhmm;
}

string utility_function::convert_seconds_to_hhmmss(float time_in_seconds)
{
	float time_in_seconds_copy = time_in_seconds;
	long long h = time_in_seconds / 3600;
	time_in_seconds = (int)time_in_seconds % 3600;
	time_in_seconds_copy -= h * 3600;
	long long m = time_in_seconds / 60;
	time_in_seconds = (int)time_in_seconds % 60;
	time_in_seconds_copy -= m * 60;
	long long s = time_in_seconds;
	time_in_seconds_copy -= s;
	string hh = to_string(h);
	if (hh.size() == 1)
	{
		hh = "0" + hh;
	}
	string mm = to_string(m);
	if (mm.size() == 1)
	{
		mm = "0" + mm;
}

	string ss = to_string(s);
	if (ss.size() == 1)
	{
		ss = "0" + ss;
	}

	string hhmmss = hh + ":" + mm + ":" + ss +"." + to_string(int(time_in_seconds_copy*10));
	return hhmmss;
}

#ifdef LINUX
double utility_function::get_current_cpu_time_in_seconds()
{
	struct timespec current_cpu_time;
	double current_cpu_time_in_seconds;
	clock_gettime(CLOCK_REALTIME, &current_cpu_time);

	current_cpu_time_in_seconds = (double)((current_cpu_time.tv_sec * 1000000000 + current_cpu_time.tv_nsec)) / 1000000000.0;
	return current_cpu_time_in_seconds;
};
#else
double utility_function::get_current_cpu_time_in_seconds()
{
	LARGE_INTEGER current_cpu_time;
	LARGE_INTEGER query_performance_frequency;
	double current_cpu_time_in_seconds;

	QueryPerformanceFrequency(&query_performance_frequency);
	QueryPerformanceCounter(&current_cpu_time);

	current_cpu_time_in_seconds = (double)(current_cpu_time.QuadPart / ((double)query_performance_frequency.QuadPart));

	return current_cpu_time_in_seconds;
};
#endif

long long utility_function::get_current_cpu_time_in_nanoseconds()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

double utility_function::get_current_cpu_time_in_milliseconds()
{
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	double ms = (double)millis;
	return ms;
}

string utility_function::get_current_time_string(string format)
{
//http://www.cplusplus.com/reference/ctime/strftime/
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 80, format.c_str(), timeinfo);
		return string(buffer);
};

string utility_function::get_current_time_string_with_ms_precision(string format)
{
//https://paul.pub/cpp-date-time/ 
	timespec ts;
	timespec_get(&ts, TIME_UTC);
	char buff[100];
	strftime(buff, sizeof buff, format.c_str(), std::gmtime(&ts.tv_sec));
	ostringstream stream;
	stream << buff << ".";
	stream.precision(3);
	stream << to_string(ts.tv_nsec).substr(0,3);
	return stream.str();
}