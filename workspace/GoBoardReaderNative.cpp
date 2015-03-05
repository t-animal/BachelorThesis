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
inline void vector_Point3f_to_Mat(vector<Point3f>& v_rect, Mat& mat){
	mat = Mat(v_rect, true);
}

#define imshow(...) ""

#else
	#define LOGD(...) fprintf(stdout, __VA_ARGS__); cout << endl;
#endif

void drawHistogram(Mat &src){
	Mat hist;
	int channels[] = {0};
	float range[] = {0,255};
	const float* ranges[] = {range};
	int histsize[] = {255};
	calcHist(&src, 1, channels, Mat(), hist, 1, histsize, ranges, true, false);

	int bin_w = cvRound( (double) src.cols/255);
	normalize(hist, hist, 0, src.rows, NORM_MINMAX, -1, Mat() );
	for( int i = 1; i < 255; i++ ){
		line(src, Point( bin_w*(i-1), src.rows - cvRound(hist.at<float>(i-1)) ) ,
		           Point(bin_w * i,    src.rows - cvRound(hist.at<float>(i)) ),
		           Scalar( 255, 255, 255), 2, 8, 0  );
	}
}

void detectCircles(Mat &src, vector<Point3f> &darkCircles, vector<Point3f> &lightCircles){

//	Mat disp;
//	cvtColor(src, disp, COLOR_HSV2BGR);
//	Mat disp2 = disp.clone();

	Mat dst, dst2;
	vector<Mat> channels;
	split(src, channels);
	Mat h = channels[0];
	Mat s = channels[1];
	Mat v = channels[2];

	equalizeHist(v, v);
	equalizeHist(s, s);
	GaussianBlur(v, v, Size(7,7), 0);
	threshold(v, v, 30, 255, THRESH_BINARY);
	HoughCircles(v, darkCircles, CV_HOUGH_GRADIENT, 3, 15, 900, src.rows/12, src.rows/12, src.rows/11);

	GaussianBlur(s, s, Size(7,7), 0);
	threshold(s, s, 20, 255, THRESH_BINARY);
	HoughCircles(s, lightCircles, CV_HOUGH_GRADIENT, 3, 15, 900, src.rows/12, src.rows/12, src.rows/11);

//	for (Vec3f  c : darkCircles) {
//		circle(disp2, Point(c[0], c[1]), c[2], Scalar(80, 80, 80), 2, 8);
//	}
//	for (Vec3f c : lightCircles) {
//		circle(disp2, Point(c[0], c[1]), c[2], Scalar(255, 255, 255), 2, 8);
//	}
//
//	threshold(dst, src, 160, 255, THRESH_BINARY);
//	Canny(dst, disp, 900, 450, 3);
//	imshow("circledingsi", disp);
	imshow("circledingsi2", v);
}

void detect(Mat &src, vector<Point2f> &intersections, vector<Point2f> &selectedIntersections, vector<Point3f> &darkCircles, vector<Point3f> &lightCircles){
	int t = getMilliCount();
	Mat gray, hsv;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	cvtColor(src, hsv, COLOR_BGR2HSV);
//	resize(src, gray, Size(), 0.75,0.75, INTER_LINEAR);
//	LOGD("Time consumed  until resized: %d", getMilliSpan(t));

//	vector<Vec4i> horz, vert;
//	detectVertHorzLines(src, horz, vert, 2, 2);
//	LOGD("Time consumed until detected lines: %d", getMilliSpan(t));
//
//	getIntersections(intersections, horz, vert);
//	LOGD("Time consumend until all intersections found: %d", getMilliSpan(t));
//
//	selectBoardIntersections(src, intersections, selectedIntersections);
//	LOGD("Time consumed until refined all points: %d", getMilliSpan(t));
//
//	LOGD("intersectionsCount: %d", intersections.size());
//	LOGD("selectedIntersectionsCount: %d", selectedIntersections.size());

	detectCircles(hsv, darkCircles, lightCircles);
	LOGD("Time consumed until found circles: %d", getMilliSpan(t));
	LOGD("dark circles found %d", darkCircles.size());
	LOGD("light circles found %d", lightCircles.size());
	LOGD("====");
	LOGD("====");
}

int main(int argc, char** argv) {
	RNG rng(12345);
	Mat src;

	//load source image and store "as is" (rgb or bgr?) with alpha
	src = imread(argv[1], -1);

	//resize roughly to nexus4 camera size
	resize(src, src, Size(800, src.rows*800.0/src.cols), 0,0, INTER_LINEAR);

	vector<Point2f> selectedIntersections, intersections;
	vector<Point3f> darkCircles, lightCircles;

	detect(src, intersections, selectedIntersections, darkCircles, lightCircles);

	//paint the points onto another image
	Mat displayImage;
	cvtColor(src, displayImage, COLOR_BGR2GRAY);
	Canny(displayImage, displayImage, 50, 200, 3);
	cvtColor(displayImage, displayImage, COLOR_GRAY2BGR);

	for (Vec3f  c : darkCircles) {
		circle(displayImage, Point(c[0], c[1]), c[2], Scalar(80, 80, 80), 2, 8);
	}
	for (Vec3f c : lightCircles) {
		circle(displayImage, Point(c[0], c[1]), c[2], Scalar(255, 255, 255), 2, 8);
	}

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
			JNIEnv * jenv, jobject  obj, jlong src, jlong java_intersections, jlong java_selectedIntersections,
			jlong java_darkCircles, jlong java_lightCircles){

		vector<Point2f> selectedIntersections, intersections;
		vector<Point3f> darkCircles, lightCircles;

		detect(*(Mat*) src, intersections, selectedIntersections, darkCircles, lightCircles);

		LOGD("outside intersectionsCount: %d", intersections.size());
		LOGD("outside selectedIntersectionsCount: %d", selectedIntersections.size());
		vector_Point2f_to_Mat(selectedIntersections, *((Mat*)java_selectedIntersections));
		vector_Point2f_to_Mat(intersections, *((Mat*)java_intersections));
		vector_Point3f_to_Mat(darkCircles, *((Mat*)java_darkCircles));
		vector_Point3f_to_Mat(lightCircles, *((Mat*)java_lightCircles));
	}
}
#endif
