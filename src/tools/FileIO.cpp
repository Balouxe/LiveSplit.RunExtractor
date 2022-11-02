#include "FileIO.h"

std::string SplitsInfo::GetTitle() {
	if (variable != "") {
		return gameName + " - " + categoryName + " (" + variable + ")";
	}
	else {
		return gameName + " - " + categoryName;
	}
}

FileIO::FileIO() : isUsable(false) {
}

bool FileIO::OpenFile(const char* file) {
	tinyxml2::XMLError loadfileErr = xmlDoc.LoadFile(file);

	if(!loadfileErr == tinyxml2::XMLError::XML_SUCCESS){
		isUsable = false;
		return 0;
	}
	body = xmlDoc.FirstChildElement("Run");
	attempts = body->FirstChildElement("AttemptHistory");
	segments = body->FirstChildElement("Segments");
	isUsable = true;
	return 1;
}

bool FileIO::GetInfo() {
	if (!isUsable) {
		return 0;
	}
	info.gameName = body->FirstChildElement("GameName")->GetText();
	info.categoryName = body->FirstChildElement("CategoryName")->GetText();
	if (body->FirstChildElement("Metadata")->FirstChildElement("Variables")->FirstChildElement("Variable") != 0) {
		info.variable = body->FirstChildElement("Metadata")->FirstChildElement("Variables")->FirstChildElement("Variable")->GetText();
	}
	info.nbRuns = body->FirstChildElement("AttemptCount")->IntText();
	info.nbSplits = 0;
	for (tinyxml2::XMLElement* child = body->FirstChildElement("Segments")->FirstChildElement(); child != 0; child = child->NextSiblingElement()) {
		info.nbSplits++;
	}
	return 1;
}

bool FileIO::GetAllRuns() {
	runsVec.clear();
	tinyxml2::XMLElement* timeNode;
	RunInfo run;
	for (tinyxml2::XMLElement* child = attempts->FirstChildElement(); child != 0; child = child->NextSiblingElement())
	{
		run.id = child->IntAttribute("id");
		run.charID = child->Attribute("id");
		timeNode = child->FirstChildElement("RealTime");
		if (timeNode != NULL) {
			run.finishedTime = timeNode->GetText();
			run.finished = true;
		}
		else {
			run.finishedTime = "-";
			run.finished = false;
		}
		runsVec.push_back(run);
	}
	return 1;
}

bool FileIO::GetSplitsNames() {
	namesVec.clear();
	std::string name;
	for (tinyxml2::XMLElement* child = segments->FirstChildElement("Segment"); child != 0; child = child->NextSiblingElement())
	{
		name = child->FirstChildElement("Name")->GetText();
		namesVec.push_back(name);
	}
	return 1;
}

bool FileIO::GetBestSplitsTimes() {
	bestSplitsVec.clear();
	std::string best;
	for (tinyxml2::XMLElement* child = segments->FirstChildElement("Segment"); child != 0; child = child->NextSiblingElement())
	{
		if (child->FirstChildElement("BestSegmentTime")->FirstChildElement("RealTime") != 0) {
			best = child->FirstChildElement("BestSegmentTime")->FirstChildElement("RealTime")->GetText();
			best = best.substr(0, 12);
		}
		else {
			best = "";
		}
		bestSplitsVec.push_back(best);
	}
	return 1;
}

bool FileIO::GetSplitsTimes(std::string id) {
	splitsVec.clear();
	std::string time;
	// TODO: fix  (don't ask why this is called iwillkillyouandeveryoneyouknow)
	// bool iwillkillyouandeveryoneyouknow;
	for (tinyxml2::XMLElement* child = segments->FirstChildElement("Segment"); child != 0; child = child->NextSiblingElement())
	{
		//iwillkillyouandeveryoneyouknow = false;
		tinyxml2::XMLElement* segmentHist = child->FirstChildElement("SegmentHistory");
		for (tinyxml2::XMLElement* subchild = segmentHist->FirstChildElement("Time"); subchild != 0;subchild = subchild->NextSiblingElement()) {
			if (subchild->Attribute("id") == id) {
				if (subchild->FirstChildElement("RealTime") != 0) {
					std::string s = subchild->FirstChildElement("RealTime")->GetText();
					s = s.substr(0, 12);
					splitsVec.push_back(s);
					// iwillkillyouandeveryoneyouknow = true;
				}
				else {
					splitsVec.push_back("");
					//iwillkillyouandeveryoneyouknow = true;
				}
				break;
			}
		}
		//if (iwillkillyouandeveryoneyouknow == false)
		//	splitsVec.push_back("");
	}
	return 1;
}

std::chrono::duration<double, std::milli> FileIO::sToDuration(std::string& s) {
	std::chrono::duration<double, std::milli> dur;
	std::stringstream ss(s);
	std::chrono::from_stream(ss, "%T", dur);
	return dur;
}

std::string FileIO::durationToS(std::chrono::duration<double, std::milli> dur) {
	auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
	dur -= duration_cast<std::chrono::milliseconds>(secs);
	auto mins = duration_cast<std::chrono::minutes>(secs);
	secs -= duration_cast<std::chrono::seconds>(mins);
	auto hour = duration_cast<std::chrono::hours>(mins);
	mins -= duration_cast<std::chrono::minutes>(hour);	

	std::stringstream ss;
	ss << hour.count() << ":" << mins.count() << ":" << secs.count() << "." << dur.count();
	return ss.str();
}

FileIO Shared::fileio = FileIO();