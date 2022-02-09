#pragma once

#include "json/json.h"

#define PARAMETER_FOUND 0
#define PARAMETER_NOT_SPECIFIED 1
#define PARAMETER_TYPE_NOT_MATCH 2

typedef std::vector<int> IntArray;
typedef std::vector<double> DoubleArray;
typedef std::vector<std::string> StringArray;
typedef std::vector<bool> BoolArray;

class CfgReader
{
	Json::Value root;
	bool parseValueTree( const std::string &input );
	Json::Value getParameter( const char *parameterName );
public:
	bool initialize( const char *path, bool is_a_file=true);
	int getParameter( const char *parameterName, int *paramValue );
	int getParameter( const char *parameterName, float *paramValue );
	int getParameter( const char *parameterName, double *paramValue );
	int getParameter( const char *parameterName, std::string *paramValue );
	int getParameter( const char *parameterName, bool *paramValue );
	int getParameter( const char *parameterName, IntArray *parameter);
	int getParameter( const char *parameterName, DoubleArray *parameter);
	int getParameter( const char *parameterName, StringArray *parameter);
	int getParameter( const char *parameterName, BoolArray *parameter);
	int getParameter( const char *parameterName, unsigned long *paramValue);
};