#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <stdio.h>

#include "lineDetection.h"
#include "intersectionDetection.h"
#include "util.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
	RNG rng(12345);

	Mat src;
	/// Load source image and convert it to gray
	src = imread(argv[1], 1);
	int t = getMilliCount();

	resize(src, src, Size(), 0.25, 0.25, INTER_LINEAR);

	Mat displayImage;
	Canny(src, displayImage, 50, 200, 3);
	cvtColor(displayImage, displayImage, CV_GRAY2BGR);

	cout << "Time consumed  until resized:" << getMilliSpan(t) << endl;

	vector<Vec4i> horz, vert;

	detectVertHorzLines(src, horz, vert, 2, 2);

	cout << "Time consumed until detected lines:" << getMilliSpan(t) << endl;

	vector<Point2f> intersections;
	getIntersections(intersections, horz, vert);

	cout << "Time consumend until all intersections found: " << getMilliSpan(t) << endl;


	vector<Point2f> selectedIntersections;
	selectBoardIntersections(src, intersections, selectedIntersections);
	cout << "Time consumed until refined all points:" << getMilliSpan(t) << endl;

	for (auto p : selectedIntersections) {
		circle(src, p, 5, Scalar(0, 255, 255), 5, 8);
		circle(displayImage, p, 5, Scalar(0, 255, 255), 5, 8);
	}
	for (auto p : intersections) {
		circle(src, p, 5, Scalar(180, 180, 180), 2, 8);
		circle(displayImage, p, 5, Scalar(180, 180, 180), 2, 8);
	}
	circle(displayImage, Point2f(src.cols/2, src.rows/2), 5, Scalar(0, 0, 255), 5, 8);

	for (auto h : horz) {
//		line(cdst, Point(h[0], h[1]), Point(h[2], h[3]),
//				Scalar(rng(255), rng(255), rng(255)), 3, CV_AA);
	}
	for (auto h : vert) {
//		line(cdst, Point(h[0], h[1]), Point(h[2], h[3]),
//				Scalar(rng(255), rng(255), rng(255)), 3, CV_AA);
	}

	imshow("source", src);
	imshow("detected lines", displayImage);
	//imshow("rotated", foo);

	waitKey();

	return 0;
}
