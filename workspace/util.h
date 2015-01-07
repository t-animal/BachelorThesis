#include <stdlib.h>
#include <sys/timeb.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int getMilliCount();
int getMilliSpan(int nTimeStart);
void rotate(cv::Mat& src, cv::Mat& dst, double angle);
