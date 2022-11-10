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
	tinyxml2::XMLElement* realTime;
	tinyxml2::XMLElement* gameTime;
	RunInfo run;
	for (tinyxml2::XMLElement* child = attempts->FirstChildElement(); child != 0; child = child->NextSiblingElement())
	{
		run.id = child->IntAttribute("id");
		run.charID = child->Attribute("id");
		realTime = child->FirstChildElement("RealTime");
		gameTime = child->FirstChildElement("GameTime");

		run.finished = false;

		if (realTime != NULL) {
			run.realTime = realTime->GetText();
			run.finished = true;
		}
		else {
			run.realTime = "-";
		}
		if (gameTime != NULL) {
			run.gameTime = gameTime->GetText();
			run.finished = true;
		}
		else {
			run.gameTime = "-";
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
		name = "";
		if(child->FirstChildElement("Name")->GetText() != 0)
			name = child->FirstChildElement("Name")->GetText();
		namesVec.push_back(name);
	}
	return 1;
}

bool FileIO::GetBestSplitsTimes() {
	bestSplitsRealVec.clear();
	bestSplitsGameVec.clear();
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
		bestSplitsRealVec.push_back(best);

		if (child->FirstChildElement("BestSegmentTime")->FirstChildElement("GameTime") != 0) {
			best = child->FirstChildElement("BestSegmentTime")->FirstChildElement("GameTime")->GetText();
			best = best.substr(0, 12);
		}
		else {
			best = "";
		}
		bestSplitsGameVec.push_back(best);
	}
	return 1;
}

bool FileIO::GetSplitsTimes(std::string id, bool gameTime) {
	splitsVec.clear();
	std::string time;
	// TODO: fix? alex's splits are weird and were my only sample so i still don't know if it's broken
	bool iwillkillyouandeveryoneyouknow;
	for (tinyxml2::XMLElement* child = segments->FirstChildElement("Segment"); child != 0; child = child->NextSiblingElement())
	{
		iwillkillyouandeveryoneyouknow = false;
		tinyxml2::XMLElement* segmentHist = child->FirstChildElement("SegmentHistory");
		for (tinyxml2::XMLElement* subchild = segmentHist->FirstChildElement("Time"); subchild != 0;subchild = subchild->NextSiblingElement()) {
			if (subchild->Attribute("id") == id) {
				if (!gameTime) {
					if (subchild->FirstChildElement("RealTime") != 0) {
						std::string s = subchild->FirstChildElement("RealTime")->GetText();
						s = s.substr(0, 12);
						splitsVec.push_back(s);
						iwillkillyouandeveryoneyouknow = true;
					}
					else {
						splitsVec.push_back("");
						iwillkillyouandeveryoneyouknow = true;
					}
				} else{
					if (subchild->FirstChildElement("GameTime") != 0) {
						std::string s = subchild->FirstChildElement("GameTime")->GetText();
						s = s.substr(0, 12);
						splitsVec.push_back(s);
						iwillkillyouandeveryoneyouknow = true;
					}
					else {
						splitsVec.push_back("");
						iwillkillyouandeveryoneyouknow = true;
					}
				}
				
				break;
			}
		}
		if (iwillkillyouandeveryoneyouknow == false)
			splitsVec.push_back("");
	}
	return 1;
}

std::chrono::duration<double, std::milli> FileIO::sToDuration(std::string& s) {
	std::chrono::duration<double, std::milli> dur;
	if (s == "") {
		dur = dur.zero();
		return dur;
	}
	std::stringstream ss(s);
	std::chrono::from_stream(ss, "%T", dur);
	return dur;
}

std::string FileIO::durationToS(std::chrono::duration<double, std::milli> dur) {
	// i know... this isn't by reference, but i have to modify "dur" in this function. no idea if that's the most efficient way to do it though.
	auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
	dur -= duration_cast<std::chrono::milliseconds>(secs);
	auto mins = duration_cast<std::chrono::minutes>(secs);
	secs -= duration_cast<std::chrono::seconds>(mins);
	auto hour = duration_cast<std::chrono::hours>(mins);
	mins -= duration_cast<std::chrono::minutes>(hour);

	std::stringstream ss;
	if (hour.count() / 10 < 1) {
		ss << "0" << hour.count() << ":";
	}
	else {
		ss << hour.count() << ":";
	}

	if (mins.count() / 10 < 1) {
		ss << "0" << mins.count() << ":";
	}
	else {
		ss << mins.count() << ":";
	}

	if (secs.count() / 10 < 1) {
		ss << "0" << secs.count() << ".";
	}
	else {
		ss << secs.count() << ".";
	}

	ss << dur.count();
	return ss.str();
}

FileIO Shared::fileio = FileIO();