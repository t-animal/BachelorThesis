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

	cout << "Time consumed  until resized:" << getMilliSpan(t) << endl;

	vector<Vec4i> horz, vert;

	detectVertHorzLines(src, horz, vert, 2,2);

	cout << "Time consumed until detected lines:" << getMilliSpan(t) << endl;

	Mat cdst;
	Canny(src, cdst, 50, 200, 3);
	cvtColor(cdst, cdst, CV_GRAY2BGR);

	vector<Point2f> intersections;
	getIntersections(intersections, horz, vert);

	vector<pair<double, Point2f> > distances;
	Point2f center(src.cols/2, src.rows/2);
	for (auto p : intersections) {
		distances.push_back(pair<double, Point2f>(norm(center-p), p));
	}

	sort(distances.begin(), distances.end(), sortFunction);

	vector<Point2f> innerIntersections;
	pair<double, Point2f> last = distances.front();
	double lastFirstDeriv  = 0;
	double lastSecondDeriv = 0;
	for(auto d : distances){
		double curFirstDeriv  = d.first-last.first;
		double curSecondDeriv = curFirstDeriv - lastFirstDeriv;

		if(lastSecondDeriv-curSecondDeriv > 50)
			break;

		lastFirstDeriv = curFirstDeriv;
		lastSecondDeriv = curSecondDeriv;
		last = d;
		innerIntersections.push_back(d.second);
	}

	cout << "Time consumed until refined all points:" << getMilliSpan(t)
			<< endl;

	for (auto p : innerIntersections) {
		circle(src, p, 5, Scalar(0, 255, 255), 5, 8);
		circle(cdst, p, 5, Scalar(0, 255, 255), 5, 8);
	}
	for (auto p : intersections) {
		circle(src, p, 5, Scalar(180, 180, 180), 2, 8);
		circle(cdst, p, 5, Scalar(180, 180, 180), 2, 8);
	}
	circle(cdst, center, 5, Scalar(0, 0, 255), 5, 8);

	for (auto h : horz) {
//		line(cdst, Point(h[0], h[1]), Point(h[2], h[3]),
//				Scalar(rng(255), rng(255), rng(255)), 3, CV_AA);
	}
	for (auto h : vert) {
//		line(cdst, Point(h[0], h[1]), Point(h[2], h[3]),
//				Scalar(rng(255), rng(255), rng(255)), 3, CV_AA);
	}

	imshow("source", src);
	imshow("detected lines", cdst);
	//imshow("rotated", foo);

	waitKey();

	return 0;
}
