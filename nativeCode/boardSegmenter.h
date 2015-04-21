#ifndef BOARD_SEGMENTER_H_
#define BOARD_SEGMENTER_H_

#include <opencv2/core/core.hpp>

class BoardSegmenter{

private:
	cv::Rect boundingBox;
	const cv::Mat src;

public:
	BoardSegmenter(const cv::Mat src) : src(src){};

	void calculateBoundingBox(cv::Rect &bBox);

	void segmentImage(cv::Mat &img);

	template<typename Tp> void unsegmentPoints(std::vector<Tp> &points);
};
#endif

template<typename Tp> void BoardSegmenter::unsegmentPoints(std::vector<Tp> &points){
	for(auto &p:points){
		reinterpret_cast<cv::Point2f&>(p).x+=boundingBox.x;
		reinterpret_cast<cv::Point2f&>(p).y+=boundingBox.y;
	}
}
