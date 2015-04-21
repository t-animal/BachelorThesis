#include "boardSegmenter.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

void BoardSegmenter::calculateBoundingBox(Rect boundingBox){
	Mat src(src);
	boundingBox = this->boundingBox;

	rectangle(src, Point(src.cols/2-20, src.rows/2-20), Point(src.cols/2+20, src.rows/2+20), Scalar(0,0,0), -1);
	floodFill(src, noArray(), Point(src.cols/2, src.rows/2), Scalar(120), &boundingBox);

	boundingBox.x -= 10;
	boundingBox.y -= 10;
	boundingBox.height += 20;
	boundingBox.width += 20;

	if(boundingBox.x < 0){
		boundingBox.height += boundingBox.x; boundingBox.x=0;
	}
	if(boundingBox.y < 0){
		boundingBox.width += boundingBox.y; boundingBox.y=0;
	}
	if(boundingBox.x + boundingBox.width > src.cols){
		boundingBox.width = src.cols-boundingBox.x;
	}
	if(boundingBox.y+boundingBox.height > src.rows){
		boundingBox.height = src.rows-boundingBox.y;
	}
}

void BoardSegmenter::segmentBoard(Mat &img){
	img = img(boundingBox);
}
