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
#include "colorDetection.h"
#include "evaluation.h"
#include "boardSegmenter.h"

using namespace cv;
using namespace std;

Evaluater *globEval = NULL;

void detect(Mat &input, vector<Point2f> &intersections, vector<Point2f> &selectedIntersections,
		vector<Point2f> &filledIntersections, vector<Point3f> &darkCircles, vector<Point3f> &lightCircles, char *board,
		Mat_<double> &transformationMatrix, Mat_<Point2f> *prevIntersections=0) {

	if(globEval != NULL) globEval->setStartTime();
//	resize(src, src, Size(), 0.75, 0.75, INTER_LINEAR);
//	globEval->saveStepTime("Resized input");


	Mat gray, hsv, bgr, threshed, src;
	src = input.clone();
	Rect bounding;
	vector<Vec4i> horz, vert;
	Point2f originalCenter(src.cols/2, src.rows/2);
	Size originalSize(src.cols, src.rows);

	warpPerspective(src, src, transformationMatrix, src.size(), INTER_LINEAR | WARP_INVERSE_MAP);
	vector<Point2f> tmp1;
	tmp1.push_back(originalCenter);
	invert(transformationMatrix, transformationMatrix);
	perspectiveTransform(tmp1, tmp1, transformationMatrix);
	originalCenter = tmp1[0];

	cvtColor(src, gray, COLOR_BGR2GRAY);
	cvtColor(src, hsv, COLOR_BGR2HSV);
	src.convertTo(bgr, CV_8UC4);
	gray.convertTo(threshed, CV_8UC1);
	adaptiveThreshold(threshed, threshed, 255, ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 45, 1);
	if(globEval != NULL) globEval->saveStepTime("Created working copies");

	//segment board from background
	BoardSegmenter boardSegmenter(threshed);
	boardSegmenter.calculateBoundingBox(bounding);
	boardSegmenter.segmentImages(src, gray, hsv, bgr, threshed);
	if(globEval != NULL) globEval->saveStepTime("Threshholded image and calculated bounding box");

	Point2f shiftedOrigCenter(originalCenter.x-bounding.x, originalCenter.y-bounding.y);
	Point2f center(src.cols/2, src.rows/2);

	//create pipeline
	LineDetector lineDetector(bgr, 1, 1);
	IntersectionDetector intersectionDetector(vert, horz, bgr);
	PieceDetector pieceDetector(hsv);
	ColorDetector colorDetector(threshed, filledIntersections);

	//detect lines
	lineDetector.detectVertHorzLines_HOUGH(horz, vert);
	if(globEval != NULL) globEval->saveStepTime("Detected all lines");


	//detect intersections in these lines
	intersectionDetector.getIntersections(intersections);
	if(globEval != NULL) globEval->saveStepTime("Found all intersections");


	//if(globEval != NULL) globEval -> checkIntersectionCorrectness(intersections, bounding.x, bounding.y);


	//detect pieces
	pieceDetector.detectPieces(darkCircles, lightCircles);
	if(globEval != NULL) globEval->saveStepTime("Detected all pieces");


	//if(globEval != NULL) globEval -> checkPieceCorrectness(darkCircles, lightCircles, bounding.x, bounding.y);


	for (auto c : darkCircles) {
		intersections.push_back(Point2f(c.x, c.y));
	}
	for (auto c : lightCircles) {
		intersections.push_back(Point2f(c.x, c.y));
	}


	//remove duplicates
	intersectionDetector.removeDuplicateIntersections();
	if(globEval != NULL) globEval->saveStepTime("Removed all duplicates");

	if(intersections.size() == 0){
		LOGD("#ERR: No intersections found, cannot continue");
		return;
	}


	//rotate intersections
	double angle = lineDetector.getAverageAngle(horz);
	rotate(intersections, intersections, center, angle);


	Mat disp(bgr.size(), bgr.type());
	bgr.copyTo(disp);
	rotate(disp, disp, angle);
	vector<Point2f> tmp; tmp.push_back(shiftedOrigCenter);
	rotate(tmp, tmp, center, angle);
	GapsFiller gapsFiller(9, tmp[0], disp);

	//select a few board intersections
	intersectionDetector.selectBoardIntersections(selectedIntersections, tmp[0]);
	if(globEval != NULL) globEval->saveStepTime("Refined all points");

	if(selectedIntersections.size() <= 4){
		LOGD("#ERR: Too few intersections selected, cannot continue");
		return;
	}

	//fill gaps using these and generate complete set of board intersections
	gapsFiller.fillGaps(selectedIntersections, filledIntersections);
	if(globEval != NULL) globEval->saveStepTime("Filled all gaps");

	gapsFiller.refine(intersections, filledIntersections);
	gapsFiller.refine(intersections, filledIntersections);
	gapsFiller.refine(intersections, filledIntersections);
	gapsFiller.refine(intersections, filledIntersections);
	gapsFiller.refine(intersections, filledIntersections);
	gapsFiller.refine(intersections, filledIntersections);
	gapsFiller.refine(intersections, filledIntersections);
	gapsFiller.refine(intersections, filledIntersections);
	if(globEval != NULL) globEval->saveStepTime("Refined filling 8x");

	//check for plausible results, discard if not so
	Point2f lastI = filledIntersections[filledIntersections.size()-1];
	for(auto i : filledIntersections){
		if(i.x < 0 || i.x > originalSize.width || i.y < 0 || i.y > originalSize.height){
			filledIntersections.clear();
			return;
		}
		if(norm(lastI-i) < 5){
			filledIntersections.clear();
			return;
		}
		lastI = i;
	}
	if(globEval != NULL) globEval->saveStepTime("Checked plausability");

	//rotate these back
	rotate(intersections, intersections, center, angle*-1);
	rotate(selectedIntersections, selectedIntersections, center, angle*-1);
	rotate(filledIntersections, filledIntersections, center, angle*-1);

	if(prevIntersections != 0){
		for(int i=0; i<prevIntersections->rows; i++){
			Point2f current = filledIntersections[i];
			Point2f old = boardSegmenter.segmentPoint(prevIntersections->at<Point2f>(0,i));
			filledIntersections[i] = Point2f((current.x*0.75+old.x*0.25), (current.y*0.75+old.y*0.25));
		}
	}


	//get color on the intersections
	uchar pieces[81];
	colorDetector.getColors(pieces);
	if(globEval != NULL) globEval->saveStepTime("Determined all intersections' colors");


	//if(globEval != NULL) globEval->checkColorCorrectness(pieces, filledIntersections, bounding.x, bounding.y);


	//rearrange color values
	for(int i=0; i<9; i++){
		for(int j=0; j<9; j++){
			board[i*9+j] = (char)pieces[80-i-j*9];
		}
	}

	invert(transformationMatrix, transformationMatrix);
	//shift intersections back, save transformation matrix
	boardSegmenter.unsegmentPoints(filledIntersections, selectedIntersections, intersections);
	boardSegmenter.unsegmentPoints(lightCircles, darkCircles);
	perspectiveTransform(filledIntersections, filledIntersections, transformationMatrix);
	perspectiveTransform(selectedIntersections, selectedIntersections, transformationMatrix);
	perspectiveTransform(intersections, intersections, transformationMatrix);
//	perspectiveTransform(lightCircles, lightCircles, transformationMatrix);
//	perspectiveTransform(darkCircles, darkCircles, transformationMatrix);
	transformationMatrix = gapsFiller.getImageTransformationMatrix();

	if(globEval != NULL) globEval->saveStepTime("Finished detection");
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
	char board[81];
	memset(board, 'u', sizeof(board));
	Mat_<double> transformationMatrix = Mat::eye(Size(3,3), CV_64F);

	Evaluater eval(filename);
	globEval = &eval;

	detect(src, intersections, selectedIntersections, filledIntersections, darkCircles, lightCircles, board, transformationMatrix);

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

	imshow("grayImage", grayDisplay);

	eval.setImage(colorDisplay);
	eval.checkOverallCorrectness(board, filledIntersections);

	Mat output(Size(src.cols, src.rows*2), CV_8UC4);
	Mat upperOutput = output(Rect(0, 0, src.cols, src.rows));
	Mat lowerOutput = output(Rect(0, src.rows, src.cols, src.rows));
	upperOutput.setTo(Scalar(80, 80, 80));

	colorDisplay.copyTo(upperOutput);

	lowerOutput.setTo(Scalar(120, 120, 120));
	Mat boardOutput = lowerOutput(Rect((src.cols-src.rows)/2, 20, src.rows-40, src.rows-40));
	Scalar black(0,0,0);
	Scalar white(255,255,255);
	int randAbstand = boardOutput.cols/18;
	for(int i=0; i<9; i++){
		line(boardOutput, Point(randAbstand+boardOutput.cols*i/9, randAbstand), Point(randAbstand+boardOutput.cols*i/9, boardOutput.rows-randAbstand), Scalar(70,70,70), 2);
		line(boardOutput, Point(randAbstand, randAbstand+boardOutput.rows*i/9), Point(boardOutput.cols-randAbstand, randAbstand+boardOutput.rows*i/9), Scalar(70,70,70), 2);
	}
	for(int i=0; i < 81; i++){
		if(board[i]=='0')
			continue;
		int row = i/9;
		int col = i%9;
		int radius = boardOutput.cols/18-10-2;//10=abstand; 2=borderwidth
		circle(boardOutput, Point(boardOutput.cols*col/9+randAbstand, boardOutput.rows*row/9+randAbstand), radius, board[i]=='w'?white:black, 2);
	}
	rectangle(boardOutput, Point(0, 0), Point(boardOutput.cols, boardOutput.cols), Scalar(70,70,70), 2);

	globEval->printStepTimes();

	namedWindow("output", WINDOW_NORMAL);
	imshow("output", output);
//	waitKey();
}

int main(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		loadAndProcessImage(argv[i]);
	}

	return 0;
}

#ifdef USE_JNI

extern "C" {
	JNIEXPORT void JNICALL Java_de_t_1animal_goboardreader_BusinessLogic_detect(
			JNIEnv * jenv, jobject obj, jlong src, jlong java_intersections, jlong java_selectedIntersections,
			jlong java_filledIntersections, jlong java_darkCircles, jlong java_lightCircles, jcharArray java_board,
			jlong java_prevIntersections) {

		vector<Point2f> selectedIntersections, intersections, filledIntersections;
		vector<Point3f> darkCircles, lightCircles;

		char board[81];
		memset(board, 'u', sizeof(board));
		Mat_<double> transformationMatrix = Mat::eye(Size(3,3), CV_64F);

		detect(*(Mat*) src, intersections, selectedIntersections, filledIntersections, darkCircles, lightCircles, board,
				transformationMatrix, (Mat_<Point2f>*) java_prevIntersections);

		jchar *jboard = jenv->GetCharArrayElements(java_board, NULL);
		for(int i=0; i<81; i++){
			jboard[i] = board[i];
		}
		jenv->ReleaseCharArrayElements(java_board, jboard, NULL);


//		LOGD("outside intersectionsCount: %d", filledIntersections.size());
//		LOGD("outside selectedIntersectionsCount: %d", selectedIntersections.size());
		vector_Point2f_to_Mat(selectedIntersections, *((Mat*)java_selectedIntersections));
		vector_Point2f_to_Mat(intersections, *((Mat*)java_intersections));
		vector_Point2f_to_Mat(filledIntersections, *((Mat*)java_filledIntersections));
		vector_Point3f_to_Mat(darkCircles, *((Mat*)java_darkCircles));
		vector_Point3f_to_Mat(lightCircles, *((Mat*)java_lightCircles));
	}

	JNIEXPORT void JNICALL Java_de_t_1animal_goboardreader_BusinessLogic_saveAsYAML(
			JNIEnv *jenv, jobject obj, jlong src, jstring filename) {

		const char *path = jenv->GetStringUTFChars(filename, 0);
		FileStorage fs(path, FileStorage::WRITE);

		fs << "matrix" << (*(Mat*)src);

		fs.release();
		jenv->ReleaseStringUTFChars(filename, path);
	}
}
#endif
