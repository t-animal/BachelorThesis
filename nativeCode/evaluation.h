#ifndef EVALUATION_H_
#define EVALUATION_H_

#include <opencv2/highgui/highgui.hpp>

#include <string>
#include <unordered_map>

using namespace std;

class Evaluater {
private:
	std::vector<cv::Point2f> emptyIntersects;
	std::vector<cv::Point2f> blackIntersects;
	std::vector<cv::Point2f> whiteIntersects;
	std::vector<cv::Point3f> blackPieces;
	std::vector<cv::Point3f> whitePieces;
	std::vector<cv::Point2f> allIntersects;
	cv::Mat image;
	string filename;
	std::vector<cv::Point2f> contour;
	bool evaluatable = false;

	int startTime = -1;
	std::vector<std::string> stepTimes;

	static vector<pair<string, string>> usedValues;

	cv::FileStorage getFileStorage();
	cv::FileStorage getMemoryStorage();
	void saveParameters(cv::FileStorage fs);

public:
	Evaluater(const char *filename);
	Evaluater(const char *, cv::Mat&);

	void setImage(cv::Mat &image){ this->image = image; }
	void checkIntersectionCorrectness(const std::vector<cv::Point2f>&, int xOffset, int yOffset);
	void checkPieceCorrectness(const std::vector<cv::Point3f>&, const std::vector<cv::Point3f>&, int xOffset, int yOffset);
	void checkFilledCorrectness(const std::vector<cv::Point2f>&);
	void checkColorCorrectness(uchar board[], std::vector<cv::Point2f> &intersections, int xOffset = 0, int yOffset = 0);
	void checkOverallCorrectness(const char* board, std::vector<cv::Point2f> intersections);

	void setStartTime();
	void saveStepTime(string description);
	void printStepTimes();

	static long conf(string name, long defaultVal);
	static double conf(string name, double defaultVal);
	static string conf(string name, string defaultVal);

#ifndef USE_JNI
	void showImage(){cv::imshow("evaluated", image);}
#endif
};

#endif
