#ifndef GAPS_FILLING_H_
#define GAPS_FILLING_H_

#include <opencv2/core/core.hpp>

class GapsFiller{
private:
	int squareLength;
	cv::Point2f center;
	cv::Mat disp;
	cv::Mat H;

	/*
	 * Generates a set of "perfect" keypoints modeling a Go-Board of \p squareLength size
	 */
	void generateReferenceKeypoints(
			std::vector<cv::Point2f> &object,   //!< return vector for the modeled keypoints
			int squareLength                    //!< the size of the go-board to model
	);

	/*
	 * Generates a set of "perfect" keypoints modeling a Go-Board trying to leave out those which do not have corresponding
	 * keypoints in the \p intersections vector
	 */
	void generateCorrespondingKeypoints(
			std::vector<cv::Point2f> &keypoints,      //!< return vector of the modeled keypoints
			std::vector<cv::Point2f> &intersections   //!< the intersections to model the keypoints after
	);

public:
	GapsFiller(int squareLength, cv::Point2f center)
		: squareLength(squareLength), center(center){};
	GapsFiller(int squareLength, cv::Point2f center, cv::Mat img)
		:squareLength(squareLength), center(center), disp(img){};

	/*
	 * Fills in the gaps of intersections which have not been found on basis of the ones already found
	 */
	void fillGaps(
			std::vector<cv::Point2f> intersections,          //!< the intersections that have been found so far
			std::vector<cv::Point2f> &filledIntersections    //!< the return vector for the filled intersections
	);

	void refine(std::vector<cv::Point2f> intersections, std::vector<cv::Point2f> &filledIntersections);

	cv::Mat getImageTransformationMatrix();
};
#endif
