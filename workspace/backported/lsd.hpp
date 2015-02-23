#ifndef _LSD_OCV3

#define _LSD_OCV3

namespace cv{

	//! Variants of Line Segment Detector
	enum { LSD_REFINE_NONE = 0,
		   LSD_REFINE_STD  = 1,
		   LSD_REFINE_ADV  = 2
		 };


	template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
	{
	    return Ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8));
	}

	class LineSegmentDetector : public Algorithm
	{
	public:
	/**
	 * Detect lines in the input image.
	 *
	 * @param _image    A grayscale(CV_8UC1) input image.
	 *                  If only a roi needs to be selected, use
	 *                  lsd_ptr->detect(image(roi), ..., lines);
	 *                  lines += Scalar(roi.x, roi.y, roi.x, roi.y);
	 * @param _lines    Return: A vector of Vec4i elements specifying the beginning and ending point of a line.
	 *                          Where Vec4i is (x1, y1, x2, y2), point 1 is the start, point 2 - end.
	 *                          Returned lines are strictly oriented depending on the gradient.
	 * @param width     Return: Vector of widths of the regions, where the lines are found. E.g. Width of line.
	 * @param prec      Return: Vector of precisions with which the lines are found.
	 * @param nfa       Return: Vector containing number of false alarms in the line region, with precision of 10%.
	 *                          The bigger the value, logarithmically better the detection.
	 *                              * -1 corresponds to 10 mean false alarms
	 *                              * 0 corresponds to 1 mean false alarm
	 *                              * 1 corresponds to 0.1 mean false alarms
	 *                          This vector will be calculated _only_ when the objects type is REFINE_ADV
	 */
		virtual void detect(cv::InputArray _image, cv::OutputArray _lines,
				cv::OutputArray width = cv::noArray(), cv::OutputArray prec = noArray(),
				cv::OutputArray nfa = noArray()) = 0;

	/**
	 * Draw lines on the given canvas.
	 * @param _image    The image, where lines will be drawn.
	 *                  Should have the size of the image, where the lines were found
	 * @param lines     The lines that need to be drawn
	 */
		virtual void drawSegments(InputOutputArray _image, InputArray lines) = 0;

	/**
	 * Draw both vectors on the image canvas. Uses blue for lines 1 and red for lines 2.
	 * @param size      The size of the image, where lines were found.
	 * @param lines1    The first lines that need to be drawn. Color - Blue.
	 * @param lines2    The second lines that need to be drawn. Color - Red.
	 * @param _image    Optional image, where lines will be drawn.
	 *                  Should have the size of the image, where the lines were found
	 * @return          The number of mismatching pixels between lines1 and lines2.
	 */
		virtual int compareSegments(const Size& size, InputArray lines1, InputArray lines2, InputOutputArray _image = noArray()) = 0;

		virtual ~LineSegmentDetector() { }
	};

	//! Returns a pointer to a LineSegmentDetector class.
	Ptr<LineSegmentDetector> createLineSegmentDetector(
		int _refine = LSD_REFINE_STD, double _scale = 0.8,
		double _sigma_scale = 0.6, double _quant = 2.0, double _ang_th = 22.5,
		double _log_eps = 0, double _density_th = 0.7, int _n_bins = 1024);

	/*
	 * End backport
	 */
}

#endif
