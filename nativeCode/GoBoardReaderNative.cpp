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
#include "gapsFilling.h"
#include "evaluation.h"

using namespace cv;
using namespace std;

Evaluater *globEval;

void rotate(vector<Point2f>& src, vector<Point2f>& dst, Point2f center, double angle) {
	Mat r = getRotationMatrix2D(center, angle, 1.0);

	transform(src, dst, r);
}

void getColors(const vector<Point2f> &intersections, uchar *pieces, Mat threshed){
	uchar *start = threshed.datastart;
	while(start != threshed.dataend){
		if(*start == 0)
			*start = 255;

		if(*start != 255)
			*start = 0;

		start++;
	}
	erode(threshed, threshed, Mat(), Point(-1, -1), 2);
	dilate(threshed, threshed, Mat(), Point(-1, -1), 2);

	int curPiece = 0;
	for(auto i : intersections){
		Mat subPix;
		getRectSubPix(threshed, Size(20,20), i, subPix);
		if((sum(subPix)/400)[0] < 80){
			pieces[curPiece++] = 'b';//sum(subPix)[0]/100*(-1);
			continue;
		}

		if((sum(subPix)/400)[0] > 220){
			pieces[curPiece++] = 'w';//sum(subPix)[0]/100;
			continue;
		}

		pieces[curPiece++] = '0';
	}
}

void detect(Mat src, vector<Point2f> &intersections, vector<Point2f> &selectedIntersections,
		vector<Point2f> &filledIntersections, vector<Point3f> &darkCircles, vector<Point3f> &lightCircles) {
	int t = getMilliCount();

//	resize(src, src, Size(), 0.75, 0.75, INTER_LINEAR);
//	LOGD("Time consumed  until resized: %d", getMilliSpan(t));

	Mat gray, hsv, bgr;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	cvtColor(src, hsv, COLOR_BGR2HSV);
	src.convertTo(bgr, CV_8UC4);

	Rect bounding = Rect();
	Mat threshed, mask;
	mask = Mat1i(Size(src.cols+2, src.rows+2));

	gray.convertTo(threshed, CV_8UC1);
	adaptiveThreshold(threshed, threshed, 255, ADAPTIVE_THRESH_MEAN_C,CV_THRESH_BINARY, 31, 1);
	threshed.convertTo(threshed, CV_8UC3);

	rectangle(threshed, Point(src.cols/2-20, src.rows/2-20), Point(src.cols/2+20, src.rows/2+20), Scalar(0,0,0), -1);
	floodFill(threshed, noArray(), Point(src.cols/2, src.rows/2), Scalar(120), &bounding);
	imshow("thrte", threshed);

	bounding.x -= 10;
	bounding.y -= 10;
	bounding.height += 20;
	bounding.width += 20;

	if(bounding.x < 0){
		bounding.height += bounding.x; bounding.x=0;
	}
	if(bounding.y < 0){
		bounding.width += bounding.y; bounding.y=0;
	}
	if(bounding.x + bounding.width > src.cols){
		bounding.width = src.cols-bounding.x;
	}
	if(bounding.y+bounding.height > src.rows){
		bounding.height = src.rows-bounding.y;
	}

	src = src(bounding);
	gray = gray(bounding);
	hsv = hsv(bounding);
	bgr = bgr(bounding);


	vector<Vec4i> horz, vert;
	detectVertHorzLines(bgr, horz, vert, 2, 2);
//	LOGD("Time consumed until detected lines: %d", getMilliSpan(t));

	double angle = getAverageAngle(horz);
	rotate(bgr, bgr, angle);
	imshow("bgr", bgr);

	getIntersections(intersections, horz, vert);
//	getIntersections_FAST(intersections, bgr);
//	LOGD("Time consumend until all intersections found: %d", getMilliSpan(t));

//	globEval -> checkIntersectionCorrectness(intersections, bounding.x, bounding.y);

	detectPieces(hsv, darkCircles, lightCircles);
//	LOGD("Time consumed until found circles: %d", getMilliSpan(t));

	for (auto c : darkCircles) {
		intersections.push_back(Point2f(c.x, c.y));
	}
	for (auto c : lightCircles) {
		intersections.push_back(Point2f(c.x, c.y));
	}

	removeDuplicateIntersections(intersections);
//	LOGD("Time consumed until removed duplicates: %d", getMilliSpan(t));

	rotate(intersections, intersections, Point2f(src.cols/2, src.rows/2), angle);

	selectBoardIntersections(src, intersections, selectedIntersections);
//	LOGD("Time consumed until refined all points: %d", getMilliSpan(t));

	fillGaps(selectedIntersections, filledIntersections, src);
//	LOGD("Time consumed until filled gaps: %d", getMilliSpan(t));

//	char pieces[81];
//	getColors(filledIntersections, pieces, hsv);
//
//	for(int i=8; i>=0; i--){
//		for(int j=8; j>=0; j--){
//			cout << to_string(pieces[j*9+i]) << "\t" ;
//		}
//		cout << endl;
//	}

	rotate(intersections, intersections, Point2f(src.cols/2, src.rows/2), angle*-1);
	rotate(selectedIntersections, selectedIntersections, Point2f(src.cols/2, src.rows/2), angle*-1);
	rotate(filledIntersections, filledIntersections, Point2f(src.cols/2, src.rows/2), angle*-1);

	for(auto &i : filledIntersections){
		i.x+=bounding.x; i.y+=bounding.y;
	}
	for(auto &i : selectedIntersections){
		i.x+=bounding.x; i.y+=bounding.y;
	}
	for(auto &i : intersections){
		i.x+=bounding.x; i.y+=bounding.y;
	}
	for(auto &i : darkCircles){
		i.x+=bounding.x; i.y+=bounding.y;
	}
	for(auto &i : lightCircles){
		i.x+=bounding.x; i.y+=bounding.y;
	}
}


void loadAndProcessImage(char *filename) {
	RNG rng(12345);
	Mat4f src;

	if (strcmp(filename + strlen(filename) - 4, ".yml") == 0) {
		FileStorage fs(filename, FileStorage::READ);

		fs["matrix"] >> src;

		fs.release();
	} else if (strcmp(filename + strlen(filename) - 4, ".png") == 0) {
		//load source image and store "as is" (rgb or bgr?) with alpha
		src = imread(filename, -1);
		src.convertTo(src, CV_RGBA2BGRA);
	} else {
		return;
	}

	cvtColor(src, src, COLOR_RGBA2BGRA);

//	LOGD("src is a %s", type2str(src.type()).c_str());

	vector<Point2f> selectedIntersections, intersections, filledIntersections;
	vector<Point3f> darkCircles, lightCircles;

	Evaluater eval(filename);
	globEval = &eval;

	detect(src, intersections, selectedIntersections, filledIntersections, darkCircles, lightCircles);

	//paint the points onto another image
	Mat grayDisplay, colorDisplay;
	src.convertTo(grayDisplay, CV_8UC3);
	src.convertTo(colorDisplay, CV_8UC3);
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
		circle(grayDisplay, p, 5, Scalar(0, 255, 255), 5, 8);
	}
	for (auto p : intersections) {
		circle(grayDisplay, p, 5, Scalar(180, 180, 180), 2, 8);
	}

	circle(grayDisplay, Point2f(src.cols / 2, src.rows / 2), 5, Scalar(0, 0, 255), 5, 8);
	circle(colorDisplay, Point2f(src.cols / 2, src.rows / 2), 5, Scalar(0, 0, 255), 5, 8);

	for (auto p : filledIntersections) {
		circle(grayDisplay, p, 8, Scalar(0, 0, 255), 1, 4);
	}

	eval.setImage(colorDisplay);
//	eval.checkIntersectionCorrectness(intersections);
//	eval.checkOverallCorrectness(filledIntersections);

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
			jlong java_filledIntersections, jlong java_darkCircles, jlong java_lightCircles) {

		vector<Point2f> selectedIntersections, intersections, filledIntersections;
		vector<Point3f> darkCircles, lightCircles;

		detect(*(Mat*) src, intersections, selectedIntersections, filledIntersections, darkCircles, lightCircles);

//		LOGD("outside intersectionsCount: %d", filledIntersections.size());
//		LOGD("outside selectedIntersectionsCount: %d", selectedIntersections.size());
		vector_Point2f_to_Mat(selectedIntersections, *((Mat*)java_selectedIntersections));
		vector_Point2f_to_Mat(intersections, *((Mat*)java_intersections));
		vector_Point2f_to_Mat(filledIntersections, *((Mat*)java_filledIntersections));
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
