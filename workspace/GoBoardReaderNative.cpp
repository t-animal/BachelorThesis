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

inline void vector_Point2f_to_Mat(vector<Point2f>& v_rect, Mat& mat) {
	mat = Mat(v_rect, true);
}
inline void vector_Point3f_to_Mat(vector<Point3f>& v_rect, Mat& mat) {
	mat = Mat(v_rect, true);
}

#define imshow(...) ""

#else
#define LOGD(...) fprintf(stdout, __VA_ARGS__); cout << endl;
#endif

void drawHistogram(const Mat &src, Mat &dst) {
	dst = src.clone();

	Mat hist;
	int channels[] = { 0 };
	float range[] = { 0, 255 };
	const float* ranges[] = { range };
	int histsize[] = { 255 };
	calcHist(&src, 1, channels, Mat(), hist, 1, histsize, ranges, true, false);

	int bin_w = cvRound((double) src.cols / 255);
	normalize(hist, hist, 0, src.rows, NORM_MINMAX, -1, Mat());
	for (int i = 1; i < 255; i++) {
		line(dst,
				Point(bin_w * (i - 1),
						src.rows - cvRound(hist.at<float>(i - 1))),
				Point(bin_w * i, src.rows - cvRound(hist.at<float>(i))),
				Scalar(255, 255, 255), 2, 8, 0);
	}
}

void detectCircles(Mat &src, vector<Point3f> &darkCircles,
		vector<Point3f> &lightCircles) {

	LOGD("src is a %s", type2str(src.type()).c_str());

	Mat disp;
	src.convertTo(disp, CV_8UC3);
	cvtColor(disp, disp, COLOR_HSV2BGR);

	vector<Mat> channels;
	split(src, channels);
	Mat h = channels[0];
	Mat s = channels[1];
	Mat v = channels[2];

//	drawHistogram(h, h1);
//	drawHistogram(s, s1);
//	drawHistogram(v, v1);
	//THE OPENCV DOCU HERE IS CRAP! => 0 ≤ v ≤ 255; 0 ≤ s ≤ 1; 0 ≤ h ≤ 360
	imshow("vPre", v / 255);
	imshow("sPre", s);
	imshow("hPre", h / 360);

//	equalizeHist(v, v);
//	equalizeHist(s, s);
//	equalizeHist(h, h);
	GaussianBlur(v, v, Size(13, 13), 0);
	GaussianBlur(s, s, Size(25, 25), 0);
	GaussianBlur(h, h, Size(25, 25), 0);

	threshold(v, v, 90, 255, THRESH_BINARY);
	threshold(s, s, 0.17, 1, THRESH_BINARY);
	threshold(h, h, 180, 360, THRESH_BINARY_INV);

	dilate(h, h, Mat(), Point(-1, -1), 5);
	dilate(s, s, Mat(), Point(-1, -1), 5);
	dilate(v, v, Mat(), Point(-1, -1), 5);
	erode(h, h, Mat(), Point(-1, -1), 5);
	erode(s, s, Mat(), Point(-1, -1), 5);
	erode(v, v, Mat(), Point(-1, -1), 5);

	imshow("hPost", h / 360);
	imshow("sPost", s);
	imshow("vPost", v / 255);

	h.convertTo((h /= 360) *= 255, CV_8UC1); //0.70833=255/360
	s.convertTo(s *= 255, CV_8UC1);
	v.convertTo(v, CV_8UC1);

	imshow("h8UC1", h);
	imshow("s8UC1", s);
	imshow("v8UC1", v);
	Mat h1;
	s.copyTo(h1);
//	Canny(h1, h1, 900, 450);

	//          (Input, Output,   method,            dp, minDist,  param1=100, param2=100, minRadius=0, maxRadius=0 )
	vector<Vec3f> lightCircles1, lightCircles2;
	HoughCircles(h, lightCircles1, CV_HOUGH_GRADIENT, 3, src.rows / 13, 900, 50,
			src.rows / 30, src.rows / 11);
	HoughCircles(s, lightCircles2, CV_HOUGH_GRADIENT, 3, src.rows / 13, 900, 50,
			src.rows / 30, src.rows / 11);
	HoughCircles(v, darkCircles, CV_HOUGH_GRADIENT, 3, src.rows / 15, 900, 50,
			src.rows / 20, src.rows / 11);

	LOGD("lc1 %d", lightCircles1.size());
	LOGD("lc2 %d", lightCircles2.size());

	lightCircles.insert(lightCircles.end(), lightCircles1.begin(),
			lightCircles1.end());
	lightCircles.insert(lightCircles.end(), lightCircles2.begin(),
			lightCircles2.end());

//	for(Point3f &c: lightCircles){
//		if(c.x == -1)
//			continue;
//
//		for(Point3f &c1:lightCircles){
//			if(c == c1 || c1.x == -1)
//				continue;
//			if(norm(Point2f(c.x, c.y)-Point2f(c1.x, c1.y)) < src.rows/16){
//				c = (c+c1);
//				c.x /= 2;
//				c.y /= 2;
//				c.z /= 2;
//			}
//			c1.x = -1;
//		}
//	}

	for (Vec3f c : darkCircles) {
		circle(disp, Point(c[0], c[1]), c[2], Scalar(80, 80, 80), 2, 8);
	}
	for (Vec3f c : lightCircles) {
		circle(disp, Point(c[0], c[1]), c[2], Scalar(255, 255, 255), 2, 8);
	}

	imshow("result", disp);
}

void detect(Mat &src, vector<Point2f> &intersections,
		vector<Point2f> &selectedIntersections, vector<Point3f> &darkCircles,
		vector<Point3f> &lightCircles) {
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
}

void loadAndProcessImage(char *filename){
	RNG rng(12345);
	Mat4f src;

	if(filename[strlen(filename)-1] == 'l'){
		FileStorage fs(filename, FileStorage::READ);

		fs["matrix"] >> src;

		fs.release();
	}else{
		//load source image and store "as is" (rgb or bgr?) with alpha
		src = imread(filename, -1);
	}

	LOGD("src is a %s", type2str(src.type()).c_str());

	vector<Point2f> selectedIntersections, intersections;
	vector<Point3f> darkCircles, lightCircles;

	detect(src, intersections, selectedIntersections, darkCircles,
			lightCircles);

	//paint the points onto another image
	Mat displayImage;
	src.convertTo(displayImage, CV_8UC3);
	cvtColor(displayImage, displayImage, COLOR_BGR2GRAY);
	Canny(displayImage, displayImage, 50, 200, 3);
	cvtColor(displayImage, displayImage, COLOR_GRAY2BGR);

	for (Vec3f c : darkCircles) {
		circle(displayImage, Point(c[0], c[1]), c[2], Scalar(80, 80, 80), 2, 8);
	}
	for (Vec3f c : lightCircles) {
		circle(displayImage, Point(c[0], c[1]), c[2], Scalar(255, 255, 255), 2,
				8);
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

//	namedWindow("detectedlines", WINDOW_AUTOSIZE);
//	namedWindow("source", WINDOW_AUTOSIZE);
//	imshow("source", src);
//	imshow("detectedlines", displayImage);

	waitKey();
}

int main(int argc, char** argv) {
	for(int i=1; i<argc; i++){
		loadAndProcessImage(argv[i]);
	}

	return 0;
}

#ifdef USE_JNI

extern "C" {
	JNIEXPORT void JNICALL Java_de_t_1animal_goboardreader_DetectorActivity_detect(
			JNIEnv * jenv, jobject obj, jlong src, jlong java_intersections, jlong java_selectedIntersections,
			jlong java_darkCircles, jlong java_lightCircles) {

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

	JNIEXPORT void JNICALL Java_de_t_1animal_goboardreader_DetectorActivity_saveAsYAML(
			JNIEnv *jenv, jobject obj, jlong src, jstring filename) {

		const char *path = jenv->GetStringUTFChars(filename, 0);
		FileStorage fs(path, FileStorage::WRITE);

		fs << "matrix" << (*(Mat*)src);

		fs.release();
		jenv->ReleaseStringUTFChars(filename, path);
	}
}
#endif
