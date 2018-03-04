
#include <string>
#include <vector>

#ifndef __PARAM_UTILITIES__
#define __PARAM_UTILITIES__

namespace param_utilities
{
	void init(int argc, char** argv);
	bool isParam(const std::string& paramName);
	bool getParamValue(const std::string& paramName, /*out*/std::string& paramValue);
	bool getParamValue(const std::string& paramName, /*out*/std::vector<std::string>& paramValue);
	bool getParamValue(const std::string& paramName, /*out*/int& paramValue);
	const std::vector<std::string>& getFreeParameters();
}

#endif // !__PARAM_UTILITIES__