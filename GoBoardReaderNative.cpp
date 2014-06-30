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

int main(int argc, char** argv) {
	Mat src;
	/// Load source image and convert it to gray
	src = imread(argv[1], 1);

	int t = getMilliCount();

	resize(src, src, Size(), 0.25, 0.25, INTER_LINEAR);

	Mat dst, cdst;
	Canny(src, dst, 50, 200, 3);
	cvtColor(dst, cdst, CV_GRAY2BGR);

	vector<Vec4i> lines, potentialLines;
	HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 50, 10);


	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		Scalar other = Scalar(255, 0, 0);
		Scalar vertical = Scalar(0, 255, 0);
		Scalar horizontal = Scalar(0, 0, 255);

		int height = l[3]-l[1];
		int width = l[2]-l[0];

		bool vert = width==0 || abs(height/width) > 2;
		bool horz = height==0 || abs(width/height) > 2;

		//cout << height << "||" <<width << "||" << height/width << "||" << width/height <<endl;

		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), (vert?vertical:(horz?horizontal:other)), 3, CV_AA);
	}
	cout << "Time consumed:" << getMilliSpan(t) << endl;

	imshow("source", src);
	imshow("detected lines", cdst);

	waitKey();

	return 0;
}
