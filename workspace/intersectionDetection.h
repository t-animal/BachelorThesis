#include <opencv2/core/core.hpp>

bool IsBetween(const double& x0, const double& x, const double& x1);

bool FindIntersection(const double& x0, const double& y0, const double& x1,
		const double& y1, const double& a0, const double& b0, const double& a1,
		const double& b1, double& xy, double& ab);

cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b);
