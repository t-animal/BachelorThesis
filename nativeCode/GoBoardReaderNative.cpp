#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <iostream>
#include <iomanip>
#include <stdio.h>

#include "lineDetection.h"
#include "intersectionDetection.h"
#include "util.h"
#include "pieceDetection.h"
#include "gapsFilling.h"

using namespace cv;
using namespace std;

void detect(Mat &src, vector<Point2f> &intersections, vector<Point2f> &selectedIntersections,
		vector<Point2f> &filledIntersections, vector<Point3f> &darkCircles, vector<Point3f> &lightCircles) {
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

	fillGaps(selectedIntersections, filledIntersections, src);
	LOGD("Time consumed until filled gaps: %d", getMilliSpan(t));
}

#ifndef JNI
void checkCorrectness(vector<Point2f> intersections, char* filename, Mat src){
	vector<Point2f> emptyIntersects;
	vector<Point2f> blackPieces;
	vector<Point2f> whitePieces;
	vector<Point2f> allIntersects;

	char annotFilename[strlen(filename)+7];
	strcpy(annotFilename, filename);
	strcpy(&annotFilename[strlen(filename)-4], "_annot.yml");

	FileStorage readStorage(annotFilename, FileStorage::READ);

	readStorage["emptyIntersects"] >> emptyIntersects;
	readStorage["blackPieces"] >> blackPieces;
	readStorage["whitePieces"] >> whitePieces;

	readStorage.release();

	allIntersects.reserve(emptyIntersects.size()+whitePieces.size()+blackPieces.size());
	allIntersects.insert(allIntersects.end(), emptyIntersects.begin(), emptyIntersects.end());
	allIntersects.insert(allIntersects.end(), blackPieces.begin(), blackPieces.end());
	allIntersects.insert(allIntersects.end(), whitePieces.begin(), whitePieces.end());

	int matchedCount=0;
	int unmatchedCount = 0;
	for(auto desired : allIntersects){
		bool matched = false;
		for(auto is : intersections){
			if(norm(desired-is) <= 15){
				matched = true;
				break;
			}
		}
		if(!matched){
			//cout << "Desired intersect " << desired << " has not been matched!" << endl;
			circle(src, desired, 10, Scalar(0, 0, 255), 4);
			unmatchedCount++;
		}else{
			circle(src, desired, 10, Scalar(0, 255, 0), 4);
			matchedCount++;
		}
	}

	if(allIntersects.size() == 0){
		cout << "There's no reference points";
		if(intersections.size() != 0) cout << " but keypoints have been found! == FAIL ==" << endl;
		else cout << " and no keypoints have been found. == SUCCESS == " << endl;
	}else{
		float percentage = matchedCount*100.0/allIntersects.size();
		cout << "Matched " << matchedCount << " out of " << allIntersects.size() << " reference points. (";
		cout << std::setprecision( 3 ) << percentage << "%) ";
		if(percentage >= 99){
			cout << "== SUCCESS ==" << endl;
		}else{
			cout << "== FAIL == " << endl;
		}
	}
}
#endif

void loadAndProcessImage(char *filename) {
	RNG rng(12345);
	Mat4f src;

	if (strcmp(filename+strlen(filename)-4, ".yml") == 0) {
		FileStorage fs(filename, FileStorage::READ);

		fs["matrix"] >> src;

		fs.release();
	} else if(strcmp(filename+strlen(filename)-4, ".png") == 0) {
		//load source image and store "as is" (rgb or bgr?) with alpha
		src = imread(filename, -1);
		src.convertTo(src, CV_RGBA2BGRA);
	}else {
		return;
	}

	cvtColor(src, src, COLOR_RGBA2BGRA);

	LOGD("src is a %s", type2str(src.type()).c_str());

	vector<Point2f> selectedIntersections, intersections, filledIntersections;
	vector<Point3f> darkCircles, lightCircles;

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

	circle(grayDisplay, Point2f(src.cols/2, src.rows/2), 5, Scalar(0, 0, 255), 5, 8);
	circle(colorDisplay, Point2f(src.cols/2, src.rows/2), 5, Scalar(0, 0, 255), 5, 8);

	for(auto p : filledIntersections){
		circle(grayDisplay, p, 8, Scalar(0,0,255), 1, 4);
	}

#ifndef JNI
	checkCorrectness(filledIntersections, filename, colorDisplay);
#endif

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

		LOGD("outside intersectionsCount: %d", filledIntersections.size());
		LOGD("outside selectedIntersectionsCount: %d", selectedIntersections.size());
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
