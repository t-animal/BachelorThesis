#include <iostream>

#include "lineDetection.h"
#include "backported/lsd.hpp"
#include "evaluation.h"

using namespace std;
using namespace cv;


void LineDetector::mergeNearbyLines(vector<Vec4i> &horz, vector<Vec4i> &vert){
	unsigned int prev;
		do {
			prev = horz.size()+vert.size();
			vector<Vec4i> horzStitched;
			for(Vec4i &l1 : horz){
				//normalisiere die richtung der linien nach rechts
				if(l1[0] > l1[2]){
					int tmp1 = l1[0];
					int tmp2 = l1[1];
					l1[0] = l1[2];
					l1[1] = l1[3];
					l1[2] = tmp1;
					l1[3] = tmp2;
				}
			}
			for(Vec4i &l1 : horz){
				Vec2i a1 = Vec2i(l1[0], l1[1]);
				Vec2i a2 = Vec2i(l1[2], l1[3]);

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

					if(distance1 < 15 && distance2 < 15 && (distance3 < 50 || distance4 < 50
							|| (a1[0] < b1[0] && b1[0] < a2[0]) ||
							(a1[0] < b2[0] && b2[0] < a2[0])
					)){
						//setze die punkte auf die jeweils aeusseren
						if(l2[0] < l1[0]){
							l1[0] = l2[0];
							l1[1] = l2[1];
						}
						if(l2[2] > l1[2]){
							l1[2] = l2[2];
							l1[3] = l2[3];
						}
						l2[0] = -1;
					}
				}
				if(l1[0] != a1[0] || l1[1] != a1[1] || l1[2] != a2[0] || l1[3] != a2[1])
					horzStitched.push_back(l1);
			}

			vector<Vec4i> vertStitched;
			for(Vec4i &l1 : vert){
				//normalisiere die richtung der linien nach unten
				if(l1[1] > l1[3]){
					int tmp1 = l1[0];
					int tmp2 = l1[1];
					l1[0] = l1[2];
					l1[1] = l1[3];
					l1[2] = tmp1;
					l1[3] = tmp2;
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

					if(distance1 < 15 && distance2 < 15 && (distance3 < 50 || distance4 < 50
							|| (a1[1] < b1[1] && b1[1] < a2[1]) ||
							(a1[1] < b2[1] && b2[1] < a2[1]))){
						//setze die punkte auf die jeweils aeusseren
						if(l2[1] < l1[1]){
							l1[0] = l2[0];
							l1[1] = l2[1];
						}
						if(l2[3] > l1[3]){
							l1[2] = l2[2];
							l1[3] = l2[3];
						}
						l2[0] = -1;
					}
				}
				if(l1[0] != a1[0] || l1[1] != a1[1] || l1[2] != a2[0] || l1[3] != a2[1])
					vertStitched.push_back(l1);
			}
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
		}while(prev != horz.size()+vert.size());

//		vector<Vec4i> horzLong, vertLong;
//		for(auto l :horz){
//			if(norm(Vec2i(l[0],l[1])-Vec2i(l[2],l[3])) > 50)
//				horzLong.push_back(l);
//		}
//		for(auto l :vert){
//			if(norm(Vec2i(l[0],l[1])-Vec2i(l[2],l[3])) > 50)
//				vertLong.push_back(l);
//		}
//		horz = horzLong;
//		vert = vertLong;
}

void LineDetector::detectVertHorzLines_LSD (Mat &img, vector<Vec4i> &horz, vector<Vec4i> &vert,
		float horzThreshhold, float vertThreshhold) {
	Mat dst;
	vector<Vec4i> lines;

	Mat first, length, parallel, remain;
	img.copyTo(first);
	img.copyTo(length);
	img.copyTo(parallel);
	img.copyTo(remain);

	Canny(img, dst, 50, 200, 3);

	Ptr<LineSegmentDetector> lsd = createLineSegmentDetector(LSD_REFINE_ADV, 0.6, 1.5, 2.0, 40, 0, 0.2, 1024);
	lsd->detect(dst, lines);

	cout << lines.size() << endl;
	RNG rng(10);
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		line(first, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(rng.next(), rng.next(), rng.next()), 2);
	}
	imshow("first", first);

	//Keep only line segments longer than some threshold
	vector<Vec4i> keep;
	rng = rng(10);
	for(auto l:lines){
		if(norm(Point(l[0],l[1])-Point(l[2], l[3])) > 30)
			keep.push_back(l);
		else
			line(img, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(120,120,120), 1);
	}
	lines = keep;


	cout << lines.size() << endl;
	rng = rng(10);
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		line(length, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(rng.next(), rng.next(), rng.next()), 2);
	}
	imshow("length", length);

	int parallels[lines.size()];
	int i=0;
	for(auto l1 : lines){
		parallels[i] = 0;
		for(auto l2 : lines){

			if(l1 != l2 && distance(l1, l2[0], l2[1]) < 3  // klein genuger abstand
					&& abs((float)(l1[3]-l1[1]) / (l1[2]-l1[0]) - (float)(l2[3]-l2[1]) / (l2[2]-l2[0])) < 0.15){
				parallels[i]++;
			}
		}
		i++;
	}
	i=0;
	keep.clear();
	for(auto l:lines){
		if(parallels[i] >= 1){
			keep.push_back(l);
		}
		i++;
	}
	lines = keep;

	cout << lines.size() << endl;
	rng = rng(10);
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		line(parallel, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(rng.next(), rng.next(), rng.next()), 2);
	}
	imshow("paralle", parallel);

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

		//cout << height << "||" <<width << "||" << height/width << "||" << width/height <<endl;
	}

	mergeNearbyLines(horz, vert);

	vector<Vec4i> all;
	all.insert(all.end(), vert.begin(), vert.end());
	all.insert(all.end(), horz.begin(), horz.end());
	//lsd->drawSegments(img, all);

	cout << all.size() << endl;
	rng = rng(10);
	for (size_t i = 0; i < all.size(); i++) {
		Vec4i l = all[i];
		line(remain, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(rng.next(), rng.next(), rng.next()), 2);
	}
	imshow("remain", remain);

	return;
	//lsd->drawSegments(img, vert);
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
		//TODO: assert height !=0
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

void LineDetector::detectVertHorzLines_HOUGH (Mat &img, vector<Vec4i> &horz, vector<Vec4i> &vert,
		float horzThreshhold, float vertThreshhold) {
	Mat dst;
	vector<Vec4i> lines;

// Parameters from: currentTime:2015-04-13 20:29:05.569829

	int kernelSize = Evaluater::conf("LINES_HOUGH_GAUSSKERNEL", 3L);
	double sigma = Evaluater::conf("LINES_HOUGH_GAUSSSIGMA", 2.);
	double cannyThresh1 = Evaluater::conf("LINES_HOUGH_CANNYTHRESH1", 50.);
	double cannyThresh2 = Evaluater::conf("LINES_HOUGH_CANNYTHRESH2", 200.);
	int aperture = Evaluater::conf("LINES_HOUGH_CANNYAPERTURE", 3L);

	int angleResolution = Evaluater::conf("LINES_HOUGH_ANGLERES", 180L);
	int houghThresh = Evaluater::conf("LINES_HOUGH_HOUGHTHRESH", 55L);
	int minLength = Evaluater::conf("LINES_HOUGH_HOUGHMINLENGTH", 55.);
	int maxGap = Evaluater::conf("LINES_HOUGH_HOUGHMAXGAP", 5.);

	GaussianBlur(img, dst, Size(kernelSize, kernelSize), sigma);
	Canny(dst, dst, cannyThresh1, cannyThresh2, aperture);

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

		//cout << height << "||" <<width << "||" << height/width << "||" << width/height <<endl;
	}
}
