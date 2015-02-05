#include "lineDetection.h"

using namespace std;
using namespace cv;


void detectVertHorzLines(Mat &img, vector<Vec4i> &horz, vector<Vec4i> &vert,
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
