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

double getAverageAngle(vector<Vec4i> lines) {
	double totalAngle = 0;
	for (Vec4i l : lines) {

		double height = l[3] - l[1];
		double width = l[2] - l[0];

		//soll = 90°
		//=> bei kleiner 90 nach rechts rotieren
		//=> bei groesser 90 nach links rotieren
		//rotate macht bei pos. winkel rotation nach links (gg uzs)
		double angle = atan(width / height) * 360 / 2 / M_PI;

		if(angle>0){
			totalAngle += 90-angle;
		}else if(angle<0){
			totalAngle -= 90+angle;
		}
	}

	totalAngle /= lines.size();
	return totalAngle;
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

cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b) {
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];
	int x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];

	if (float d = ((float) (x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))) {
		cv::Point2f pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4)
				- (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4)
				- (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		return pt;
	} else
		return cv::Point2f(-1, -1);
}

/**
 * Rotate an image
 */
void rotate(Mat& src, Mat& dst, double angle) {
	int len = max(src.cols, src.rows);
	Point2f pt(len / 2., len / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);

	warpAffine(src, dst, r, cv::Size(len, len));
}

bool refine(vector<Vec4i> &horz, vector<Vec4i> &vert) {

	vector<Vec4i> horzTmp(horz), vertTmp(vert);

	horz.clear();
	vert.clear();

	for (auto h : horzTmp) {
		int intersectionCount = 0;
		for (auto v : vertTmp) {
			double a, b;
			if (FindIntersection(h[0], h[1], h[2], h[3], v[0], v[1], v[2], v[3],
					a, b))
				intersectionCount++;

			if (intersectionCount > 4) {
				horz.push_back(h);
				break;
			}
		}
	}

	for (auto v : vertTmp) {
		int intersectionCount = 0;
		for (auto h : horzTmp) {
			double a, b;
			if (FindIntersection(h[0], h[1], h[2], h[3], v[0], v[1], v[2], v[3],
					a, b))
				intersectionCount++;

			if (intersectionCount > 4) {
				vert.push_back(v);
				break;
			}
		}
	}

	cout << "Eliminated " << horzTmp.size() - horz.size() << " horizontal and "
			<< vertTmp.size() - vert.size() << " vertical lines" << endl;
	return horzTmp.size() - horz.size() + vertTmp.size() - vert.size() != 0;

}

int main(int argc, char** argv) {
	RNG rng(12345);

	Mat src;
	/// Load source image and convert it to gray
	src = imread(argv[1], 1);
	int t = getMilliCount();

	resize(src, src, Size(), 0.25, 0.25, INTER_LINEAR);

	cout << "Time consumed  until resized:" << getMilliSpan(t) << endl;

	vector<Vec4i> horz, vert;

	detectVertHorzLines(src, horz, vert, 2.7, 2.7);

	cout << "Time consumed until detected lines:" << getMilliSpan(t) << endl;

	Mat cdst;
	Canny(src, cdst, 50, 200, 3);
	cvtColor(cdst, cdst, CV_GRAY2BGR);

	vector<Point2f> intersections;
	for (auto h : horz) {
		for (auto v : vert) {
			Point2f intersection = computeIntersect(h, v);
			intersections.push_back(intersection);
		}
	}

	cout << "Time consumed until got all points:" << getMilliSpan(t) << endl;

	for(int i=0; i<intersections.size(); i++){
		for(int j=0; j<intersections.size(); j++){
			if(i!=j && norm(intersections[i]-intersections[j])<10){
				intersections[j] = Point(-1, -1);
			}
		}
	}

	cout << "Time consumed until refined all points:" << getMilliSpan(t) << endl;

	for(auto p : intersections ){
		circle(src, p, 5, Scalar(rng.next(),rng.next(),rng.next()), 2, 8);
		circle(cdst, p, 5, Scalar(rng.next(),rng.next(),rng.next()), 2, 8);
	}

	for (auto h : horz) {
//		line(cdst, Point(h[0], h[1]), Point(h[2], h[3]),
//				Scalar(rng(255), rng(255), rng(255)), 3, CV_AA);
	}
	for (auto h : vert) {
//		line(cdst, Point(h[0], h[1]), Point(h[2], h[3]),
//				Scalar(rng(255), rng(255), rng(255)), 3, CV_AA);
	}

	while (refine(horz, vert))
		cout << "Time consumed refining:" << getMilliSpan(t) << endl;
	cout << "Time consumed refining:" << getMilliSpan(t) << endl;

	double angle = getAverageAngle(horz);

	cout << "Time consumed until got angle:" << getMilliSpan(t) << endl;

	vector<Point> allPoints;

	for (auto l : vert) {
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 0, 0), 3,
				CV_AA);
		allPoints.push_back(Point(l[0], l[1]));
		allPoints.push_back(Point(l[2], l[3]));
	}
	for (auto l : horz) {
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 255, 0), 3,
				CV_AA);
		allPoints.push_back(Point(l[0], l[1]));
		allPoints.push_back(Point(l[2], l[3]));
	}

	cout << "Rotating by: " << angle << endl;

	rotate(src, src, angle);
	rotate(cdst, cdst, angle);

	Rect bounding = boundingRect(allPoints);

	rectangle(cdst,bounding,Scalar(255,255,255), 3);
	rectangle(src,bounding,Scalar(255,255,255), 3);

	cout << "Time consumed until rotated:" << getMilliSpan(t) << endl;

	cout << "Time consumed total:" << getMilliSpan(t) << endl;

	imshow("source", src);
	imshow("detected lines", cdst);
	//imshow("rotated", foo);

	waitKey();

	return 0;
}
