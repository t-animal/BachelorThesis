#include "boardSegmenter.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;

void BoardSegmenter::calculateBoundingBox(Rect &bBox){
	Mat img;
	src.copyTo(img);

	rectangle(img, Point(img.cols/2-40, img.rows/2-40), Point(img.cols/2+40, img.rows/2+40), Scalar(0,0,0), -1);
	floodFill(img, noArray(), Point(img.cols/2, img.rows/2), Scalar(120), &boundingBox);

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
	if(boundingBox.x + boundingBox.width > img.cols){
		boundingBox.width = img.cols-boundingBox.x;
	}
	if(boundingBox.y+boundingBox.height > img.rows){
		boundingBox.height = img.rows-boundingBox.y;
	}

	bBox = this->boundingBox;
}
