
//#define __DEBUG__

#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <stdio.h>

#include "param_utilities.h"

using namespace std;
namespace filesystem = std::experimental::filesystem;

string to_hex(const char& byte)
{
	static char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	
	string result = "0x";

	result += hex_chars[(byte & 0xF0) >> 4];
	result += hex_chars[(byte & 0x0F) >> 0];
	
	return result;
}

void buildPneumonics(unordered_map<char, string>& pneumonics)
{
	auto simplePneumonic = [](const char& c) -> string
	{
		return to_hex(c) + " ('" + c + "')";
	};

	for(char i = 'a'; i <= 'z'; ++i)
		pneumonics[i] = simplePneumonic(i);
	for(char i = 'A'; i <= 'Z'; ++i)
		pneumonics[i] = simplePneumonic(i);
	for(char i = '0'; i <= '9'; ++i)
		pneumonics[i] = simplePneumonic(i);

	pneumonics[0x00] = "0x00 (NULL)";
	pneumonics[0x0D]   = "0x0D (CR)";
	pneumonics[0x0A]   = "0x0A (LF)";

	for(char i = numeric_limits<char>::min(); i < numeric_limits<char>::max(); ++i)
	{
		if(pneumonics.count(i) > 0)
			continue;

		pneumonics.insert(make_pair(i, to_hex(i)));
	}
}

void printHelp()
{
	std::cout << "*** Textify ***" << endl;
	std::cout << endl;
	std::cout << "Commandline arguments:" << endl;
	std::cout << "-fileMode (textify a single file)" << endl;
	std::cout << "-dirMode (textify a whole directory)" << endl;
	std::cout << "-inputPath (path to input file/directory)" << endl;
	std::cout << "-outputPath (path to output file/directory)" << endl;
}

const string wrongDash = "\\";
const string correctDash = "/";
const string noPneumonic = "Unknown";
const int charactersPerLine = 10;

vector<char> OpenFile(const string& path)
{
	FILE *fileptr;
	vector<char> buffer;
	long filelen;

	fileptr = fopen(path.c_str(), "rb");  // Open the file in binary mode
	fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	filelen = ftell(fileptr);             // Get the current byte offset in the file
	rewind(fileptr);                      // Jump back to the beginning of the file

	buffer.resize(filelen + 1);           // Enough memory for file + \0
	fread(&buffer.front(), filelen, 1, fileptr); // Read in the entire file
	fclose(fileptr); // Close the file

	return buffer;
}

void TextifyFile(const string& inputPath, const string& outputPath, const unordered_map<char, string>& pneumonics)
{
	auto makePath = [](const string& fullPath) -> string
	{
		auto correctDashPathEnd = fullPath.find_last_of(correctDash.front());
		auto wrongDashPathEnd   = fullPath.find_last_of(wrongDash.front());

		string::size_type pathEnd;
		if(string::npos == correctDashPathEnd && string::npos == wrongDashPathEnd)
			return fullPath;
		else if(string::npos == correctDashPathEnd)
			pathEnd = wrongDashPathEnd;
		else if(string::npos == wrongDashPathEnd)
			pathEnd = correctDashPathEnd;
		else
			pathEnd = max(wrongDashPathEnd, correctDashPathEnd);

		return fullPath.substr(0, pathEnd);
	};

	filesystem::create_directories(makePath(outputPath));

	ifstream inputFile(inputPath);
	ofstream outputFile(outputPath);
	
	if(!inputFile.good())
	{
		std::cout << "Cant open " + inputPath << endl;
		return;
	}
	inputFile.close();
	auto bytes = OpenFile(inputPath);

	if(!outputFile.good())
	{
		std::cout << "Cant open " + outputPath << endl;
		return;
	}

	for(size_t i = 0; i < bytes.size(); ++i)
	{
		char c = bytes[i];
		const string* pneumonic = &noPneumonic;

		auto it = pneumonics.find(c);
		if(it != pneumonics.end())
			pneumonic = &(it->second);

		outputFile << *pneumonic << ":";

		if((i + 1) % charactersPerLine == 0)
			outputFile << endl;
	}
}

void TextifyDirectory(const string& inputPath, const string& outputPath, const unordered_map<char, string>& pneumonics)
{
	filesystem::path input = inputPath;
	filesystem::path output = outputPath;

	if(!filesystem::exists(input))
		return;

	filesystem::remove_all(output);

	if(!filesystem::exists(output))
		filesystem::create_directory(output);

	auto replaceAll = [](string& str, string find, string replace) -> void
	{
		while(str.find(find) != std::string::npos)
			str.replace(str.find(find), find.size(), replace);
	};

	auto relativePath = [&replaceAll](string parentPath, string path) -> string
	{
		replaceAll(parentPath, wrongDash, correctDash);
		replaceAll(path,       wrongDash, correctDash);
		
		return path.substr(parentPath.size());
	};

	for(auto& file : filesystem::recursive_directory_iterator(input))
	{
		if(filesystem::is_directory(file.status()))
			continue;

		auto outputFilePath = outputPath + correctDash + relativePath(inputPath, file.path().string());
		outputFilePath += "_.txt";

		TextifyFile(file.path().string(), outputFilePath, pneumonics);
	}
}

int main(int argc, char** argv)
{
#ifdef __DEBUG__

	volatile char dummy;
	std::cout << "Connect debugger now or type any char and press enter" << endl;
	std::cin >> (char)dummy;

#endif //__DEBUG__
	
	param_utilities::init(argc, argv);

	string inputPath;
	if(!param_utilities::getParamValue("inputPath", inputPath))
	{
		std::cout << "Missing or empty -inputPath argument!" << endl;
		return 0;
	}

	string outputPath;
	if(!param_utilities::getParamValue("outputPath", outputPath))
	{
		std::cout << "Missing or empty -outputPath argument!" << endl;
		return 0;
	}

	unordered_map<char, string> pneumonics;
	buildPneumonics(pneumonics);

	if(param_utilities::isParam("fileMode"))
		TextifyFile(inputPath, outputPath, pneumonics);
	else if(param_utilities::isParam("dirMode"))
		TextifyDirectory(inputPath, outputPath, pneumonics);
	else
		std::cout << "Not specified mode! Required -fileMode or -dirMode!" << endl;

	return 0;
}