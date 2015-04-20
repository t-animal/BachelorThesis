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
			object.push_back(Point2f((offset-i)*KPDIST+400, (offset-j)*KPDIST+240));
		}
	}
}

void GapsFiller::generateCorrespondingKeypoints(vector<Point2f> &keypoints, vector<Point2f> &intersections, Point2f &mp){
	Mat disp = Mat::zeros(Size(mp.x*2, mp.y*2), CV_8UC1);
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
		circle(disp, i, 5, Scalar(255));
		circle(disp, mp, 5, Scalar(120));
		line(disp, Point2f(0, mp.y), Point2f(mp.x*2, mp.y), Scalar(120));
		line(disp, Point2f(mp.x, 0), Point2f(mp.x, mp.y*2), Scalar(120));
//		imshow("disp", disp);
//		waitKey();
		if(i.x - lastX < 0){
//			cout << "NEW LINE ====" << endl;
//			cout << "current location: left of center: " << colsLeftOfCenter << " rowsAboveCenter: " << rowsAboveCenter<< endl;

			//new line
			if(lastY < mp.y)
				rowsAboveCenter++;

			//insert additional lines, if there's a line without keypoints
			while(lastY + averageDistance *1.5 < i.y){
				if(lastY + averageDistance * 1.5 < mp.y)
					rowsAboveCenter++;
				row++;
				lastY += averageDistance;
			}

			lastX = i.x;
			row++;
			col = 0;

			if(i.x < smallestX - averageDistance*0.5){
				//we have an outlier to the left => shift all others one to the right
				//TODO: support outliers by multiple averageDistances
//				cout << " outlieroffset" << (smallestX-i.x);
				int outlierCount = round((smallestX-i.x)/averageDistance);
//				cout << " outliercount " << outlierCount << endl;
				for(Point2f &kp : keypoints){
					kp.x += KPDIST * outlierCount;
				}
				smallestX = i.x;
				colsLeftOfCenter += outlierCount;
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

		if(i.x < mp.x - 0.5 * averageDistance && firstLine)
			colsLeftOfCenter++;
		else if(i.x > mp.x && i == intersections[0]){
			//wenn erster Stein in erster Zeile rechts vom Mittelpunkt liegt, diesen als negativ kennzeichen fuer spaetere verschiebung
//			cout << "correcting first line" << (mp.x-i.x) << " " << averageDistance << endl;
			colsLeftOfCenter = round((mp.x-i.x)/averageDistance);
		}

		col++;
	}
	row++;

//	cout << "Left of center:" << colsLeftOfCenter << " above center:" << rowsAboveCenter << endl;

	for(Point2f &kp : keypoints){
		kp.y -= (rowsAboveCenter-1)*KPDIST;
		kp.x -= colsLeftOfCenter*KPDIST;
	}
}

void GapsFiller::fillGaps(vector<Point2f> intersections, vector<Point2f> &filledIntersections, Mat &src){
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
