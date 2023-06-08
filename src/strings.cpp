#include "strings.h"

std::string int32ToStr(const int32_t& val)
{
	std::stringstream s;
	s << val;
	return s.str();
}

std::string int64ToStr(const int64_t& val)
{
	std::stringstream s;
	s << val;
	return s.str();
}

std::string doubleToStr(const double& val)
{
	std::stringstream s;
	s << val;
	return s.str();
}
