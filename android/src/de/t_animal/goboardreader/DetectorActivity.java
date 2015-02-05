package de.t_animal.goboardreader;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfPoint2f;
import org.opencv.core.Point;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

public class DetectorActivity extends Activity implements CvCameraViewListener2 {

	private static final String TAG = "T_ANIMAL::GBR::DetectorActivity";
	private static final Scalar RED = new Scalar(255, 0, 0, 255);
	private static final Scalar WHITE = new Scalar(255, 255, 255, 255);
	private static final Scalar YELLOW = new Scalar(255, 255, 0, 255);

	private Mat grayImage;
	private Mat colorImage;
	private CameraBridgeViewBase mOpenCvCameraView;

	/**
	 * Begin app lifecycle callbacks
	 */

	@Override
	public void onCreate(Bundle savedInstanceState) {
		Log.i(TAG, "called onCreate");
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		setContentView(R.layout.detector_activity);

		mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.detector_activity_surface_view);
		mOpenCvCameraView.setCvCameraViewListener(this);
	}

	@Override
	public void onPause()
	{
		super.onPause();
		if (mOpenCvCameraView != null)
			mOpenCvCameraView.disableView();
	}

	@Override
	public void onResume()
	{
		super.onResume();
		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
	}

	public void onDestroy() {
		super.onDestroy();
		mOpenCvCameraView.disableView();
	}

	/**
	 * Begin OpenCV callbacks
	 */

	@Override
	public void onCameraViewStarted(int width, int height) {
		grayImage = new Mat();
		colorImage = new Mat();
	}

	@Override
	public void onCameraViewStopped() {
		grayImage.release();
		colorImage.release();
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		grayImage = inputFrame.gray();
		Imgproc.cvtColor(grayImage, colorImage, Imgproc.COLOR_GRAY2RGB);

		MatOfPoint2f intersections = new MatOfPoint2f();
		MatOfPoint2f selectedIntersections = new MatOfPoint2f();

		detect(grayImage.getNativeObjAddr(), intersections.getNativeObjAddr(),
				selectedIntersections.getNativeObjAddr());

		for (int i = 0; i < intersections.rows(); i++) {
			double[] p = intersections.get(i, 0);
			Core.circle(colorImage, new Point(p[0], p[1]), 10, WHITE);
		}

		for (int i = 0; i < selectedIntersections.rows(); i++) {
			double[] p = selectedIntersections.get(i, 0);
			Core.circle(colorImage, new Point(p[0], p[1]), 10, YELLOW, 3);
		}

		Core.circle(colorImage, new Point(colorImage.width() / 2, colorImage.height() / 2), 10, RED, 3);

		return colorImage;
	}

	/**
	 * Begin private helpers
	 */
	private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
			case LoaderCallbackInterface.SUCCESS:
				Log.i(TAG, "OpenCV loaded successfully");
				System.loadLibrary("goboardreader");
				Log.i(TAG, "loaded goboardreader");
				mOpenCvCameraView.enableView();

				break;
			default:
				super.onManagerConnected(status);
				break;
			}
		}
	};

	public native void detect(long mgray, long intersections, long selectedIntersections);
}
