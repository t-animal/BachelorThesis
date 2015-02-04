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

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

public class DetectorActivity extends Activity implements CvCameraViewListener2 {

	private static final String TAG = "T_ANIMAL::GBR::DetectorActivity";
	private static final Scalar RED = new Scalar(255, 0, 0, 255);

	private Mat image;
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
		image = new Mat();
	}

	@Override
	public void onCameraViewStopped() {
		image.release();
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		image = inputFrame.gray();

		MatOfPoint2f intersections = new MatOfPoint2f();
		MatOfPoint2f selectedIntersections = new MatOfPoint2f();

		detect(image.getNativeObjAddr(), intersections.getNativeObjAddr(),
				selectedIntersections.getNativeObjAddr());

		Log.i(TAG, "intersrows:" + intersections.rows());
		Log.i(TAG, "interscols:" + intersections.cols());

		for (int i = 0; i < intersections.rows(); i++) {
			double[] p = intersections.get(i, 0);
			Core.circle(image, new Point(p[0], p[1]), 10, RED);
		}

		for (int i = 0; i < intersections.cols(); i++) {
			double[] p = intersections.get(0, i);
			if (p == null)
				continue;
			Core.circle(image, new Point(p[0], p[1]), 10, RED);
		}

		Core.line(image, new Point(0, 0), new Point(100, 100), RED, 10);

		return image;
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
