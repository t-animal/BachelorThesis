#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <iostream>
#include <string>

#include "gapsFilling.h"
#include "util.h"

using namespace std;
using namespace cv;

#define KPDIST 58.613

void generateReferenceKeypoints(vector<Point2f> &object, int squareLength){
	int offset = (squareLength-1)/2;
	for(int i=0; i<squareLength; i++){
		for(int j=0; j<squareLength; j++){
			object.push_back(Point2f((offset-i)*KPDIST+400, (offset-j)*KPDIST+240));
		}
	}
}

void generateCorrespondingKeypoints(vector<Point2f> &keypoints, vector<Point2f> &intersections, Point2f &mp){
	//estimate average distance between keypoints
	float averageDistance = 0;
	int count = 0;
	float lastX = -1, lastY = -1;
	for(auto i : intersections){
		if(lastX == -1){
			lastX = i.x;
			continue;
		}

		if(i.x-lastX >= 0){
			averageDistance += i.x-lastX;
			count++;
		}

		lastX = i.x;
	}

	assert(count != 0);
	averageDistance /= count;

	lastX = intersections[0].x;
	lastY = intersections[0].y;
	int smallestX = intersections[0].x;

	bool firstLine = true;
	int col=0, row=0;
	int rowsAboveCenter=0, colsLeftOfCenter=0;
	for(auto i: intersections){
		if(i.x - lastX < 0){
			//new line
			if(lastY < mp.y)
				rowsAboveCenter++;

			lastX = i.x;
			row++;
			col = 0;

			if(i.x < smallestX - averageDistance*0.5){
				//we have an outlier to the left => shift all others one to the right
				//TODO: support outliers by multiple averageDistances
				for(Point2f &kp : keypoints){
					kp.x += KPDIST;
				}
				smallestX = i.x;
				colsLeftOfCenter++;
			}else if(i.x > smallestX + averageDistance*0.5){
				//we have a missing intersection at the beginning of the line
				while(smallestX + col * averageDistance < i.x){
					col++;//todo rechnerisch bestimmen
				}
				col--;
			}

			keypoints.push_back(Point2f(col*KPDIST+400, row*KPDIST+240));//todo 400 und 240 durch mittelpunktskoordinaten ersetzen

			col++;
			firstLine = false;
			continue;
		}

		while(i.x - lastX > averageDistance*1.15){
			//"skip" one keypoint
			lastX += averageDistance;
			if(lastX < mp.x - 0.5*averageDistance && firstLine)
				colsLeftOfCenter++;

			col++;
		}

		keypoints.push_back(Point2f(col*KPDIST+400, row*KPDIST+240));

		lastX = i.x;
		lastY = i.y;

		if(i.x < mp.x - 0.5*averageDistance && firstLine)
			colsLeftOfCenter++;

		col++;
	}
	row++;

//	cout << "Left of center:" << colsLeftOfCenter << " above center:" << rowsAboveCenter << endl;

	for(Point2f &kp : keypoints){
		kp.y -= (rowsAboveCenter-1)*KPDIST;
		kp.x -= colsLeftOfCenter*KPDIST;
	}
}

void fillGaps(vector<Point2f> intersections, vector<Point2f> &filledIntersections, Mat &src){
	Point2f center(src.cols/2, src.rows/2);

	vector<Point2f> object, correspondingKeypoints;
	generateCorrespondingKeypoints(object, intersections, center);
	generateReferenceKeypoints(filledIntersections, 9);

	if(intersections.size() < 4 || object.size() != intersections.size()){
//		LOGD("homography detection impossible: object: %d, intersections: %d", object.size(), intersections.size());
	}else{
		Mat H = findHomography(object, intersections, RANSAC, 5);
		perspectiveTransform(filledIntersections, filledIntersections, H);
		perspectiveTransform(object, object, H);
	}
}
