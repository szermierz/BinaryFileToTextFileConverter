
#include "param_utilities.h"

#include <map>

using namespace std;

namespace param_utilities
{
	namespace
	{
		map<string, vector<string> > parameters;
		vector<string> freeParameters;
	}

	void init(int argc, char** argv)
	{
		vector<string>* currentParams = &freeParameters;
		for(int i = 0; i < argc; ++i)
		{
			if(argv[i][0] == '-')
			{
				currentParams = &parameters[string(argv[i]).substr(1)];
				continue;
			}

			currentParams->push_back(argv[i]);
		}
	}

	bool isParam(const string& paramName)
	{
		return parameters.find(paramName) != parameters.end();
	}

	bool getParamValue(const string& paramName, /*out*/string& paramValue)
	{
		auto paramIt = parameters.find(paramName);
		if(paramIt == parameters.end())
			return false;

		if(!paramIt->second.empty())
			paramValue = paramIt->second.front();
		else
			return false;

		return true;
	}

	bool getParamValue(const string& paramName, /*out*/vector<string>& paramValue)
	{
		auto paramIt = parameters.find(paramName);
		if(paramIt == parameters.end())
			return false;

		paramValue = paramIt->second;

		return true;
	}

	bool getParamValue(const string& paramName, /*out*/int& paramValue)
	{
		string dummy;
		if(!getParamValue(paramName, dummy))
			return false;

		paramValue = std::stoi(dummy);
		return true;
	}

	const std::vector<std::string>& getFreeParameters()
	{
		return freeParameters;
	}
}