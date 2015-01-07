#include <stdlib.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

void detectVertHorzLines(cv::Mat &img, std::vector<cv::Vec4i> &horz, std::vector<cv::Vec4i> &vert,
		float horzThreshhold = 2, float vertThreshhold = 2);

double getAverageAngle(std::vector<cv::Vec4i> lines);
