#ifndef LINEDETECTION_H_
#define LINEDETECTION_H_

#include <stdlib.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class LineDetector {
private:
	//calculates the distance of the point P(x,y) from the line l
	inline double distance(cv::Vec4i l, long x, long y){
		return abs((l[3]-l[1])*x - (l[2]-l[0])*y + l[2]*l[1] - l[3]*l[0]) / sqrt((l[3]-l[1])*(l[3]-l[1])+(l[2]-l[0])*(l[2]-l[0]));
	}

	void mergeNearbyLines(std::vector<cv::Vec4i> &horz, std::vector<cv::Vec4i> &vert);
	/**
	 * Detect all vertical and horizontal lines in an image. Verticality and horizontality are determined by their
	 * pitch compared to a specified threshhold.
	 */
	void detectVertHorzLines_HOUGH(
			cv::Mat &img,                   //!< the image to analyse
			std::vector<cv::Vec4i> &horz,   //!< output of horizontal lines
			std::vector<cv::Vec4i> &vert,   //!< output of vertical lines
			float horzThreshhold = 2,       //!< min absolute pitch against the x-axis of horizontal lines
			float vertThreshhold = 2        //!< min absolute pitch against the y-axis of vertical lines
			);

	void detectVertHorzLines_LSD (cv::Mat &img, std::vector<cv::Vec4i> &horz, std::vector<cv::Vec4i> &vert,
			float horzThreshhold, float vertThreshhold);

public:
	inline void detectVertHorzLines (cv::Mat &img, std::vector<cv::Vec4i> &horz, std::vector<cv::Vec4i> &vert,
			float horzThreshhold, float vertThreshhold){
		detectVertHorzLines_HOUGH(img, horz, vert, horzThreshhold, vertThreshhold);
	}

	/**
	 * Get the averagle angle to the x axis of all entries of \p lines. The return value is in the range -90 < x < 90.
	 */
	double getAverageAngle(std::vector<cv::Vec4i> lines);
};
#endif
