#ifndef EVALUATION_H_
#define EVALUATION_H_

#include <string>
#include <unordered_map>

using namespace std;

class Evaluater {
private:
	std::vector<cv::Point2f> emptyIntersects;
	std::vector<cv::Point2f> blackPieces;
	std::vector<cv::Point2f> whitePieces;
	std::vector<cv::Point2f> allIntersects;
	cv::Mat image;
	string filename;

	static vector<pair<string, string>> usedValues;

	cv::FileStorage getFileStorage();
	void saveParameters(cv::FileStorage fs);

public:
	Evaluater(const char *filename);
	Evaluater(const char *, cv::Mat&);

	void setImage(cv::Mat &image){ this->image = image; }
	void checkIntersectionCorrectness(const std::vector<cv::Point2f>&);
	void checkOverallCorrectness(const std::vector<cv::Point2f>&);


	static long conf(string name, long defaultVal);
	static double conf(string name, double defaultVal);
	static string conf(string name, string defaultVal);
};

#endif