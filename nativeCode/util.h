#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include <sys/timeb.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

/*
 * Get the current milliseconds since the epoch
 */
int getMilliCount();

/**
 * Get the time passed since nTimeStart in milliseconds
 *
 * \param nTimeStart a timestamp in milliseconds
 *
 * \returns milliseconds since nTimeStart
 */
int getMilliSpan(int nTimeStart);


/**
 * Rotate an image ccw by \p angle degrees
 *
 * \param src the image to rotate
 * \param dst the output array
 * \param angle the angle to rotate the image by
 */
void rotate(cv::Mat& src, cv::Mat& dst, double angle);

/**
 * Utility function to sort a vector of pairs after pair.first
 *
 * \param a: the first pair to compare
 * \param b: the seconds par to compare
 *
 * \returns a.first < b.first
 */
bool sortFunction(std::pair<double, cv::Point2f> a, std::pair<double, cv::Point2f> b);

/**
 * Returns the opencv type of the Matrix as a human readable String
 *
 * \param type: an opencv type
 */
std::string type2str(int type);


/**
 * Draws a histogram of \p src onto \p dst
 *
 * \param src: the Mat to calculate the histogram of
 * \param dest: the Mat to draw the histogram onto
 */
void drawHistogram(const cv::Mat &src, cv::Mat &dst);

#endif