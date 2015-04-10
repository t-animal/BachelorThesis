#ifndef INTERSECTIONDETECTION_H_
#define INTERSECTIONDETECTION_H_

#include <opencv2/core/core.hpp>

/**
 * Returns whether \p x0 <= \p x <= \p x1
 */
bool IsBetween(const double& x0, const double& x, const double& x1);

/**
 * FIXME: Probably broken anyway...
 */
bool FindIntersection(const double& x0, const double& y0, const double& x1,
		const double& y1, const double& a0, const double& b0, const double& a1,
		const double& b1, double& xy, double& ab);

/**
 * Returns whether the two line segments \p a and \p b intersect
 */
cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b);

/**
 * Get the intersections of all horizontal vectors \p horz with all vertical vectors \p vert. An intersection is only
 * counted if there's no other intersection within \p maxOffset
 */
void getIntersections(
		std::vector<cv::Point2f> &intersections, //!< return vector to write the intersections to
		const std::vector<cv::Vec4i> &horz,      //!< horizontal line segments
		const std::vector<cv::Vec4i> &vert,      //!< vertical line segments
		const int maxOffset = 10                 //!< distance in norm2 within which two intersections are
		                                         //!<   considered the same
		);

void getIntersections_FAST(std::vector<cv::Point2f> &intersections, cv::Mat src);

/**
 * Selects those intersections that are part of the go-board. Starts at the center of the image, computes the distance
 * between the two intersection closest to it and then repeatedly selects all the intersections closer than this
 * distance to a previously selected one.
 */
void selectBoardIntersections(
		cv::Mat &src,                                     //!< the src image
		std::vector<cv::Point2f> intersections,           //!< the input intersections
		std::vector<cv::Point2f> &selectedIntersections   //!< output vector for the filtered intersections
		);

/**
 * Removes duplicates, ie intersections closer to each other than a specific threshhold
 */
void removeDuplicateIntersections(
		std::vector<cv::Point2f> &intersections  //!< all the intersections
		);
#endif
