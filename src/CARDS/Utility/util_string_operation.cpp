#include "util_string_operation.h"

string utility_function::left(string s, int pos)
{
	s = s.substr(0, pos);
	return s;
}
string utility_function::right(string s, int pos)
{
	s = s.substr(pos, (int)s.size());
	return s;
};

// string utility_function::remove_string_after_last_char(string original_string, char split, bool include)
// {
// 	const char *ptr = strrchr(original_string.c_str(), split);
// 	long long last = long long(&original_string.back() - ptr) / sizeof(char);
// 	string proto_path = include ? original_string.substr(0, original_string.size() - last) : original_string.substr(0, original_string.size() - last - 1);
// 	return proto_path;
// }