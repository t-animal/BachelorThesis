#include "util.h"

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

bool sortFunction(pair<double, Point2f> a, pair<double, Point2f> b){
	return a.first<b.first;
}

void rotate(Mat& src, Mat& dst, double angle) {
	int len = max(src.cols, src.rows);
	Point2f pt(len / 2., len / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);

	warpAffine(src, dst, r, cv::Size(len, len));
}
