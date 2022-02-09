#pragma once
#include "util_include_all.h"

namespace utility_function
{
	enum empties_t { empties_ok, no_empties };
	template <typename Container>
	Container& string_split(
		Container&                            result,
		const typename Container::value_type& s,
		const typename Container::value_type& delimiters,
		empties_t                      empties = empties_ok)
	{
		result.clear();
		size_t current;
		size_t next = -1;
		do
		{
			if (empties == no_empties)
			{
				next = s.find_first_not_of(delimiters, next + 1);
				if (next == Container::value_type::npos) break;
				next -= 1;
			}
			current = next + 1;
			next = s.find_first_of(delimiters, current);
			result.push_back(s.substr(current, next - current));
		} while (next != Container::value_type::npos);
		return result;
	};

	//left substring
	string left(string s, int pos);
	
	//right substring
	string right(string s, int pos);

	// string remove_string_after_last_char(string original_string, char split, bool include = true);
}