#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timeb.h>

using namespace cv;
using namespace std;

int getMilliCount() {
	timeb tb;
	ftime(&tb);
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}

int getMilliSpan(int nTimeStart) {
	int nSpan = getMilliCount() - nTimeStart;
	if (nSpan < 0)
		nSpan += 0x100000 * 1000;
	return nSpan;
}

void detectVertHorzLines(Mat &img, vector<Vec4i> &horz, vector<Vec4i> &vert,
		float horzThreshhold = 2, float vertThreshhold = 2) {
	Mat dst;
	vector<Vec4i> lines;

	Canny(img, dst, 50, 200, 3);

	HoughLinesP(dst, lines, 1, CV_PI / 180, 40, 70, 10);

	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];

		int height = l[3] - l[1];
		int width = l[2] - l[0];

		bool isVert = width == 0 || abs(height / float(width)) > horzThreshhold;
		bool isHorz = height == 0
				|| abs(width / float(height)) > vertThreshhold;

		if (isVert) {
			vert.push_back(l);
		} else if (isHorz) {
			horz.push_back(l);
		}

		//cout << height << "||" <<width << "||" << height/width << "||" << width/height <<endl;
	}
}

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

int main(int argc, char** argv) {
	Mat src;
	/// Load source image and convert it to gray
	src = imread(argv[1], 1);
	int t = getMilliCount();

	resize(src, src, Size(), 0.25, 0.25, INTER_LINEAR);

	vector<Vec4i> horz, vert;

	detectVertHorzLines(src, horz, vert, 2.7, 2.7);

	Mat cdst;
	Canny(src, cdst, 50, 200, 3);
	cvtColor(cdst, cdst, CV_GRAY2BGR);

	RNG rng(12345);
	vector<Vec4i> horzUsed, vertUsed;
	for (auto h : horz) {
		int intersectionCount = 0;
		for (auto v : vert) {
			double a, b;
			if (FindIntersection(h[0], h[1], h[2], h[3], v[0], v[1], v[2], v[3],
					a, b))
				intersectionCount++;

			if (intersectionCount > 2) {
				line(cdst, Point(h[0], h[1]), Point(h[2], h[3]),
						Scalar(0, 255, 0), 3, CV_AA);
				horzUsed.push_back(h);
				break;
			}
		}
	}
	for (auto v : vert) {
		int intersectionCount = 0;
		for (auto h : horz) {
			double a, b;
			if (FindIntersection(h[0], h[1], h[2], h[3], v[0], v[1], v[2], v[3],
					a, b))
				intersectionCount++;

			if (intersectionCount > 2) {
				line(cdst, Point(v[0], v[1]), Point(v[2], v[3]),
						Scalar(255, 0, 0), 3, CV_AA);
				vertUsed.push_back(v);
				break;
			}
		}
	}

	cout << "Time consumed:" << getMilliSpan(t) << endl;

	imshow("source", src);
	imshow("detected lines", cdst);

	waitKey();

	return 0;
}
