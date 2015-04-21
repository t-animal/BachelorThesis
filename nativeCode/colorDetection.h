#ifndef COLORDETECTION_H_
#define COLORDETECTION_H_

#include <opencv2/core/core.hpp>

class ColorDetector{
private:
	const cv::Mat &src;
	const std::vector<cv::Point2f> &intersections;

public:
	ColorDetector(const cv::Mat &src, const std::vector<cv::Point2f> &intersections)
		: src(src), intersections(intersections){};

	void getColors(uchar *pieces);
};


#endif
