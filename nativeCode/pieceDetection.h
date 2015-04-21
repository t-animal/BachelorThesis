#ifndef PIECEDETECTION_H_
#define PIECEDETECTION_H_

class PieceDetector{
private:
	const cv::Mat src;
public:
	PieceDetector(const cv::Mat &src):src(src){};
	/**
	 * Detects white and black pieces on the board. Whites may be skipped if they are too uncertain to deliver better
	 * accuracy.
	 *
	 * /param darkCircles: the output vector of black pieces
	 * /param whiteCircles: the output vector of white pieces
	 */
	void detectPieces(std::vector<cv::Point3f> &darkPieces, std::vector<cv::Point3f> &lightPieces);
};


#endif
