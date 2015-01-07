#include "intersectionDetection.h"

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
