#include <iostream>

#include "lineDetection.h"
#include "backported/lsd.hpp"
#include "evaluation.h"

using namespace std;
using namespace cv;


void LineDetector::mergeNearbyLines(vector<Vec4i> &horz, vector<Vec4i> &vert){

	for(Vec4i &l1 : horz){
		//normalize line direction to the right
		if(l1[0] > l1[2]){
			int tmp1 = l1[0];
			int tmp2 = l1[1];
			l1[0] = l1[2];
			l1[1] = l1[3];
			l1[2] = tmp1;
			l1[3] = tmp2;
		}
	}

	for(Vec4i &l1 : vert){
		//normalize line direction downwards
		if(l1[1] > l1[3]){
			int tmp1 = l1[0];
			int tmp2 = l1[1];
			l1[0] = l1[2];
			l1[1] = l1[3];
			l1[2] = tmp1;
			l1[3] = tmp2;
		}
	}

	unsigned int prev;
		do {
			prev = horz.size()+vert.size();
			vector<Vec4i> horzStitched;
			vector<Vec4i> vertStitched;

			for(Vec4i &l1 : horz){
				Vec2i a1 = Vec2i(l1[0], l1[1]);
				Vec2i a2 = Vec2i(l1[2], l1[3]);

				//line segment has been stitched already
				if(l1[0] < 0)
					continue;

				for(Vec4i &l2 : horz){
					if(l1 == l2 || l2[0] < 0)
						continue;

					Vec2i b1 = Vec2i(l2[0], l2[1]);
					Vec2i b2 = Vec2i(l2[2], l2[3]);

					double distance1 = distance(l1, l2[0], l2[1]);
					double distance2 = distance(l1, l2[2], l2[3]);

					double distance3 = norm(a2-b1);
					double distance4 = norm(b2-a1);

					//if  l2's endpoints are at most 15px from l1 in orthogonal direction
					//and the biggest distance is at most 50px or they overlap
					if(distance1 < 5 && distance2 < 5 && (distance3 < 30 || distance4 < 30
							|| (a1[0] < b1[0] && b1[0] < a2[0]) ||
							(a1[0] < b2[0] && b2[0] < a2[0])
					)){
						//set the new endpoints to the outermost points
						if(l2[0] < l1[0]){
							l1[0] = l2[0];
							l1[1] = l2[1];
						}
						if(l2[2] > l1[2]){
							l1[2] = l2[2];
							l1[3] = l2[3];
						}
						//discard the second line
						l2[0] = -1;
					}
				}
			}

			for(Vec4i &l1 : vert){
				Vec2i a1 = Vec2i(l1[0], l1[1]);
				Vec2i a2 = Vec2i(l1[2], l1[3]);

				if(l1[0] < 0)
					continue;

				for(Vec4i &l2 : vert){
					if(l1 == l2 || l2[0] < 0)
						continue;

					Vec2i b1 = Vec2i(l2[0], l2[1]);
					Vec2i b2 = Vec2i(l2[2], l2[3]);

					double distance1 = distance(l1, l2[0], l2[1]);
					double distance2 = distance(l1, l2[2], l2[3]);

					double distance3 = norm(a2-b1);
					double distance4 = norm(b2-a1);

					//if  l2's endpoints are at most 15px from l1 in orthogonal direction
					//and the biggest distance is at most 50px or they overlap
					if(distance1 < 5 && distance2 < 5 && (distance3 < 30 || distance4 < 30
							|| (a1[1] < b1[1] && b1[1] < a2[1]) ||
							(a1[1] < b2[1] && b2[1] < a2[1]))){
						//set the new endpoints to the outermost points
						if(l2[1] < l1[1]){
							l1[0] = l2[0];
							l1[1] = l2[1];
						}
						if(l2[3] > l1[3]){
							l1[2] = l2[2];
							l1[3] = l2[3];
						}
						//discard the second line
						l2[0] = -1;
					}
				}
			}
			//look at all lines again that were not changed
			//todo: werden linien hier doppelt hinzefuegt (wegen vier zeilen vorher)
			for(auto l:horz){
				if(l[0]>0)
					horzStitched.push_back(l);
			}
			for(auto l:vert){
				if(l[0]>0)
					vertStitched.push_back(l);
			}
			horz = horzStitched;
			vert = vertStitched;

		//do as long as lines have changed
		}while(prev != horz.size()+vert.size());
}

void LineDetector::detectVertHorzLines_LSD (vector<Vec4i> &horz, vector<Vec4i> &vert) {
	Mat dst;
	vector<Vec4i> lines;

#ifdef DEBUG
	Mat first, length, parallel, remain;
	src.copyTo(first);
	src.copyTo(length);
	src.copyTo(parallel);
	src.copyTo(remain);
#endif

	//chosen from currentTime: '2015-05-02 18:02:38.440363'
	int kernelSize = Evaluater::conf("LINES_LSD_GAUSSKERNEL", 7L);
	double gausSigma = Evaluater::conf("LINES_LSD_GAUSSSIGMA", 7.);
	double scale = Evaluater::conf("LINES_LSD_SCALE", 0.4);
	double lsdSigma = Evaluater::conf("LINES_LSD_SIGMA", 1.55);
	double lsdAngleThresh = Evaluater::conf("LINES_LSD_ANGLETHRESH", 7.0); //das hier ist der parameter, der die fp bestimmt
	double lsdDensityThresh = Evaluater::conf("LINES_LSD_DENSITYTHRESH", 0.7);

	GaussianBlur(src, dst, Size(kernelSize, kernelSize), gausSigma);
	Canny(src, dst, 55, 205, 3);

#ifdef DEBUG
	imshow("src", src);
	imshow("dst", dst);
#endif

	Ptr<LineSegmentDetector> lsd = createLineSegmentDetector(LSD_REFINE_NONE, scale, lsdSigma, 2.0, lsdAngleThresh, 0, lsdDensityThresh);
	lsd->detect(dst, lines);

	//Keep only line segments longer than some threshold
	vector<Vec4i> keep;
	for(auto l:lines){
#ifdef DEBUG
		line(first, Point(l[0],l[1]), Point(l[2], l[3]), Scalar(255), 2);
		imshow("first", first);
#endif
		if(norm(Point(l[0],l[1])-Point(l[2], l[3])) > 20){
#ifdef DEBUG
			line(length, Point(l[0],l[1]), Point(l[2], l[3]), Scalar(255), 2);
			imshow("length", length);
#endif
			keep.push_back(l);
		}
	}
	lines = keep;

	//find number of parallel lines in vicinity
	int parallels[lines.size()];
	int i=0;
	for(auto l1 : lines){
		parallels[i] = 0;
		for(auto l2 : lines){

			if(l1 != l2 && distance(l1, l2[0], l2[1]) < 5  // klein genuger abstand
					&& ( abs((float)(l1[3]-l1[1]) / (l1[2]-l1[0]) - (float)(l2[3]-l2[1]) / (l2[2]-l2[0])) < 0.15
						 || abs((float)(l1[2]-l1[0]) / (l1[3]-l1[1]) - (float)(l2[2]-l2[0]) / (l2[3]-l2[1])) < 0.15 )){
				parallels[i]++;
			}
		}
		i++;
	}
	//keep only lines with at least one parallel
	i=0;
	keep.clear();
	for(auto l:lines){
		if(parallels[i] >= 1){
#ifdef DEBUG
			line(parallel, Point(l[0],l[1]), Point(l[2], l[3]), Scalar(255), 2);
			imshow("parallel", parallel);
#endif
			keep.push_back(l);
		}
		i++;
	}
	lines = keep;

	//classify in horizontal and vertical lines
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];

		int height = l[3] - l[1];
		int width = l[2] - l[0];

		bool isVert = width == 0 || abs(height / float(width)) > horzThreshhold;
		bool isHorz = height == 0 || abs(width / float(height)) > vertThreshhold;

		if (isVert) {
			vert.push_back(l);
		} else if (isHorz) {
			horz.push_back(l);
		}
	}

	mergeNearbyLines(horz, vert);


	//Keep only line segments longer than some threshold
	keep.clear();
	for(auto l:horz){
#ifdef DEBUG
		line(first, Point(l[0],l[1]), Point(l[2], l[3]), Scalar(255), 2);
		imshow("first", first);
#endif
		if(norm(Point(l[0],l[1])-Point(l[2], l[3])) > 40){
			keep.push_back(l);
		}
	}
	horz = keep;
	//Keep only line segments longer than some threshold
	keep.clear();
	for(auto l:vert){
#ifdef DEBUG
		line(first, Point(l[0],l[1]), Point(l[2], l[3]), Scalar(255), 2);
		imshow("first", first);
#endif
		if(norm(Point(l[0],l[1])-Point(l[2], l[3])) > 40){
			keep.push_back(l);
		}
	}
	vert = keep;
#ifdef DEBUG
	vector<Vec4i> all;
	all.insert(all.end(), vert.begin(), vert.end());
	all.insert(all.end(), horz.begin(), horz.end());

	for(auto l:all){
		line(remain, Point(l[0],l[1]), Point(l[2], l[3]), Scalar(255), 2);
		imshow("remain", remain);
	}
#endif

	return;
}

double LineDetector::getAverageAngle(vector<Vec4i> lines) {
	double totalAngle = 0;
	for (Vec4i l : lines) {

		double height = l[3] - l[1];
		double width = l[2] - l[0];

		//soll = 90°
		//=> bei kleiner 90 nach rechts rotieren
		//=> bei groesser 90 nach links rotieren
		//rotate macht bei pos. winkel rotation nach links (gg uzs)
		//assert(height != 0);
		double angle = atan(width / height) * 360 / 2 / M_PI;

		if (angle > 0) {
			totalAngle += 90 - angle;
		} else if (angle < 0) {
			totalAngle -= 90 + angle;
		}
	}

	totalAngle /= lines.size();
	return totalAngle;
}

void LineDetector::detectVertHorzLines_HOUGH (vector<Vec4i> &horz, vector<Vec4i> &vert) {
	Mat dst;
	vector<Vec4i> lines;

// Parameters from: currentTime:'2015-05-01 19:06:45.513663'
	int kernelSize = Evaluater::conf("LINES_HOUGH_GAUSSKERNEL", 3L);
	double sigma = Evaluater::conf("LINES_HOUGH_GAUSSSIGMA", 2.);
	double cannyThresh1 = Evaluater::conf("LINES_HOUGH_CANNYTHRESH1", 55.);
	double cannyThresh2 = Evaluater::conf("LINES_HOUGH_CANNYTHRESH2", 205.);
	int aperture = Evaluater::conf("LINES_HOUGH_CANNYAPERTURE", 3L);

	int angleResolution = Evaluater::conf("LINES_HOUGH_ANGLERES", 180L);
	int houghThresh = Evaluater::conf("LINES_HOUGH_HOUGHTHRESH", 50L);
	int minLength = Evaluater::conf("LINES_HOUGH_HOUGHMINLENGTH", 57.);
	int maxGap = Evaluater::conf("LINES_HOUGH_HOUGHMAXGAP", 5.);

	GaussianBlur(src, dst, Size(kernelSize, kernelSize), sigma);
	Canny(dst, dst, cannyThresh1, cannyThresh2, aperture);
	dst.convertTo(dst, CV_8UC1);

	HoughLinesP(dst, lines, 1, CV_PI/angleResolution, houghThresh, minLength, maxGap);
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];

		int height = l[3] - l[1];
		int width = l[2] - l[0];

		bool isVert = width == 0 || abs(height / float(width)) > horzThreshhold;
		bool isHorz = height == 0 || abs(width / float(height)) > vertThreshhold;

		if (isVert) {
			vert.push_back(l);
		} else if (isHorz) {
			horz.push_back(l);
		}
	}
}
