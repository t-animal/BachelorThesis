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

void detectVertHorzLines(Mat &img, vector<Vec4i> &horz, vector<Vec4i> &vert, float horzThreshhold = 2, float vertThreshhold = 2){
	Mat dst;
	vector<Vec4i> lines;

	Canny(img, dst, 50, 200, 3);

	HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 50, 10);

	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];

		int height = l[3]-l[1];
		int width = l[2]-l[0];

		bool isVert = width==0 || abs(height/float(width)) > horzThreshhold;
		bool isHorz = height==0 || abs(width/float(height)) > vertThreshhold;

		if(isVert){
			vert.push_back(l);
		}else if(isHorz){
			horz.push_back(l);
		}

		//cout << height << "||" <<width << "||" << height/width << "||" << width/height <<endl;
	}
}

int main(int argc, char** argv) {
	Mat src;
	/// Load source image and convert it to gray
	src = imread(argv[1], 1);
	int t = getMilliCount();

	resize(src, src, Size(), 0.25, 0.25, INTER_LINEAR);

	vector<Vec4i> horz, vert;

	detectVertHorzLines(src,horz, vert, 2.5, 2.5);

	Scalar vertical = Scalar(0, 255, 0);
	Mat cdst;
	Canny(src, cdst, 50, 200, 3);
	cvtColor(cdst, cdst, CV_GRAY2BGR);

	for(auto h : horz){
		line(cdst, Point(h[0], h[1]), Point(h[2], h[3]), Scalar(255, 0, 0), 3, CV_AA);
	}
	for(auto v : vert){
		line(cdst, Point(v[0], v[1]), Point(v[2], v[3]), Scalar(0, 255, 000), 3, CV_AA);
	}

	cout << "Time consumed:" << getMilliSpan(t) << endl;

	imshow("source", src);
	imshow("detected lines", cdst);

	waitKey();

	return 0;
}
