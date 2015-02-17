#include <iostream>

#include "lineDetection.h"

using namespace std;
using namespace cv;

#define detectVertHorzLines_LSD detectVertHorzLines

//calculates the distance of the point P(x,y) from the line l
inline double distance(Vec4i l, long x, long y){
	return abs((l[3]-l[1])*x - (l[2]-l[0])*y + l[2]*l[1] - l[3]*l[0]) / sqrt((l[3]-l[1])*(l[3]-l[1])+(l[2]-l[0])*(l[2]-l[0]));
}

void mergeNearbyLines(vector<Vec4i> &horz, vector<Vec4i> &vert){
	int prev;
		do{
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

					if(distance1 < 5 && distance2 < 5 && (distance3 < 15 || distance4 < 15
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

					if(distance1 < 10 && distance2 < 10 && (distance3 < 15 || distance4 < 15
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
		}while(prev!=horz.size()+vert.size());

		vector<Vec4i> horzLong, vertLong;
		for(auto l :horz){
			if(norm(Vec2i(l[0],l[1])-Vec2i(l[2],l[3])) > 50)
				horzLong.push_back(l);
		}
		for(auto l :vert){
			if(norm(Vec2i(l[0],l[1])-Vec2i(l[2],l[3])) > 50)
				vertLong.push_back(l);
		}
		horz = horzLong;
		vert = vertLong;
}

void detectVertHorzLines_LSD (Mat &img, vector<Vec4i> &horz, vector<Vec4i> &vert,
		float horzThreshhold, float vertThreshhold) {
	Mat dst;
	vector<Vec4i> lines;

	Canny(img, dst, 50, 200, 3);
//
//	HoughLinesP(dst, lines, 1, CV_PI / 180, 40, 70, 10);

	Ptr<LineSegmentDetector> lsd = createLineSegmentDetector();
	lsd->detect(dst, lines);

//	lsd->drawSegments(img, lines);

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

	cout << "horz" << horz.size() << endl;
	cout << "vert" << vert.size() << endl;

	mergeNearbyLines(horz, vert);

	cout << "horz" << horz.size() << endl;
	cout << "vert" << vert.size() << endl;

//	RNG rng(12345);
//	for(auto l : horz){
//		line(img, Point(l[0],l[1]), Point(l[2],l[3]), Scalar(rng.next(),rng.next(),rng.next()), 1);
//	}
//	for(auto l : vert){
//		line(img, Point(l[0],l[1]), Point(l[2],l[3]), Scalar(rng.next(),rng.next(),rng.next()), 1);
//	}

	vector<Vec4i> all;
	all.insert(all.end(), vert.begin(), vert.end());
	all.insert(all.end(), horz.begin(), horz.end());
	//lsd->drawSegments(img, all);

	return;
	//lsd->drawSegments(img, vert);
}

double getAverageAngle(vector<Vec4i> lines) {
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

void detectVertHorzLines_HOUGH (Mat &img, vector<Vec4i> &horz, vector<Vec4i> &vert,
		float horzThreshhold, float vertThreshhold) {
	Mat dst;
	vector<Vec4i> lines;



	Canny(img, dst, 50, 200, 3);

	HoughLinesP(dst, lines, 1, CV_PI / 180, 40, 70, 10);

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
