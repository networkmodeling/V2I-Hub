#include "cfg_reader.h"
#include <fstream>

bool CfgReader::initialize( const char *path , bool is_a_file)
{
	std::string json_string = "";
	if (is_a_file)
	{ 
		std::ifstream ifs(path);

		if (!ifs.is_open()) {
			// Need an error handler here, e.g., output to a log file. Right now we simply return false
			printf( "Failed to open file: %s ", path);
			return false;
		}
		json_string = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	}
	else
	{
		json_string = path;
	}
	Json::Reader reader;
	if (!reader.parse( json_string, root))
	{
		// Need a better error handler here, e.g., output to a log file
		printf( "Failed to parse file: %s ", path);
		return false;
	}

   return true;
}

Json::Value CfgReader::getParameter( const char *paramName )
{
	Json::Value nullParameter(Json::nullValue);
	Json::Value value = root.get(paramName, nullParameter);

	if (value.isNull())
	{
		// Need a better error handler here, e.g., output to a log file
		printf( "Parameter %s not specified.\n", paramName);
	}

	return value;
}


int CfgReader::getParameter( const char *paramName, int *paramValue )
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isNumeric())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	
	*paramValue = value.asInt();

	return 0;
}

int CfgReader::getParameter( const char *paramName, unsigned long *paramValue )
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isNumeric())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	
	*paramValue = value.asInt();
	
	return 0;
}

int CfgReader::getParameter( const char *paramName, float *paramValue )
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isNumeric())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	*paramValue = value.asDouble();
	
	return 0;
}

int CfgReader::getParameter( const char *paramName, double *paramValue )
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isNumeric())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	*paramValue = value.asDouble();
	
	return 0;
}

int CfgReader::getParameter( const char *paramName, std::string *paramValue )
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isString())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	
	paramValue->assign(value.asString());
	
	return 0;
}

int CfgReader::getParameter( const char *paramName, bool *paramValue )
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isBool())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	
	*paramValue = value.asBool();
	
	return 0;
}

int CfgReader::getParameter( const char *paramName, IntArray *paramList)
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isArray())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	
	for (unsigned int index = 0; index < value.size(); index++)
	{
		if (!(value[index].isNumeric()))
		{
			return PARAMETER_TYPE_NOT_MATCH;
		}
		paramList->push_back(value[index].asInt());
	}

	return 0;
}	

int CfgReader::getParameter( const char *paramName, DoubleArray *paramList)
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isArray())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	
	for (unsigned int index = 0; index < value.size(); index++)
	{
		if (!(value[index].isNumeric()))
		{
			return PARAMETER_TYPE_NOT_MATCH;
		}
		paramList->push_back(value[index].asDouble());
	}

	return 0;
}	

int CfgReader::getParameter( const char *paramName, StringArray *paramList)
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isArray())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	
	for (unsigned int index = 0; index < value.size(); index++)
	{
		if (!(value[index].isString()))
		{
			return PARAMETER_TYPE_NOT_MATCH;
		}
		paramList->push_back(value[index].asString());
	}

	return 0;
}

int CfgReader::getParameter( const char *paramName, BoolArray *paramList)
{
	Json::Value value = getParameter(paramName);
	
	if (value.isNull())
	{
		return PARAMETER_NOT_SPECIFIED;
	}
	
	if (!value.isArray())
	{
		return PARAMETER_TYPE_NOT_MATCH;
	}
	
	for (unsigned int index = 0; index < value.size(); index++)
	{
		if (!(value[index].isBool()))
		{
			return PARAMETER_TYPE_NOT_MATCH;
		}
		paramList->push_back(value[index].asBool());
	}

	return 0;
}