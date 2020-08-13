#pragma once

#include <AzCore/std/string/string.h>
#include <sstream>

namespace StringUtils
{

	inline bool ends_with(AZStd::string const & value, AZStd::string const & ending)
	{
		if (ending.size() > value.size()) return false;
		return AZStd::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}


	static inline bool isspace(char c)
	{
		return c == ' ' || c == '\n' || c == '\t';
	}

	// trim from start (in place)
	static inline void ltrim(AZStd::string &s) {
		s.erase(s.begin(), AZStd::find_if(s.begin(), s.end(), [](int ch) {
			return !isspace(ch);
		}));
	}

	// trim from end (in place)
	static inline void rtrim(AZStd::string &s) {
		s.erase(AZStd::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !isspace(ch);
		}).base(), s.end());
	}

	// trim from both ends (in place)
	static inline void trim(AZStd::string &s) {
		ltrim(s);
		rtrim(s);
	}

	// trim from start (copying)
	static inline AZStd::string ltrim_copy(AZStd::string s) {
		ltrim(s);
		return s;
	}

	// trim from end (copying)
	static inline AZStd::string rtrim_copy(AZStd::string s) {
		rtrim(s);
		return s;
	}


	static inline void RemoveLineComments(AZStd::string &str)
	{
		while (true)
		{
			size_t nFPos = str.find("//");
			if (nFPos + 1)
			{
				size_t eol = str.find('\n', nFPos);
				if (eol + 1) {
					str.erase(nFPos, eol - nFPos);
				}
				else
				{
					str.erase(nFPos);
					return;
				}

			}
			else
			{
				break;
			}
		}
	}

	static inline AZStd::string GetKvValue(AZStd::string str, int level)
	{
		std::string value;
		std::istringstream  stream(str.c_str());
		level = std::max(1, level);//at least one iteration
		while (stream && level--) {
			stream >> value;
		}

		return AZStd::string(value.c_str());
	}

	static inline AZStd::vector<AZStd::string> SplitString(AZStd::string s, AZStd::string delimiter)
	{
		AZStd::vector<AZStd::string> vec;

		size_t last = 0; size_t next = 0; 
		while ((next = s.find(delimiter, last)) != string::npos) 
		{ 
			vec.push_back(s.substr(last, next - last));
			last = next + 1; 
		} 
		vec.push_back(s.substr(last));
		return vec;
	}

}
