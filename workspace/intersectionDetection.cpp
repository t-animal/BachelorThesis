#include "intersectionDetection.h"

using namespace std;
using namespace cv;

bool IsBetween(const double& x0, const double& x, const double& x1) {
	return (x >= x0) && (x <= x1);
}

bool FindIntersection(const double& x0, const double& y0, const double& x1,
		const double& y1, const double& a0, const double& b0, const double& a1,
		const double& b1, double& xy, double& ab) {
	// four endpoints are x0, y0 & x1,y1 & a0,b0 & a1,b1
	// returned values xy and ab are the fractional distance along xy and ab
	// and are only defined when the result is true

	bool partial = false;
	double denom = (b0 - b1) * (x0 - x1) - (y0 - y1) * (a0 - a1);
	if (denom == 0) {
		xy = -1;
		ab = -1;
	} else {
		xy = (a0 * (y1 - b1) + a1 * (b0 - y1) + x1 * (b1 - b0)) / denom;
		partial = IsBetween(0, xy, 1);
		if (partial) {
			// no point calculating this unless xy is between 0 & 1
			ab = (y1 * (x0 - a1) + b1 * (x1 - x0) + y0 * (a1 - x1)) / denom;
		}
	}
	if (partial && IsBetween(0, ab, 1)) {
		ab = 1 - ab;
		xy = 1 - xy;
		return true;
	} else
		return false;
}

Point2f computeIntersect(Vec4i a, Vec4i b) {
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];
	int x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];

	if (float d = ((float) (x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))) {
		Point2f pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4)
				- (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4)
				- (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		return pt;
	} else
		return Point2f(-1, -1);
}


void getIntersections(vector<Point2f> &intersections, const vector<Vec4i> &horz,
		const vector<Vec4i> &vert, int maxOffset) {

	for (auto h : horz) {
		for (auto v : vert) {
			Point2f newIntersect = computeIntersect(h, v);

			bool add = true;
			for (auto existingIntersect : intersections) {
				if (norm(existingIntersect - newIntersect) < maxOffset) {
					add = false;
				}
			}
			if(add)
				intersections.push_back(newIntersect);
		}
	}
}

void selectBoardIntersections_old(Mat &src, vector<Point2f> intersections, vector<Point2f> &selectedIntersections){
	double curDist, closestDistance = 9999, secondClosestDistance = 9999;
	Point2f firstIntersection, nextIntersection;
	Point2f center(src.cols/2, src.rows/2);
	for (auto p : intersections) {
		if((curDist = norm(center-p)) < closestDistance){
			nextIntersection = firstIntersection;
			firstIntersection = p;
			secondClosestDistance = closestDistance;
			closestDistance = curDist;
		}else if(curDist < secondClosestDistance){
			secondClosestDistance = curDist;
			nextIntersection = p;
		}
	}

	selectedIntersections.push_back(firstIntersection);
	selectedIntersections.push_back(nextIntersection);
	int curAverageDistance = norm(firstIntersection - nextIntersection);

	unsigned int curLength;
	do {
		curLength = selectedIntersections.size();
		for(auto si : selectedIntersections){
			for(auto i : intersections){
				if(norm(si-i) < curAverageDistance*1.5){
					bool select = true;
					for(auto si2:selectedIntersections){
						if(i == si2){
							select = false;
							//cout << "not select" << endl;
						}
					}
					if(select)
						selectedIntersections.push_back(i);
					if(selectedIntersections.size() == 9*9)
						break;
				}
			}
			if(selectedIntersections.size() == 9*9)
				break;
		}

	}while(curLength != selectedIntersections.size() && selectedIntersections.size() != 9*9);

}

class MiddlePointSorter{
private:
	Point2f mp;

public:
	MiddlePointSorter(Point2f mp){
		this->mp = mp;
	}

	bool operator() (Point2f a, Point2f b){ return norm(Mat(mp-a), NORM_L1) < norm(Mat(mp-b), NORM_L1); }
};

bool UpperLeftPointSorter(Point2f a, Point2f b){
	if(abs(a.y-b.y) < 15){
		return a.x < b.x;
	}else{
		return a.y < b.y;
	}
}

#include <iostream>
void selectBoardIntersections(Mat &src, vector<Point2f> intersections, vector<Point2f> &selectedIntersections){
	Point2f mp(src.cols/2, src.rows/2);
	//sort(intersections, MiddlePointSorter(mp));

	//select all intersections within 150px of the center of the image
	for(auto i : intersections){
		if(abs(mp.x-i.x) < 150 && abs(mp.y - i.y) < 150)
			selectedIntersections.push_back(i);
	}

	sort(selectedIntersections, UpperLeftPointSorter);
}
