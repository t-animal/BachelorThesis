#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <iostream>
#include <stdio.h>

#include "lineDetection.h"
#include "intersectionDetection.h"
#include "util.h"
#include "pieceDetection.h"

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

void generateReferenceKeypoints(vector<Point2f> &object, int squareLength, Point2f &mp){
	int offset = (squareLength-1)/2;
	for(int i=0; i<squareLength; i++){
		for(int j=0; j<squareLength; j++){
			object.push_back(Point2f(mp.x+(offset-i)*50, mp.y+(offset-j)*50));
		}
	}
}

void removeDuplicateIntersections(vector<Point2f> &intersections){
	for(Point2f &i : intersections){
		for(Point2f &j : intersections){
			if(i == j || i.x < 0 || j.x < 0)
				continue;

			if(norm(i-j)< 20){
				j.x = -100;
				j.y = -100;
			}
		}
	}
}

void detect(Mat &src, vector<Point2f> &intersections, vector<Point2f> &selectedIntersections,
		vector<Point3f> &darkCircles, vector<Point3f> &lightCircles) {
	int t = getMilliCount();

//	resize(src, src, Size(), 0.75, 0.75, INTER_LINEAR);
//	LOGD("Time consumed  until resized: %d", getMilliSpan(t));

	Mat gray, hsv, bgr;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	cvtColor(src, hsv, COLOR_BGR2HSV);
	src.convertTo(bgr, CV_8UC4);

	vector<Vec4i> horz, vert;
	detectVertHorzLines(bgr, horz, vert, 2, 2);
	LOGD("Time consumed until detected lines: %d", getMilliSpan(t));

	getIntersections(intersections, horz, vert);
	LOGD("Time consumend until all intersections found: %d", getMilliSpan(t));

	detectPieces(hsv, darkCircles, lightCircles);
	LOGD("Time consumed until found circles: %d", getMilliSpan(t));

	for(auto c : darkCircles){
		intersections.push_back(Point2f(c.x, c.y));
	}
	for(auto c : lightCircles){
		intersections.push_back(Point2f(c.x, c.y));
	}

	removeDuplicateIntersections(intersections);
	LOGD("Time consumed until removed duplicates: %d", getMilliSpan(t));

	selectBoardIntersections(src, intersections, selectedIntersections);
	LOGD("Time consumed until refined all points: %d", getMilliSpan(t));
}


void loadAndProcessImage(char *filename) {
	RNG rng(12345);
	Mat4f src;

	if (filename[strlen(filename) - 1] == 'l') {
		FileStorage fs(filename, FileStorage::READ);

		fs["matrix"] >> src;

		fs.release();
	} else {
		//load source image and store "as is" (rgb or bgr?) with alpha
		src = imread(filename, -1);
		src.convertTo(src, CV_RGBA2BGRA);
	}

	LOGD("src is a %s", type2str(src.type()).c_str());

	vector<Point2f> selectedIntersections, intersections;
	vector<Point3f> darkCircles, lightCircles;

	detect(src, intersections, selectedIntersections, darkCircles, lightCircles);

	//paint the points onto another image
	Mat grayDisplay, colorDisplay;
	src.convertTo(grayDisplay, CV_8UC3);
	src.convertTo(colorDisplay, CV_8UC3);
	cvtColor(colorDisplay, colorDisplay, COLOR_RGB2BGR);
	cvtColor(grayDisplay, grayDisplay, COLOR_BGR2GRAY);
	Canny(grayDisplay, grayDisplay, 50, 200, 3);
	cvtColor(grayDisplay, grayDisplay, COLOR_GRAY2BGR);

	for (Vec3f c : darkCircles) {
		circle(colorDisplay, Point(c[0], c[1]), c[2], Scalar(80, 80, 80), 2, 8);
		circle(grayDisplay, Point(c[0], c[1]), c[2], Scalar(80, 80, 80), 2, 8);
	}
	for (Vec3f c : lightCircles) {
		circle(colorDisplay, Point(c[0], c[1]), c[2], Scalar(255, 255, 255), 2, 8);
		circle(grayDisplay, Point(c[0], c[1]), c[2], Scalar(255, 255, 255), 2, 8);
	}

	for (auto p : selectedIntersections) {
		circle(colorDisplay, p, 5, Scalar(0, 255, 255), 5, 8);
		circle(grayDisplay, p, 5, Scalar(0, 255, 255), 5, 8);
	}
	for (auto p : intersections) {
		circle(colorDisplay, p, 5, Scalar(180, 180, 180), 2, 8);
		circle(grayDisplay, p, 5, Scalar(180, 180, 180), 2, 8);
	}
	Point2f center(src.cols/2, src.rows/2);
	circle(grayDisplay, Point2f(src.cols/2, src.rows/2), 5, Scalar(0, 0, 255), 5, 8);
	circle(colorDisplay, Point2f(src.cols/2, src.rows/2), 5, Scalar(0, 0, 255), 5, 8);


	vector<Point2f> object, scene;
	while(selectedIntersections.size() < 25)
		selectedIntersections.push_back(Point2f(-100, -100));
	generateReferenceKeypoints(object, sqrt(selectedIntersections.size()), center);
	generateReferenceKeypoints(scene, 9, center);

	if(object.size() != selectedIntersections.size()){
		LOGD("homography detection impossible: object: %d, intersections: %d", object.size(), selectedIntersections.size());
	}else{
		Mat H = findHomography(object, selectedIntersections, RANSAC);
		perspectiveTransform(scene, scene, H);
		//warpPerspective(colorDisplay, colorDisplay, H, colorDisplay.size());

		for(auto p : scene){
			circle(colorDisplay, p, 8, Scalar(0,0,255), 2, 8);
		}
	}

	namedWindow("detectedlines", WINDOW_AUTOSIZE);
	namedWindow("source", WINDOW_AUTOSIZE);
	imshow("source", colorDisplay);
	imshow("detectedlines", grayDisplay);

	waitKey();
}

int main(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
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
