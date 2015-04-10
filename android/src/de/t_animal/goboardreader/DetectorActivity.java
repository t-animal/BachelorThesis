package de.t_animal.goboardreader;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.core.Scalar;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;

import android.app.Activity;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;

public class DetectorActivity extends Activity implements CvCameraViewListener2 {

	private static final String TAG = "T_ANIMAL::GBR::DetectorActivity";
	private static final Scalar RED = new Scalar(255, 0, 0, 255);
	private static final Scalar DARK_RED = new Scalar(180, 0, 0, 255);
	private static final Scalar WHITE = new Scalar(255, 255, 255, 255);
	private static final Scalar DARK_GRAY = new Scalar(80, 80, 80, 255);
	private static final Scalar LIGHT_GRAY = new Scalar(180, 180, 180, 20);
	private static final Scalar YELLOW = new Scalar(255, 255, 0, 20);

	private Mat grayImage, colorImage, detectionImage;
	private CameraManipulatingView mOpenCvCameraView;
	private boolean saveNextImage = false;

	/**
	 * Begin app lifecycle callbacks
	 */

	@Override
	public void onCreate(Bundle savedInstanceState) {
		Log.i(TAG, "called onCreate");
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		setContentView(R.layout.detector_activity);

		mOpenCvCameraView = (CameraManipulatingView) findViewById(R.id.detector_activity_surface_view);
		mOpenCvCameraView.setCvCameraViewListener(this);
		mOpenCvCameraView.enableFpsMeter();
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
	 * Begin user interaction callbacks
	 */

	public void saveNextImage(View v) {
		synchronized (this) {
			saveNextImage = true;
		}
	}

	/**
	 * Begin OpenCV callbacks
	 */

	@Override
	public void onCameraViewStarted(int width, int height) {
		grayImage = new Mat();
		colorImage = new Mat();
		detectionImage = new Mat();

		Camera.Parameters camParams = mOpenCvCameraView.getCameraParameters();

		// Log.i(TAG, "is auto exposure lock supported:" + camParams.isAutoExposureLockSupported());
		// Log.i(TAG, "is auto whitebalance lock supported:" + camParams.isAutoWhiteBalanceLockSupported());
		// camParams.setAutoExposureLock(true);
		// camParams.setAutoWhiteBalanceLock(true);
		mOpenCvCameraView.setCameraParameters(camParams);
	}

	@Override
	public void onCameraViewStopped() {
		grayImage.release();
		colorImage.release();
		detectionImage.release();
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		colorImage = inputFrame.rgba();
		grayImage = inputFrame.gray();
		Imgproc.cvtColor(grayImage, grayImage, Imgproc.COLOR_GRAY2RGB);
		colorImage.convertTo(detectionImage, CvType.CV_32FC4);

		boolean saveThisImage = false;
		synchronized (this) {
			if (saveNextImage) {
				saveThisImage = true;
				saveNextImage = false;
			}
		}
		String filename = "";
		File storagePath = getExternalFilesDir(null);
		if (saveThisImage) {
			Mat rgbImage = new Mat();
			Imgproc.cvtColor(colorImage, rgbImage, Imgproc.COLOR_BGR2RGBA);
			filename = android.os.Build.PRODUCT.replace(" ", "-") + "__"
					+ android.os.Build.MANUFACTURER.replace(" ", "-") + "-" + android.os.Build.MODEL.replace(" ", "-")
					+ "__" + android.os.Build.DISPLAY.replace(" ", "-") + "_"
					+ new SimpleDateFormat("yyyy-MM-dd--HH:mm:ss.SSS").format(new Date());

			Highgui.imwrite(new File(storagePath, filename + "_unprocessed.png").getPath(), rgbImage);
			saveAsYAML(detectionImage.getNativeObjAddr(),
					new File(storagePath, filename + "_unprocessed.yml").getPath());
			rgbImage.release();
		}

		// MatOfPoint2f intersections = new MatOfPoint2f();
		// MatOfPoint2f selectedIntersections = new MatOfPoint2f();
		// MatOfPoint3f filledIntersections = new MatOfPoint3f();
		// MatOfPoint3f darkCircles = new MatOfPoint3f();
		// MatOfPoint3f lightCircles = new MatOfPoint3f();

		// detect(detectionImage.getNativeObjAddr(), intersections.getNativeObjAddr(),
		// selectedIntersections.getNativeObjAddr(), filledIntersections.getNativeObjAddr(),
		// darkCircles.getNativeObjAddr(), lightCircles.getNativeObjAddr());
		//
		// for (int i = 0; i < intersections.rows(); i++) {
		// double[] p = intersections.get(i, 0);
		// Core.circle(colorImage, new Point(p[0], p[1]), 10, LIGHT_GRAY, 1);
		// }
		//
		// for (int i = 0; i < selectedIntersections.rows(); i++) {
		// double[] p = selectedIntersections.get(i, 0);
		// Core.circle(colorImage, new Point(p[0], p[1]), 10, YELLOW, 1);
		// }
		//
		// for (int i = 0; i < darkCircles.rows(); i++) {
		// double[] p = darkCircles.get(i, 0);
		// Core.circle(colorImage, new Point(p[0], p[1]), (int) p[2], DARK_GRAY, 1);
		// }
		//
		// for (int i = 0; i < lightCircles.rows(); i++) {
		// double[] p = lightCircles.get(i, 0);
		// Core.circle(colorImage, new Point(p[0], p[1]), (int) p[2], WHITE, 1);
		// }
		//
		// for (int i = 0; i < filledIntersections.rows(); i++) {
		// double[] p = filledIntersections.get(i, 0);
		// Core.circle(colorImage, new Point(p[0], p[1]), 10, DARK_RED, 3);
		// }

		Core.circle(colorImage, new Point(colorImage.width() / 2, colorImage.height() / 2), 10, RED, 3);

		if (saveThisImage) {
			Mat rgbImage = new Mat();
			Imgproc.cvtColor(colorImage, rgbImage, Imgproc.COLOR_BGR2RGB);

			Highgui.imwrite(new File(storagePath, filename + "_processed.png").getPath(), rgbImage);
			Core.subtract(Mat.ones(colorImage.size(), colorImage.type()).setTo(new Scalar(255)), colorImage, colorImage);

			rgbImage.release();

		}

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

	public native void detect(long mgray, long intersections, long selectedIntersections, long filledIntersections,
			long darkCircles, long lightCircles);

	public native void saveAsYAML(long image, String filename);
}
