#ifndef LINEDETECTION_H_
#define LINEDETECTION_H_

#include <stdlib.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class LineDetector {
private:
	const cv::Mat src;
	float horzThreshhold;
	float vertThreshhold;

	//calculates the distance of the point P(x,y) from the line l
	inline double distance(cv::Vec4i l, long x, long y){
		return abs((l[3]-l[1])*x - (l[2]-l[0])*y + l[2]*l[1] - l[3]*l[0]) / sqrt((l[3]-l[1])*(l[3]-l[1])+(l[2]-l[0])*(l[2]-l[0]));
	}

	void mergeNearbyLines(std::vector<cv::Vec4i> &horz, std::vector<cv::Vec4i> &vert);
public:
	LineDetector(const cv::Mat src) : LineDetector(src, 2, 2){};
	LineDetector(const cv::Mat src, float horzThreshhold, float vertThreshhold)
		: src(src), horzThreshhold(horzThreshhold), vertThreshhold(vertThreshhold){};

	/**
	 * Detect all vertical and horizontal lines in an image. Verticality and horizontality are determined by their
	 * pitch compared to a specified threshhold. Lines are detected using probabalistic hough transform.
	 */
	void detectVertHorzLines_HOUGH(
			std::vector<cv::Vec4i> &horz,   //!< output of horizontal lines
			std::vector<cv::Vec4i> &vert    //!< output of vertical lines
			);

	/**
	 * Detect all vertical and horizontal lines in an image. Verticality and horizontality are determined by their
	 * pitch compared to a specified threshhold. Lines are detected using the LSD algorithm, then filtered by length
	 * and stitched.
	 */
	void detectVertHorzLines_LSD (
			std::vector<cv::Vec4i> &horz,   //!< output of horizontal lines
			std::vector<cv::Vec4i> &vert    //!< output of vertical lines
			);

	/**
	 * Get the averagle angle to the x axis of all entries of \p lines. The return value is in the range -90 < x < 90.
	 */
	static double getAverageAngle(std::vector<cv::Vec4i> lines);
};
#endif
