#ifndef PIECEDETECTION_H_
#define PIECEDETECTION_H_

/**
 * Detects white and black pieces on the board. Whites may be skipped if they are too uncertain to deliver better
 * accuracy.
 *
 * /param src: the image with the board on it
 * /param darkCircles: the output vector of black pieces
 * /param whiteCircles: the output vector of white pieces
 */
void detectPieces(cv::Mat &src, std::vector<cv::Point3f> &darkPieces, std::vector<cv::Point3f> &lightPieces);


#endif
