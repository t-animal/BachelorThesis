#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <iomanip>

#include "evaluation.h"

using namespace std;
using namespace cv;


Evaluater::Evaluater(const char *filename, Mat &image) : Evaluater(filename){
	this->image = image;
}

Evaluater::Evaluater(const char *filename){
	char annotFilename[strlen(filename) + 7];
	strcpy(annotFilename, filename);
	strcpy(&annotFilename[strlen(filename) - 4], "_annot.yml");

	FileStorage readStorage(annotFilename, FileStorage::READ);

	readStorage["emptyIntersects"] >> emptyIntersects;
	readStorage["blackPieces"] >> blackPieces;
	readStorage["whitePieces"] >> whitePieces;

	readStorage.release();

	allIntersects.reserve(emptyIntersects.size() + whitePieces.size() + blackPieces.size());
	allIntersects.insert(allIntersects.end(), emptyIntersects.begin(), emptyIntersects.end());
	allIntersects.insert(allIntersects.end(), blackPieces.begin(), blackPieces.end());
	allIntersects.insert(allIntersects.end(), whitePieces.begin(), whitePieces.end());
}

void Evaluater::checkIntersectionCorrectness(const vector<Point2f> &intersections){
	vector<Point2f> contour, intersectCopy;
	intersectCopy.reserve(allIntersects.size());
	intersectCopy.insert(intersectCopy.begin(), allIntersects.begin(), allIntersects.end());

	convexHull(allIntersects, contour);

	int matched = 0;
	int insideKeypoints = 0;

	for(Point2f i : intersections){
		if(pointPolygonTest(contour, i, true) >= -15){
			insideKeypoints++;

			for(Point2f &ai: intersectCopy){
				int offset = norm(i-ai);

				if(offset < 15){
					circle(image, ai, 10, Scalar(0, 255, 0), 4);
					ai.x=-10;
					ai.y=-10;

					matched++;
					break;
				}
			}

		}
	}

	cout << endl << matched << " intersections have been correctly found. " << endl
		<< allIntersects.size() - matched << " intersections are missing a keypoint." << endl
		<< "there are " << insideKeypoints - matched << " bogus keypoints." << endl << endl;
}

void Evaluater::checkOverallCorrectness(const vector<Point2f> &intersections) {
	int matchedCount = 0;
	int unmatchedCount = 0;
	for (auto desired : allIntersects) {
		bool matched = false;
		for (auto is : intersections) {
			if (norm(desired - is) <= 15) {
				matched = true;
				break;
			}
		}
		if (!matched) {
			//cout << "Desired intersect " << desired << " has not been matched!" << endl;
			circle(image, desired, 10, Scalar(0, 0, 255), 4);
			unmatchedCount++;
		} else {
			circle(image, desired, 10, Scalar(0, 255, 0), 4);
			matchedCount++;
		}
	}

	if (allIntersects.size() == 0) {
		cout << "There's no reference points";
		if (intersections.size() != 0)
			cout << " but keypoints have been found! == FAIL ==" << endl;
		else
			cout << " and no keypoints have been found. == SUCCESS == " << endl;
	} else {
		float percentage = matchedCount * 100.0 / allIntersects.size();
		cout << "Matched " << matchedCount << " out of " << allIntersects.size() << " reference points. (";
		cout << std::setprecision(3) << percentage << "%) ";
		if (percentage >= 99) {
			cout << "== SUCCESS ==" << endl;
		} else {
			cout << "== FAIL == " << endl;
		}
	}
}
