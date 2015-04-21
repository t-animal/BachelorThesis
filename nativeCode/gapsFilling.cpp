#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>

#include "gapsFilling.h"
#include "util.h"

using namespace std;
using namespace cv;

#define KPDIST 58.613

void GapsFiller::generateReferenceKeypoints(vector<Point2f> &object, int squareLength){
	int offset = (squareLength-1)/2;
	for(int i=0; i<squareLength; i++){
		for(int j=0; j<squareLength; j++){
			object.push_back(Point2f((offset-i)*KPDIST+center.x, (offset-j)*KPDIST+center.y));
		}
	}
}

void GapsFiller::generateCorrespondingKeypoints(vector<Point2f> &keypoints, vector<Point2f> &intersections){
	//estimate average distance between keypoints
	float averageDistance = 0;
	int count = 0;
	float lastX = -1, lastY = -1;
	vector<float> distances(keypoints.size());
	for(auto i : intersections){
		if(lastX == -1){
			lastX = i.x;
			continue;
		}

		if(i.x-lastX >= 0){
			averageDistance += i.x-lastX;
			distances.push_back(i.x-lastX);
			count++;
		}

		lastX = i.x;
	}

	sort(distances.begin(), distances.end());
	averageDistance = distances[distances.size()/2];

//	assert(count != 0);
//	averageDistance /= count;

	lastX = intersections[0].x;
	lastY = intersections[0].y;
	int smallestX = intersections[0].x;

	bool firstLine = true;
	int col=0, row=0;
	int rowsAboveCenter=0, colsLeftOfCenter=0;
	for(auto i: intersections){
		if(i.x - lastX < 0){
			//new line
			if(lastY < center.y)
				rowsAboveCenter++;

			//insert additional lines, if there's a line without keypoints
			while(lastY + averageDistance *1.5 < i.y){
				if(lastY + averageDistance * 1.5 < center.y)
					rowsAboveCenter++;
				row++;
				lastY += averageDistance;
			}

			lastX = i.x;
			row++;
			col = 0;

			//if we have an outlier to the left => shift all others one to the right
			if(i.x < smallestX - averageDistance*0.5){
				int outlierCount = round((smallestX-i.x)/averageDistance);
				for(Point2f &kp : keypoints){
					kp.x += KPDIST * outlierCount;
				}
				smallestX = i.x;
				colsLeftOfCenter += outlierCount;
			}

			//if we have a missing intersection at the beginning of the line => skip columns
			if(i.x > smallestX + averageDistance*0.5){
				while(smallestX + col * averageDistance < i.x){
					col++;//todo rechnerisch bestimmen
				}
				col--;
			}

			keypoints.push_back(Point2f(col*KPDIST+center.x, row*KPDIST+center.y));

			col++;
			firstLine = false;
			continue;
		}

		//if there are keypoints missing in the middle of the line => skip colums
		while(i.x - lastX > averageDistance*1.15){
			lastX += averageDistance;
			if(lastX < center.x - 0.5*averageDistance && firstLine)
				colsLeftOfCenter++;

			col++;
		}

		keypoints.push_back(Point2f(col*KPDIST+center.x, row*KPDIST+center.y));

		lastX = i.x;
		lastY = i.y;

		if(i.x < center.x - 0.5 * averageDistance && firstLine)
			colsLeftOfCenter++;

		//first piece of first line is right of center => mark negative because we're gonna shift it right later
		if(i.x > center.x && i == intersections[0])
			colsLeftOfCenter = round((center.x-i.x)/averageDistance);

		col++;
	}
	row++;

	for(Point2f &kp : keypoints){
		kp.y -= (rowsAboveCenter-1)*KPDIST;
		kp.x -= colsLeftOfCenter*KPDIST;
	}
}

void GapsFiller::fillGaps(vector<Point2f> intersections, vector<Point2f> &filledIntersections){
	vector<Point2f> object, correspondingKeypoints;
	generateCorrespondingKeypoints(object, intersections);
	generateReferenceKeypoints(filledIntersections, 9);


	if(intersections.size() < 4 || object.size() != intersections.size()){
//		LOGD("homography detection impossible: object: %d, intersections: %d", object.size(), intersections.size());
	}else{
		Mat H = findHomography(object, intersections, RANSAC, 5);
		perspectiveTransform(filledIntersections, filledIntersections, H);
		perspectiveTransform(object, object, H);
	}
}
