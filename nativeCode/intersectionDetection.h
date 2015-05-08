#ifndef INTERSECTIONDETECTION_H_
#define INTERSECTIONDETECTION_H_

#include <opencv2/core/core.hpp>

class IntersectionDetector {
private:
	/**
	 * Returns whether \p x0 <= \p x <= \p x1
	 */
	bool IsBetween(const double& x0, const double& x, const double& x1);

	/**
	 * Returns whether the two line segments \p a and \p b intersect
	 */
	cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b);

	const std::vector<cv::Vec4i> &horz;
	const std::vector<cv::Vec4i> &vert;
	const cv::Mat src;
	std::vector<cv::Point2f> *intersections = NULL;
public:
	IntersectionDetector(const std::vector<cv::Vec4i> &horz, const std::vector<cv::Vec4i> &vert, const cv::Mat src)
		: horz(horz), vert(vert), src(src){};
	IntersectionDetector(const cv::Mat src)
		: IntersectionDetector(std::vector<cv::Vec4i>(), std::vector<cv::Vec4i>(), src){};

	/**
	 * Get the intersections of all horizontal vectors \p horz with all vertical vectors \p vert. An intersection is only
	 * counted if there's no other intersection within \p maxOffset
	 */
	void getIntersections(
			std::vector<cv::Point2f> &intersections, //!< return vector to write the intersections to
			const int maxOffset = 10                 //!< distance in norm2 within which two intersections are
													 //!<   considered the same
			);

	void getIntersections_FAST(std::vector<cv::Point2f> &intersections);
	void getIntersections_ORB(std::vector<cv::Point2f> &intersections);

	/**
	 * Selects those intersections that are most certainly part of the Go-Board by selecting all within a specific range
	 * from the image center
	 */
	void selectBoardIntersections(
			std::vector<cv::Point2f> &selectedIntersections,  //!< output vector for the filtered intersections
			cv::Point2f center = cv::Point2f(-1,-1)            //!< center around which to get the intersections
			                                                  //!<    if not provided the image center is used
			);

	/**
	 * Selects those intersections that are part of the go-board. Starts at the center of the image, computes the distance
	 * between the two intersection closest to it and then repeatedly selects all the intersections closer than this
	 * distance to a previously selected one.
	 */
	void selectIntersectionsCloud(
			std::vector<cv::Point2f> &selectedIntersections   //!< output vector for the filtered intersections
			);

	/**
	 * Removes duplicates, ie intersections closer to each other than a specific threshhold
	 */
	void removeDuplicateIntersections();

	static void sort(std::vector<cv::Point2f> &intersections);
};
#endif
