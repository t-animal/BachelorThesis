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

#ifdef USE_JNI

#include <jni.h>
#include <android/log.h>

#define LOG_TAG "T_ANIMAL::GBR::NativeComponent"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

inline void vector_Point2f_to_Mat(vector<Point2f>& v_rect, Mat& mat){
	mat = Mat(v_rect, true);
}

#else
	#define LOGD(...) fprintf(stdout, __VA_ARGS__); cout << endl;
#endif

void detect(Mat &src, vector<Point2f> &intersections, vector<Point2f> &selectedIntersections){
	int t = getMilliCount();
//	Mat src;
//	resize(src, src, Size(), 0.75,0.75, INTER_LINEAR);
//	LOGD("Time consumed  until resized: %d", getMilliSpan(t));

	vector<Vec4i> horz, vert;
	detectVertHorzLines(src, horz, vert, 2, 2);
	LOGD("Time consumed until detected lines: %d", getMilliSpan(t));

	getIntersections(intersections, horz, vert);
	LOGD("Time consumend until all intersections found: %d", getMilliSpan(t));

	selectBoardIntersections(src, intersections, selectedIntersections);
	LOGD("Time consumed until refined all points: %d", getMilliSpan(t));

	LOGD("intersectionsCount: %d", intersections.size());
	LOGD("selectedIntersectionsCount: %d", selectedIntersections.size());


	vector<Vec3f> darkCircles,lightCircles;
	Mat dst;
	cvtColor(src, dst, CV_BGR2GRAY);
	GaussianBlur(dst, dst, Size(3, 3), 0.7);
	threshold(dst, dst, 15, 255, THRESH_BINARY);
	HoughCircles(dst, darkCircles, CV_HOUGH_GRADIENT, 3, 15, 900, 20, 8, 15);
	LOGD("circles found %d", darkCircles.size())
	LOGD("Time consumed until found dark circles: %d", getMilliSpan(t));

	cvtColor(src, dst, CV_BGR2GRAY);
	GaussianBlur(dst, dst, Size(3, 3), 0.7);
	threshold(dst, dst, 190, 255, THRESH_BINARY);
	HoughCircles(dst, lightCircles, CV_HOUGH_GRADIENT, 3, 15, 900, 27, 8, 15);
	LOGD("circles found %d", darkCircles.size())
	LOGD("Time consumed until found light circles: %d", getMilliSpan(t));

	for (auto c : darkCircles) {
		circle(src, Point(c[0], c[1]), c[2], Scalar(80, 80, 80), 2, 8);
	}
	for (auto c : lightCircles) {
		circle(src, Point(c[0], c[1]), c[2], Scalar(255, 255, 255), 2, 8);
	}

	Mat disp = dst.clone();
	Mat disp2 = dst.clone();
	Canny(dst, disp, 900, 450, 3);
	imshow("circledingsi", disp);
	imshow("circledingsi2", disp2);

}

int main(int argc, char** argv) {
	RNG rng(12345);
	Mat src;

	//load source image and convert it to gray
	src = imread(argv[1], 1);

	//resize roughly to nexus4 camera size
	resize(src, src, Size(800, src.rows*800.0/src.cols), 0,0, INTER_LINEAR);

	vector<Point2f> selectedIntersections, intersections;
	detect(src, intersections, selectedIntersections);

	//paint the points onto another image
	Mat displayImage;
	Canny(src, displayImage, 50, 200, 3);
	cvtColor(displayImage, displayImage, COLOR_GRAY2BGR);

//	for (auto p : selectedIntersections) {
//		circle(src, p, 5, Scalar(0, 255, 255), 5, 8);
//		circle(displayImage, p, 5, Scalar(0, 255, 255), 5, 8);
//	}
//	for (auto p : intersections) {
//		circle(src, p, 5, Scalar(180, 180, 180), 2, 8);
//		circle(displayImage, p, 5, Scalar(180, 180, 180), 2, 8);
//	}
//	circle(displayImage, Point2f(src.cols/2, src.rows/2), 5, Scalar(0, 0, 255), 5, 8);

	namedWindow("detectedlines", WINDOW_AUTOSIZE);
	namedWindow("source", WINDOW_AUTOSIZE);
	imshow("source", src);
	imshow("detectedlines", displayImage);

	waitKey();

	return 0;
}


#ifdef USE_JNI

extern "C"{
	JNIEXPORT void JNICALL Java_de_t_1animal_goboardreader_DetectorActivity_detect(
			JNIEnv * jenv, jobject  obj, jlong mgray, jlong java_intersections, jlong java_selectedIntersections){

		vector<Point2f> selectedIntersections, intersections;

		detect(*(Mat*) mgray, intersections, selectedIntersections);

		LOGD("outside intersectionsCount: %d", intersections.size());
		LOGD("outside selectedIntersectionsCount: %d", selectedIntersections.size());
		vector_Point2f_to_Mat(selectedIntersections, *((Mat*)java_selectedIntersections));
		vector_Point2f_to_Mat(intersections, *((Mat*)java_intersections));
	}
}
#endif
