#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>

#include "gapsFilling.h"
#include "util.h"
#include "intersectionDetection.h"

using namespace std;
using namespace cv;

#define KPDIST 58.613

void GapsFiller::generateReferenceKeypoints(vector<Point2f> &object, int squareLength) {
	int offset = (squareLength - 1) / 2;
	for (int i = 0; i < squareLength; i++) {
		for (int j = 0; j < squareLength; j++) {
			object.push_back(Point2f((offset - i) * KPDIST + center.x, (offset - j) * KPDIST + center.y));
		}
	}
}

void GapsFiller::generateCorrespondingKeypoints(vector<Point2f> &keypoints, vector<Point2f> &intersections) {
	//estimate average distance between keypoints
	float averageDistance = 0;
	int count = 0;
	float lastX = -1, lastY = -1;
	vector<float> distances(keypoints.size());
	for (auto i : intersections) {
		if (lastX == -1) {
			lastX = i.x;
			continue;
		}

		if (i.x - lastX >= 0) {
			averageDistance += i.x - lastX;
			distances.push_back(i.x - lastX);
			count++;
		}

		lastX = i.x;
	}

	sort(distances.begin(), distances.end());
	averageDistance = distances[distances.size() / 2];

//	assert(count != 0);
//	averageDistance /= count;

	lastX = intersections[0].x;
	lastY = intersections[0].y;
	int smallestX = intersections[0].x;
	int col = 0, row = 0;

	Point closestLocation;
	Point2f closestPoint;
	double closestDistance = 99999;
	for (auto i : intersections) {
		if (i.x - lastX < 0) {
			//new line
			row++;
			col = 0;

			//insert additional lines, if there's a line without keypoints
			while (lastY + averageDistance * 1.3 < i.y) {
				lastY += averageDistance;

				for(int i=0; i < 6; i++){
					Point2f newPoint = Point2f(smallestX+i*averageDistance, lastY);
					if(norm(center-newPoint) < closestDistance){
						closestDistance = norm(center-newPoint);
						closestLocation = Point(col, row);
						closestPoint = newPoint;
					}
					circle(disp, newPoint, 8, Scalar(255), 2);
				}

				row++;
			}

			//if we have an outlier to the left => shift all others one to the right
			if (i.x < smallestX - averageDistance * 0.5) {
				int outlierCount = round((smallestX - i.x) / averageDistance);
				for (Point2f &kp : keypoints) {
					kp.x += KPDIST * outlierCount;
				}
				smallestX = i.x;
			}

			//if we have a missing intersection at the beginning of the line => skip columns
			if (i.x > smallestX + averageDistance * 0.5) {
				while (smallestX + col * averageDistance < i.x) {
					col++; //todo rechnerisch bestimmen
				}
				col--;
			}

		}else{

			//if there are keypoints missing in the middle of the line => skip colums
			while (i.x - lastX > averageDistance * 1.5) {
				lastX += averageDistance;

				Point2f newPoint = Point2f(lastX, i.y);
				if(norm(center-newPoint) < closestDistance){
					closestDistance = norm(center-newPoint);
					closestLocation = Point(col, row);
					closestPoint = newPoint;
				}
				circle(disp, newPoint, 8, Scalar(255), 2);

				col++;
			}
		}

		if(norm(center-i) < closestDistance){
			closestDistance = norm(center-i);
			closestLocation = Point(col, row);
			closestPoint = i;
		}

		keypoints.push_back(Point2f(col * KPDIST + center.x, row * KPDIST + center.y));

		circle(disp, i, 8, Scalar(255), 2);

		lastX = i.x;
		lastY = i.y;

		col++;
	}

	circle(disp, center, 2, Scalar(0,0,255), 3);
	for (auto i : intersections) {
		circle(disp, i, 5, Scalar(255,255,255), 3);
	}
	circle(disp,  closestPoint, 5, Scalar(0,0,255), 3);

	int rowsAboveCenter = closestLocation.y;
	int colsLeftOfCenter = closestLocation.x;

	for (Point2f &kp : keypoints) {
		kp.y -= (rowsAboveCenter) * KPDIST;
		kp.x -= colsLeftOfCenter * KPDIST;

		circle(disp, kp, 2, Scalar(0,255,255), 3);
	}


	//cout << "#" << "rowsAboveCenter " << rowsAboveCenter << " colsLeftOfCenter " << colsLeftOfCenter << endl;
}

void GapsFiller::fillGaps(vector<Point2f> intersections, vector<Point2f> &filledIntersections) {
	vector<Point2f> object, correspondingKeypoints;
	generateCorrespondingKeypoints(object, intersections);
	generateReferenceKeypoints(filledIntersections, 9);

	if (intersections.size() < 4 || object.size() != intersections.size()) {
//		LOGD("homography detection impossible: object: %d, intersections: %d", object.size(), intersections.size());
		return;
	} else {
		H = findHomography(object, intersections, RANSAC, 5);
		perspectiveTransform(filledIntersections, filledIntersections, H);
		perspectiveTransform(object, object, H);
	}
}

void GapsFiller::refine(vector<Point2f> intersections, vector<Point2f> &filledIntersections){
	vector<Point2f> selectedIntersections, filledIntersections2, object;
	IntersectionDetector::sort(filledIntersections);
	generateReferenceKeypoints(filledIntersections2, 9);

	Mat disp;
	this->disp.copyTo(disp);

	int row=0, col=0;
	int prevX = intersections[0].x;
	float closestDistance = 999999;
	Point closestLocation(-1,-1);
	Point2f closestPoint;
	for(auto &i : filledIntersections){
		if(i.x<prevX){
			col=0;
			row++;
		}
		bool matched=false;

		if(norm(center-i) < closestDistance){
			closestDistance = norm(center-i);
			closestLocation = Point(col, row);
			closestPoint = i;
		}

		for(auto j : intersections){
			if(norm(i-j) < 15){
				i.x = j.x;
				i.y = j.y;
				selectedIntersections.push_back(j);
				object.push_back(Point2f(col * KPDIST + center.x, row * KPDIST + center.y));

				circle(disp, i, 3, Scalar(0,255,0), 4);

				matched = true;
				break;
			}
		}
		if(!matched){
			circle(disp, i, 2, Scalar(0,0,255), 2);
		}
		col++;
		prevX = i.x;
	}


	circle(disp, closestPoint, 12, Scalar(180,250,255),6);
	circle(disp, center, 12, Scalar(180,250,255),6);
	for(auto j : intersections){
		circle(disp, j, 2, Scalar(255,255,255), 2);
	}

	int rowsAboveCenter = closestLocation.y;
	int colsLeftOfCenter = closestLocation.x;

	for (Point2f &kp : object) {
		kp.y -= (rowsAboveCenter) * KPDIST;
		kp.x -= colsLeftOfCenter * KPDIST;
	}

	if (selectedIntersections.size() < 4 || object.size() != selectedIntersections.size()) {
//		LOGD("homography detection impossible: object: %d, intersections: %d", object.size(), intersections.size());
		return;
	} else {
		H = findHomography(object, selectedIntersections, RANSAC, 5);
		perspectiveTransform(filledIntersections2, filledIntersections2, H);
		perspectiveTransform(object, object, H);
	}


	filledIntersections.clear();
	filledIntersections.insert(filledIntersections.end(), filledIntersections2.begin(), filledIntersections2.end());
//	filledIntersections = filledIntersections2;
}

Mat GapsFiller::getImageTransformationMatrix(){
	Mat H;
	this->H.copyTo(H);
	((double*)H.data)[2] = 0.0;
	((double*)H.data)[5] = 0.0;
	return H;
}
