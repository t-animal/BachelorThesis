

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
	#define LOGD(...)
#endif

void detect(Mat &mgray, vector<Point2f> &intersections, vector<Point2f> &selectedIntersections){
	int t = getMilliCount();
	Mat src;
	resize(mgray, src, Size(800,480), 0,0, INTER_LINEAR);
	LOGD("Time consumed  until resized: %d", getMilliSpan(t));
	cout << "Time consumed  until resized:" << getMilliSpan(t) << endl;

	vector<Vec4i> horz, vert;
	detectVertHorzLines(src, horz, vert, 2, 2);
	LOGD("Time consumed until detected lines:%d", getMilliSpan(t));
	cout << "Time consumed until detected lines:" << getMilliSpan(t) << endl;

	getIntersections(intersections, horz, vert);
	LOGD("Time consumend until all intersections found: %d", getMilliSpan(t));
	cout << "Time consumend until all intersections found: " << getMilliSpan(t) << endl;

	selectBoardIntersections(src, intersections, selectedIntersections);
	LOGD("Time consumed until refined all points:%d", getMilliSpan(t));
	cout << "Time consumed until refined all points:" << getMilliSpan(t) << endl;

	LOGD("intersectionsCount: %d", intersections.size());
	LOGD("selectedIntersectionsCount: %d", selectedIntersections.size());
}

int main(int argc, char** argv) {
	RNG rng(12345);


	Mat src;
	/// Load source image and convert it to gray
	src = imread(argv[1], 1);
	resize(src, src, Size(800,480), 0,0, INTER_LINEAR);

	Mat displayImage;
	Canny(src, displayImage, 50, 200, 3);
	cvtColor(displayImage, displayImage, CV_GRAY2BGR);

	vector<Point2f> selectedIntersections, intersections;
	detect(src, intersections,  selectedIntersections);

	for (auto p : selectedIntersections) {
		circle(src, p, 5, Scalar(0, 255, 255), 5, 8);
		circle(displayImage, p, 5, Scalar(0, 255, 255), 5, 8);
	}
	for (auto p : intersections) {
		circle(src, p, 5, Scalar(180, 180, 180), 2, 8);
		circle(displayImage, p, 5, Scalar(180, 180, 180), 2, 8);
	}
	circle(displayImage, Point2f(src.cols/2, src.rows/2), 5, Scalar(0, 0, 255), 5, 8);

	imshow("source", src);
	imshow("detected lines", displayImage);
	//imshow("rotated", foo);

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
