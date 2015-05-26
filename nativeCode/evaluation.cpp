#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <unordered_map>
#include <sys/time.h>
#include <time.h>

#include "evaluation.h"
#include "util.h"

using namespace std;
using namespace cv;

vector<pair<string, string>> Evaluater::usedValues = vector<pair<string, string>>();

Evaluater::Evaluater(const char *filename, Mat &image) :
		Evaluater(filename) {
	this->image = image;
}

Evaluater::Evaluater(const char *filename) {
	this->filename = string(filename);
	char annotFilename[strlen(filename) + 7];
	strcpy(annotFilename, filename);
	strcpy(&annotFilename[strlen(filename) - 4], "_annot.yml");

	FileStorage readStorage(annotFilename, FileStorage::READ);

	readStorage["emptyIntersects"] >> emptyIntersects;
	readStorage["blackIntersects"] >> blackIntersects;
	readStorage["whiteIntersects"] >> whiteIntersects;
	readStorage["blackPieces"] >> blackPieces;
	readStorage["whitePieces"] >> whitePieces;

	readStorage.release();

	allIntersects.reserve(emptyIntersects.size() + whiteIntersects.size() + blackIntersects.size());
	allIntersects.insert(allIntersects.end(), emptyIntersects.begin(), emptyIntersects.end());
	allIntersects.insert(allIntersects.end(), blackIntersects.begin(), blackIntersects.end());
	allIntersects.insert(allIntersects.end(), whiteIntersects.begin(), whiteIntersects.end());

	if (allIntersects.size() != 0) {
		evaluatable = true;
		convexHull(allIntersects, contour);
	}
}

FileStorage Evaluater::getFileStorage() {
	timeval curTime;
	gettimeofday(&curTime, NULL);

	char buffer[80];
	strftime(buffer, 80, "%Y-%m-%d--%T.", localtime(&curTime.tv_sec));

	string outputFilename("run_");
	outputFilename.append(buffer);
	outputFilename.append(to_string(curTime.tv_usec / 1000));

	FileStorage fs(outputFilename, FileStorage::WRITE);
	fs << "runTime" << buffer;
	fs << "filename" << filename;

	return fs;
}

FileStorage Evaluater::getMemoryStorage() {
	timeval curTime;
	gettimeofday(&curTime, NULL);

	char buffer[80];
	strftime(buffer, 80, "%Y-%m-%d--%T.", localtime(&curTime.tv_sec));

	FileStorage fs("foo.yml", FileStorage::WRITE | FileStorage::MEMORY);
	fs << "runTime" << buffer;
	fs << "filename" << filename;

	return fs;
}

void Evaluater::saveParameters(FileStorage fs) {
	fs << "usedParams" << "============= Line intentionally left blank ===========";
	for (auto current : usedValues) {
		fs << current.first << current.second;
	}
}

void Evaluater::checkIntersectionCorrectness(const vector<Point2f> &intersections, int xOffset = 0, int yOffset = 0) {
	if (!evaluatable)
		return;

	vector<Point2f> intersectCopy;
	intersectCopy.reserve(allIntersects.size());
	intersectCopy.insert(intersectCopy.begin(), allIntersects.begin(), allIntersects.end());

	int matched = 0;
	int insideKeypoints = 0;

	for (Point2f i : intersections) {
		i.x += xOffset;
		i.y += yOffset;
		bool wasMatched = false;
		if (pointPolygonTest(contour, i, true) >= -15) {
			insideKeypoints++;

			for (Point2f &ref : intersectCopy) {
				int offset = norm(i - ref);

				if (offset < 15) {
					circle(image, ref, 10, Scalar(0, 255, 0), 4);
					ref.x = -10;
					ref.y = -10;

					matched++;
					wasMatched = true;
					break;
				}
			}

		}else{
			circle(image, i, 10, Scalar(255, 255, 255), 4);
			continue;
		}
		if (!wasMatched)
			circle(image, i, 10, Scalar(0, 0, 255), 4);
	}
	for(auto ref : intersectCopy){
		if(ref.x >= 0)
			circle(image, ref, 10, Scalar(150, 150, 150), 4);
	}

	string test;
	FileStorage fs = getMemoryStorage();

	fs << "avg_matched" << (matched / (float) allIntersects.size() * 100);
	fs << "sum_available" << (int) allIntersects.size();
	fs << "sum_matched" << matched;
	fs << "sum_wrong" << insideKeypoints - matched;

	saveParameters(fs);

	test = fs.releaseAndGetString();

	cout << test << endl;
}

void Evaluater::checkFilledCorrectness(const vector<Point2f> &intersections) {
	if (!evaluatable)
		return;

	int matchedCount = 0;
	int unmatchedCount = 0;
	for (auto desired : allIntersects) {
		bool matched = false;
		for (auto is : intersections) {
			if (norm(desired - is) <= 15) {
				matched = true;
				break;
			}
		}
		if (!matched) {
			circle(image, desired, 10, Scalar(0, 0, 255), 4);
			unmatchedCount++;
		} else {
			circle(image, desired, 10, Scalar(0, 255, 0), 4);
			matchedCount++;
		}
	}

	string output;
	FileStorage fs = getMemoryStorage();

	float percentage = allIntersects.size() != 0 ? matchedCount * 100.0 / allIntersects.size() : 0;

	fs << "avg_correct" << percentage;
	fs << "sum_available" << (int) allIntersects.size();
	fs << "sum_matched" << matchedCount;
	fs << "sum_wrong" << (int)(allIntersects.size() - matchedCount);

	saveParameters(fs);

	output = fs.releaseAndGetString();

	cout << output << endl;

	if (allIntersects.size() == 0) {
		cout << "#There's no reference points";
		if (intersections.size() != 0)
			cout << " but keypoints have been found! == FAIL ==" << endl;
		else
			cout << " and no keypoints have been found. == SUCCESS == " << endl;
	} else {
		float percentage = matchedCount * 100.0 / allIntersects.size();
		cout << "#Matched " << matchedCount << " out of " << allIntersects.size() << " reference points. (";
		cout << std::setprecision(3) << percentage << "%) ";
		if (percentage >= 90) {
			cout << "== SUCCESS ==" << endl;
		} else {
			cout << "== FAIL == " << endl;
		}
	}
}

void Evaluater::checkPieceCorrectness(const vector<Point3f> &blackPieces, const vector<Point3f> &whitePieces,
		int xOffset = 0, int yOffset = 0) {
	if (!evaluatable)
		return;

	vector<Point3f> copyBlack, copyWhite;
	copyBlack.reserve(this->blackPieces.size());
	copyWhite.reserve(this->whitePieces.size());
	copyBlack.insert(copyBlack.end(), this->blackPieces.begin(), this->blackPieces.end());
	copyWhite.insert(copyWhite.end(), this->whitePieces.begin(), this->whitePieces.end());

	int matched = 0;
	int insidePieces = 0;


	for (auto p : blackPieces) {
		Point2f p2d(p.x, p.y);
		p2d.x += xOffset;
		p2d.y += yOffset;

		if (pointPolygonTest(contour, p2d, true) >= -15) {
			bool wasMatched = false;
			insidePieces++;

			for (auto &ref : copyBlack) {
				int locationOffset = norm(Point2f(ref.x, ref.y) - p2d);
				int sizeOffset = abs(ref.z-p.z);

				if (locationOffset < 15 && sizeOffset < 10) {
					circle(image, Point2f(ref.x, ref.y), 10, Scalar(0, 255, 0), 4);
					ref.x = -10;
					ref.y = -10;

					matched++;
					wasMatched = true;
					break;
				}
			}

			if (!wasMatched)
				circle(image, p2d, 10, Scalar(0, 0, 255), 4);
		}
	}


	for (auto p : whitePieces) {
		Point2f p2d(p.x, p.y);
		p2d.x += xOffset;
		p2d.y += yOffset;

		if (pointPolygonTest(contour, p2d, true) >= -15) {
			bool wasMatched = false;
			insidePieces++;

			for (auto &ref : copyWhite) {
				int locationOffset = norm(Point2f(ref.x, ref.y) - p2d);
				int sizeOffset = abs(ref.z-p.z);

				if (locationOffset < 15 && sizeOffset < 10) {
					circle(image, Point2f(ref.x, ref.y), 10, Scalar(0, 255, 0), 4);
					ref.x = -10;
					ref.y = -10;

					matched++;
					wasMatched = true;
					break;
				}
			}

			if (!wasMatched)
				circle(image, p2d, 10, Scalar(0, 0, 255), 4);
		}
	}


	string output;
	FileStorage fs = getMemoryStorage();

	int totalPieceCount = this->blackPieces.size() + this->whitePieces.size();
	fs << "avg_correct" << (totalPieceCount == 0? INT_MAX : (float) (matched / (float) totalPieceCount* 100));
	fs << "sum_available" << (int) (this->blackPieces.size() + this->whitePieces.size());
	fs << "sum_matched" << matched;
	fs << "sum_wrong" << insidePieces - matched;

	saveParameters(fs);

	output = fs.releaseAndGetString();

	cout << output << endl;
}



void Evaluater::checkOverallCorrectness(const char* board, vector<Point2f> intersections) {
	if (!evaluatable)
		return;

	int matchedCount=0;
	int falsePositive=0;
	int missed=0;
	int discarded=0;
	int wrongLoc=0;
	for(int c=0; c<81; c++){
		char color = board[c];
		if(color == 'u'){
			discarded+=81;
			break;
		}

		bool matched = false;
		vector<Point2f> annot = (color == 'b'?blackIntersects:(color=='w'?whiteIntersects:emptyIntersects));
		Point2f i = intersections[80-c%9*9-c/9];
		for(auto a:annot){
			if(norm(i-a) < 15){
				matched = true;
				break;
			}
		}


		if(matched)
			matchedCount++;
		else{
			for(auto a:allIntersects){
				if(norm(i-a) < 15){
					matched = true;
					break;
				}
			}
			if(matched)
				if(color == '0')
					missed++;
				else
					falsePositive++;
			else
				wrongLoc++;
		}
	}

	string output;
	FileStorage fs = getMemoryStorage();

	if(matchedCount+missed+falsePositive!=81)
		cout << filename << endl;

	fs << "sum_available" << (int) (blackIntersects.size()+whiteIntersects.size()+emptyIntersects.size());
	fs << "sum_occupied" << (int) (blackIntersects.size()+whiteIntersects.size());
	fs << "sum_empty" << (int) emptyIntersects.size();
	fs << "sum_matched" << matchedCount;
	fs << "sum_wrong" << missed + falsePositive;
	fs << "sum_missed" << missed;
	fs << "sum_falsePos" << falsePositive;
	fs << "sum_discarded" << discarded;
	fs << "sum_wrongLoc" << wrongLoc;

	saveParameters(fs);

	output = fs.releaseAndGetString();

	cout << "#" << output << endl;
}

void Evaluater::checkColorCorrectness(uchar board[], vector<Point2f> &intersections, int xOffset, int yOffset){
	int correct=0, wrong = 0, checked=0;
	for(int i=0; i<81; i++){
		Point2f intersection = intersections[i];
		intersection.x += xOffset;
		intersection.y += yOffset;

		double nearestDistance = INT_MAX;
		char nearestColor = 0;
		for(Point2f white: whiteIntersects){
			if(norm(white-intersection) < nearestDistance){
				nearestColor = 'w';
				nearestDistance = norm(white-intersection);
			}
		}
		for(Point2f black: blackIntersects){
			if(norm(black-intersection) < nearestDistance){
				nearestColor = 'b';
				nearestDistance = norm(black-intersection);
			}
		}
		for(Point2f empty: emptyIntersects){
			if(norm(empty-intersection) < nearestDistance){
				nearestColor = '0';
				nearestDistance = norm(empty-intersection);
			}
		}

		if(nearestDistance <= 15){
			checked++;
			//we've found the correct intersection
			if(nearestColor == board[i])
				correct++;
			else
				wrong++;
		}else{
			//the intersection itself was wrong, can't evaluate
		}
	}

	FileStorage fs = getMemoryStorage();
	fs << "sum_available" << checked << "sum_matched" << correct << "sum_wrong" << wrong;
	fs << "avg_quality" << ((checked != 0)? correct/(float)checked*100 : 0);
	saveParameters(fs);
	String output = fs.releaseAndGetString();

	cout << output;

}


void Evaluater::setStartTime(){
	startTime = getMilliCount();
}
void Evaluater::saveStepTime(string description){
	if(startTime == -1)
		return;

	stringstream ss;
	ss << "# " <<  setfill('0') << setw(3) << getMilliCount()-startTime << " :: " << description;
	stepTimes.push_back(ss.str());
}
void Evaluater::printStepTimes(){
	for(auto s : stepTimes){
		cout << s << endl;
	}
}

long Evaluater::conf(String name, long defaultVal) {
#ifdef USE_JNI
	return defaultVal;
#else
	char* value = getenv(name.c_str());
	long returnVal = value != NULL ? stol(value) : defaultVal;

	usedValues.push_back(pair<string, string>(name, to_string(returnVal)));

	return returnVal;
#endif
}

double Evaluater::conf(String name, double defaultVal) {
#ifdef USE_JNI
	return defaultVal;
#else
	char* value = getenv(name.c_str());
	double returnVal = value != NULL ? stod(value) : defaultVal;

	usedValues.push_back(pair<string, string>(name, to_string(returnVal)));

	return returnVal;
#endif
}

String Evaluater::conf(String name, String defaultVal) {
	char* value = getenv(name.c_str());
	String returnVal = value != NULL ? String(value) : defaultVal;

	usedValues.push_back(pair<string, string>(name, returnVal));

	return returnVal;
}
