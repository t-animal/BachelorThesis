/*!
 * \file lineDetection.h
 */

#include <stdlib.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

/**
 * Detect all vertical and horizontal lines in an image. Verticality and horizontality are determined by their
 * pitch compared to a specified threshhold.
 */
void detectVertHorzLines(
		cv::Mat &img,                   //!< the image to analyse
		std::vector<cv::Vec4i> &horz,   //!< output of horizontal lines
		std::vector<cv::Vec4i> &vert,   //!< output of vertical lines
		float horzThreshhold = 2,       //!< min absolute pitch against the x-axis of horizontal lines
		float vertThreshhold = 2        //!< min absolute pitch against the y-axis of vertical lines
		);

/**
 * Get the averagle angle to the x axis of all entries of \p lines. The return value is in the range -90 < x < 90.
 */
double getAverageAngle(std::vector<cv::Vec4i> lines);
