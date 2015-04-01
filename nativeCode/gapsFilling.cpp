#include "gapsFilling.h"

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <iostream>

using namespace std;
using namespace cv;

#define KPDIST 58.613

void generateReferenceKeypoints(vector<Point2f> &object, int squareLength){
	int offset = (squareLength-1)/2;
	for(int i=0; i<squareLength; i++){
		for(int j=0; j<squareLength; j++){
			object.push_back(Point2f((offset-i)*KPDIST, (offset-j)*KPDIST));
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

	cout << "average distance:" << averageDistance << endl;

	//todo: luecken in y-richtung!
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

			if(i.x < smallestX - averageDistance*0.15){
				//we have an outlier to the left => shift all others one to the right
				//TODO: support outliers by multiple averageDistances
				for(Point2f &kp : keypoints){
					kp.x += KPDIST;
				}
				smallestX = i.x;
			}else if(i.x > smallestX + averageDistance*0.15){
				cout << "missing start in next line!";
				//we have a missing intersection at the beginning of the line
				while(smallestX + col * averageDistance < i.x){
					col++;//todo rechnerisch bestimmen
				}
				col--;
			}

			keypoints.push_back(Point2f(col*KPDIST, row*KPDIST));

			col++;
			firstLine = false;
			cout << endl << "1 ";
			continue;
		}

		while(i.x - lastX > averageDistance*1.15){
			//"skip" one keypoint
			lastX += averageDistance;
			if(lastX < mp.x && firstLine)
				colsLeftOfCenter++;

			cout << "0 ";

			col++;
		}

		keypoints.push_back(Point2f(col*KPDIST, row*KPDIST));
		cout << "1 ";

		lastX = i.x;
		lastY = i.y;

		if(i.x < mp.x && firstLine)
			colsLeftOfCenter++;

		col++;
	}
	cout << endl;
	row++;

	cout << "Left of center:" << colsLeftOfCenter << " above center:" << rowsAboveCenter << endl;

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

	if(intersections.size() == 0 || object.size() != intersections.size()){
		cout << "homography detection impossible: object: %d, intersections: %d", object.size(), intersections.size();
	}else{
		Mat H = findHomography(object, intersections, RANSAC, 5);
		perspectiveTransform(filledIntersections, filledIntersections, H);
		perspectiveTransform(object, object, H);
		//warpPerspective(colorDisplay, colorDisplay, H, colorDisplay.size());

		for(auto p : object){
			circle(src, p, 8, Scalar(0,0,180), 2, 8);
		}
	}
}
