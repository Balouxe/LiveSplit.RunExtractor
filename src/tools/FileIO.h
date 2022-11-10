#pragma once

#include "../tinyxml2/tinyxml2.h"
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>

struct SplitsInfo {
	std::string gameName;
	std::string categoryName;
	std::string variable = "";
	int nbRuns;
	int nbSplits;
	std::string GetTitle();
};

struct RunInfo {
	int id;
	const char* charID;
	bool finished;
	const char* realTime;
	const char* gameTime;
};

class FileIO
{
public:
	FileIO();
	bool OpenFile(const char* file);
	bool GetInfo();
	bool GetAllRuns();
	bool GetSplitsNames();
	bool GetBestSplitsTimes();
	bool GetSplitsTimes(std::string id, bool gameTime = false);

	static std::chrono::duration<double, std::milli> sToDuration(std::string& s);
	static std::string durationToS(std::chrono::duration<double, std::milli> dur);

	std::vector<RunInfo> runsVec;
	std::vector<std::string> namesVec;
	std::vector<std::string> bestSplitsRealVec;
	std::vector<std::string> bestSplitsGameVec;
	std::vector<std::string> splitsVec;
	SplitsInfo info;
private:
	bool isUsable;
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLElement* body;
	tinyxml2::XMLNode* attempts;
	tinyxml2::XMLNode* segments;
};

class Shared {
public:
	static FileIO fileio;
};