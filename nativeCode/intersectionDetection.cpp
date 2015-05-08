#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "intersectionDetection.h"
#include "evaluation.h"

using namespace std;
using namespace cv;

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


bool IntersectionDetector::IsBetween(const double& x0, const double& x, const double& x1) {
	return (x >= x0) && (x <= x1);
}

Point2f IntersectionDetector::computeIntersect(Vec4i a, Vec4i b) {
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


void IntersectionDetector::getIntersections(vector<Point2f> &intersections, int maxOffset) {
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

	this->intersections = &intersections;
}

void IntersectionDetector::getIntersections_FAST(vector<Point2f> &intersections){
	vector<KeyPoint> keypoints;
	Mat dst;
	cvtColor(src, dst, COLOR_BGR2GRAY);
	//values from currentTime: '2015-05-02 22:02:06.525161'
	int kernelSize = Evaluater::conf("INTERSECT_FAST_GAUSSKERNEL", 3L);
	int sigma = Evaluater::conf("INTERSECT_FAST_GAUSSSIGMA", 3L);
	int threshold = Evaluater::conf("INTERSECT_FAST_THRESHOLD", 13L);
	bool nonMaxSupp = Evaluater::conf("INTERSECT_FAST_NONMAXSUPP", 1L);
	int type = Evaluater::conf("INTERSECT_FAST_TYPE", 0L);

	GaussianBlur(dst, dst, Size(kernelSize, kernelSize), sigma);
	FASTX(dst, keypoints, threshold, nonMaxSupp, type);

	for(KeyPoint kp : keypoints){
		intersections.push_back(kp.pt);
	}

	this->intersections = &intersections;

	removeDuplicateIntersections();
}


void IntersectionDetector::getIntersections_ORB(vector<Point2f> &intersections){
	vector<KeyPoint> keypoints;
	Mat dst;
	cvtColor(src, dst, COLOR_BGR2GRAY);
	GaussianBlur(dst, dst, Size(3,3), 2);

	ORB orb;
	orb(dst, Mat::ones(dst.size(), CV_8U), keypoints, noArray());

	for(KeyPoint kp : keypoints){
		intersections.push_back(kp.pt);
	}

	this->intersections = &intersections;
}

void IntersectionDetector::selectIntersectionsCloud(vector<Point2f> &selectedIntersections){
	double curDist, closestDistance = 9999, secondClosestDistance = 9999;
	Point2f firstIntersection, nextIntersection;
	Point2f center(src.cols/2, src.rows/2);
	for (auto p : *intersections) {
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
			for(auto i : *intersections){
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

void IntersectionDetector::selectBoardIntersections(vector<Point2f> &selectedIntersections, Point2f center){
	Point2f mp(center);
	if(center.x == -1)
			mp = Point2f(src.cols/2, src.rows/2);

	//select all intersections within 150px of the center of the image
	//todo evaluate, ob fester wert oder variabel
	int radius = 85;
	while(selectedIntersections.size() < 20 && radius < 150){
		selectedIntersections.clear();
		radius += 5;

		for(auto i : *intersections){
			if(abs(mp.x-i.x) < radius && abs(mp.y - i.y) < radius){
				selectedIntersections.push_back(i);
			}
		}
	}
	sort(selectedIntersections);
}

void IntersectionDetector::sort(vector<Point2f> &intersections){
	cv::sort(intersections, UpperLeftPointSorter);
}


void IntersectionDetector::removeDuplicateIntersections(){
	for(Point2f &i : *intersections){
		for(Point2f &j : *intersections){
			if(i == j || i.x < 0 || j.x < 0)
				continue;

			if(norm(i-j) < 20){
				j.x = -100;
				j.y = -100;
			}
		}
	}
}
