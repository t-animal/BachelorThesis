#include <opencv2/core/core.hpp>

#include "lineDetection.h"
#include "intersectionDetection.h"
#include "util.h"

using namespace cv;
using namespace std;

void detectPieces(Mat &src, vector<Point3f> &darkPieces, vector<Point3f> &lightPieces) {
	vector<Mat> channels;
	split(src, channels);
	Mat h = channels[0];
	Mat s = channels[1];
	Mat v = channels[2];

	//THE OPENCV DOCU HERE IS CRAP! => 0 ≤ v ≤ 255; 0 ≤ s ≤ 1; 0 ≤ h ≤ 360

	GaussianBlur(v, v, Size(13, 13), 0);
	GaussianBlur(s, s, Size(25, 25), 0);
	GaussianBlur(h, h, Size(25, 25), 0);

	threshold(v, v, 70, 255, THRESH_BINARY);
	threshold(s, s, 0.17, 1, THRESH_BINARY);
	threshold(h, h, 70, 360, THRESH_BINARY);

	//remove speckles
	dilate(h, h, Mat(), Point(-1, -1), 5);
	dilate(s, s, Mat(), Point(-1, -1), 5);
	dilate(v, v, Mat(), Point(-1, -1), 5);
	erode(h, h, Mat(), Point(-1, -1), 5);
	erode(s, s, Mat(), Point(-1, -1), 5);
	erode(v, v, Mat(), Point(-1, -1), 5);

	if(countNonZero(h) < h.rows*h.cols*4/5){ //wenn weniger als 80% weiss, discarde
		h = Mat::zeros(h.size(), h.type());
	}
	if(countNonZero(s) < s.rows*s.cols*4/5){ //wenn weniger als 80% weiss, discarde
		s = Mat::zeros(s.size(), s.type());
	}

	h.convertTo((h /= 360) *= 255, CV_8UC1); //0.70833=255/360
	s.convertTo(s *= 255, CV_8UC1);
	v.convertTo(v, CV_8UC1);

	//          (Input, Output,   method,            dp, minDist,      param1, param2, minRad,  maxRad )
	vector<Vec3f> lightCircles1, lightCircles2;
	HoughCircles(h, lightCircles1, CV_HOUGH_GRADIENT, 3, src.rows / 13, 900, 50, src.rows / 30, src.rows / 11);
	HoughCircles(s, lightCircles2, CV_HOUGH_GRADIENT, 3, src.rows / 13, 900, 50, src.rows / 30, src.rows / 11);
	HoughCircles(v, darkPieces,   CV_HOUGH_GRADIENT, 3, src.rows / 15, 900, 50, src.rows / 20, src.rows / 11);

	lightPieces.insert(lightPieces.end(), lightCircles1.begin(), lightCircles1.end());
	lightPieces.insert(lightPieces.end(), lightCircles2.begin(), lightCircles2.end());
}
