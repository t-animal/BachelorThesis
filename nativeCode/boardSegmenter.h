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

	template<typename T> void segmentImages(T &img);
	template<typename T, typename... Args> void segmentImages(T &img, Args&... args);

	template<typename T> void unsegmentPoints(std::vector<T> &points);
	template<typename T, typename... Args> void unsegmentPoints(std::vector<T> &points, Args&... args);
};

template<typename T> void BoardSegmenter::unsegmentPoints(std::vector<T> &points){
	for(auto &p:points){
		p.x+=boundingBox.x;
		p.y+=boundingBox.y;
	}
}

template<typename T, typename... Args> void BoardSegmenter::unsegmentPoints(std::vector<T> &points, Args&... args){
	for(auto &p:points){
		p.x+=boundingBox.x;
		p.y+=boundingBox.y;
	}
	unsegmentPoints(args...);
}

template<typename T>  void BoardSegmenter::segmentImages(T &img){
	img = img(boundingBox);
}

template<typename T, typename... Args> void BoardSegmenter::segmentImages(T &img, Args&... args){
	img = img(boundingBox);
	segmentImages(args...);
}

#endif
