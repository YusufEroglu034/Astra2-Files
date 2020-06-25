#pragma once
#include <string>
#include <algorithm>

inline bool str_to_number (bool& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (strtol(in, nullptr, 10) != 0);
	return true;
}

inline bool str_to_bool(bool& out, const std::string &in)
{
	out = in.at(0) == '1';
	return true;
}

inline bool str_to_number (char& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (char) strtol(in, nullptr, 10);
	return true;
}

inline bool str_to_number (uint8_t& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (uint8_t) strtoul(in, nullptr, 10);
	return true;
}

inline bool str_to_number (int16_t& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (int16_t) strtol(in, nullptr, 10);
	return true;
}

inline bool str_to_number (uint16_t& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (uint16_t) strtoul(in, nullptr, 10);
	return true;
}

inline bool str_to_number (int32_t& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (int32_t) strtol(in, nullptr, 10);
	return true;
}

inline bool str_to_number (uint32_t& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (uint32_t) strtoul(in, nullptr, 10);
	return true;
}

inline bool str_to_number (int64_t& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (int64_t) strtoull(in, nullptr, 10);
	return true;
}

inline bool str_to_number (uint64_t& out, const char *in)
{
	if (0==in || 0==in[0])  return false;

    out = (uint64_t) strtoull(in, nullptr, 10);
    return true;
}

inline bool str_to_number (float& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (float) strtof(in, nullptr);
	return true;
}

inline bool str_to_number (double& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (double) strtod(in, nullptr);
	return true;
}

inline bool str_to_float(float& out, const char *in)
{
	char* end;
	double ret = (double)strtod(in, &end);
	bool parsed = (*end == '\0');

	out = parsed ? (float)ret : 0.0f;
	return parsed;
}

inline bool str_to_double (double& out, const char *in)
{
	char* end;
	double ret = (double) strtod(in, &end);
	bool parsed = (*end == '\0');

	out = parsed ? ret : 0;
	return parsed;
}

inline bool str_to_number (long double& out, const char *in)
{
	if (0==in || 0==in[0])	return false;

	out = (long double) strtold(in, nullptr);
	return true;
}

//@source: http://stackoverflow.com/a/2845275/2205532
inline bool is_positive_number(const std::string & str)
{
	if (str.empty() || ((!isdigit(str[0])) && (str[0] != '-') && (str[0] != '+'))) return false;

	char * p;
	strtol(str.c_str(), &p, 10);

	//Finally, check that the referenced pointer points to the end of the string. If that happens, said string is a number.
	return (*p == 0);
}


//Deletes the second map entry and clears the map when its done.
template <typename M> void FreeClear(M & mM)
{
	for (typename M::iterator it = mM.begin(); it != mM.end(); ++it) {
		delete it->second;
	}
	mM.clear();
}

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

template <class container>
void wipe(container & a) {
	typename container::iterator first, past;

	first = a.begin();
	past = a.end();

	while (first != past)
		delete *(first++);

	a.clear();
}

inline void str_to_lower_ascii(std::string& rstRet)
{
	std::transform(rstRet.begin(), rstRet.end(), rstRet.begin(), ::tolower);
}

inline void str_to_upper_ascii(std::string& rstRet)
{
	std::transform(rstRet.begin(), rstRet.end(), rstRet.begin(), ::toupper);
}


#if !defined(__FreeBSD__) && !defined(_WIN32)
#include <stdio.h>
#include <sys/types.h>

size_t strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
			break;
		} while (--n != 0);
	}

	if (n == 0) {
		if (siz != 0)
			*d = '\0';
		while (*s++);
	}

	return(s - src - 1);
}
#endif


