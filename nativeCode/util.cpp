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

bool sortFunction(pair<double, Point2f> a, pair<double, Point2f> b) {
	return a.first < b.first;
}

void rotate(Mat& src, Mat& dst, double angle) {
	int len = max(src.cols, src.rows);
	Point2f pt(len / 2., len / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);

	warpAffine(src, dst, r, cv::Size(len, len));
}


void rotate(vector<Point2f>& src, vector<Point2f>& dst, Point2f center, double angle) {
	Mat r = getRotationMatrix2D(center, angle, 1.0);

	transform(src, dst, r);
}

string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:
		r = "8U";
		break;
	case CV_8S:
		r = "8S";
		break;
	case CV_16U:
		r = "16U";
		break;
	case CV_16S:
		r = "16S";
		break;
	case CV_32S:
		r = "32S";
		break;
	case CV_32F:
		r = "32F";
		break;
	case CV_64F:
		r = "64F";
		break;
	default:
		r = "User";
		break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}


void drawHistogram(const Mat &src, Mat &dst) {
	dst = src.clone();

	Mat hist;
	int channels[] = { 0 };
	float range[] = { 0, 255 };
	const float* ranges[] = { range };
	int histsize[] = { 255 };
	calcHist(&src, 1, channels, Mat(), hist, 1, histsize, ranges, true, false);

	int bin_w = cvRound((double) src.cols / 255);
	normalize(hist, hist, 0, src.rows, NORM_MINMAX, -1, Mat());
	for (int i = 1; i < 255; i++) {
		line(dst, Point(bin_w * (i - 1), src.rows - cvRound(hist.at<float>(i - 1))),
				Point(bin_w * i, src.rows - cvRound(hist.at<float>(i))), Scalar(255, 255, 255), 2, 8, 0);
	}
}
