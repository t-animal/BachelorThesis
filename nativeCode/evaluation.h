#ifndef EVALUATION_H_
#define EVALUATION_H_

class Evaluater {
private:
	std::vector<cv::Point2f> emptyIntersects;
	std::vector<cv::Point2f> blackPieces;
	std::vector<cv::Point2f> whitePieces;
	std::vector<cv::Point2f> allIntersects;
	cv::Mat image;

public:
	Evaluater(const char *filename);
	Evaluater(const char *, cv::Mat&);
	void setImage(cv::Mat &image){ this->image = image; }
	void checkIntersectionCorrectness(const std::vector<cv::Point2f>&);
	void checkOverallCorrectness(const std::vector<cv::Point2f>&);
};

#endif
