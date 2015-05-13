package de.t_animal.goboardreader;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.app.Activity;
import android.content.Intent;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.Toast;

public class DetectorActivity extends Activity implements CvCameraViewListener2, OnTouchListener {

	private static final String TAG = "T_ANIMAL::GBR::DetectorActivity";

	private CameraManipulatingView mOpenCvCameraView;
	private ImageView mStateIconView;
	private BusinessLogic businessLogic;

	private enum State {
		STOPPED, RECORDING, PAUSED
	};

	private State currentState = State.STOPPED;

	/**
	 * Begin app lifecycle callbacks
	 */

	@Override
	public void onCreate(Bundle savedInstanceState) {
		Log.i(TAG, "called onCreate");
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		setContentView(R.layout.detector_activity);

		mStateIconView = (ImageView) findViewById(R.id.state_view);

		mOpenCvCameraView = (CameraManipulatingView) findViewById(R.id.detector_activity_surface_view);
		mOpenCvCameraView.setCvCameraViewListener(this);
		mOpenCvCameraView.enableFpsMeter();

		mOpenCvCameraView.setOnTouchListener(this);

		businessLogic = new BusinessLogic(this);
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
		businessLogic.onResume();

		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
	}

	public void onDestroy() {
		super.onDestroy();
		mOpenCvCameraView.disableView();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.detector_activity_menu_actions, menu);

		if (currentState == State.RECORDING || currentState == State.PAUSED) {
			menu.findItem(R.id.action_stop).setVisible(true);
			menu.findItem(R.id.action_record).setVisible(false);

			if (currentState == State.RECORDING) {
				menu.findItem(R.id.action_pause).setVisible(true);
			} else {
				menu.findItem(R.id.action_pause).setVisible(false);
				menu.findItem(R.id.action_record).setVisible(true);
			}
		} else {
			menu.findItem(R.id.action_stop).setVisible(false);
			menu.findItem(R.id.action_pause).setVisible(false);
			menu.findItem(R.id.action_record).setVisible(true);
		}
		return super.onCreateOptionsMenu(menu);
	}

	/**
	 * Begin user interaction callbacks
	 */

	private long timeDown = 0;

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_UP) {
			if (event.getEventTime() - event.getDownTime() > 1500) {
				saveNextImage(v);
			} else {
				// set center
			}
		}
		return true;
	}

	public void saveNextImage(View v) {
		businessLogic.saveNextImage();
	}

	public void goToReplay(MenuItem m) {
		startActivity(new Intent(getApplicationContext(), ReplayActivity.class));
	}

	public void setStatePaused(MenuItem item) {
		currentState = State.PAUSED;

		mStateIconView.setVisibility(View.VISIBLE);
		mStateIconView.setImageResource(R.drawable.ic_recording_paused);

		invalidateOptionsMenu();
	}

	public void setStateRecording(MenuItem item) {
		currentState = State.RECORDING;

		Parameters params = mOpenCvCameraView.getCameraParameters();
		params.setAutoExposureLock(true);
		params.setAutoWhiteBalanceLock(true);
		mOpenCvCameraView.setCameraParameters(params);

		mStateIconView.setVisibility(View.VISIBLE);
		mStateIconView.setImageResource(R.drawable.ic_recording);

		Runnable r = new Runnable() {
			boolean iconVisible = true;

			public void run() {
				if (currentState != State.RECORDING)
					return;

				if (iconVisible) {
					mStateIconView.setVisibility(View.VISIBLE);
					mStateIconView.postDelayed(this, 750);
				} else {
					mStateIconView.setVisibility(View.INVISIBLE);
					mStateIconView.postDelayed(this, 500);
				}
				iconVisible = !iconVisible;
			}
		};
		mStateIconView.post(r);

		invalidateOptionsMenu();
	}

	public void setStateStopped(MenuItem item) {
		currentState = State.STOPPED;

		mStateIconView.setVisibility(View.VISIBLE);
		mStateIconView.setImageResource(R.drawable.ic_recording_stopped);

		mStateIconView.animate().setStartDelay(500).setDuration(250).alpha(0)
				.setListener(new AnimatorListenerAdapter() {
					@Override
					public void onAnimationEnd(Animator animation) {
						mStateIconView.setVisibility(View.INVISIBLE);
						mStateIconView.setAlpha(1.0f);
					}
				}).start();

		invalidateOptionsMenu();

		try {
			File file = new File(getExternalFilesDir(null), "GoGame-"
					+ new SimpleDateFormat("yyyy-MM-dd--HH:mm:ss").format(new Date()));
			FileOutputStream fos = new FileOutputStream(file);
			OutputStreamWriter osw = new OutputStreamWriter(fos);

			for (String s : businessLogic.getGame())
				osw.write(s + "\n");

			osw.close();
			fos.close();

			businessLogic.clearGame();

			Toast.makeText(this, "Your game was saved.", Toast.LENGTH_SHORT).show();
		} catch (IOException e) {
			// should not happen
			Toast.makeText(this,
					"An error occured while saving. Please record a new game and stop it immediately to retry",
					Toast.LENGTH_LONG).show();
		}

	}

	/**
	 * Begin OpenCV callbacks
	 */

	@Override
	public void onCameraViewStarted(int width, int height) {
		businessLogic.onStart();

		Camera.Parameters camParams = mOpenCvCameraView.getCameraParameters();

		// Log.i(TAG, "is auto exposure lock supported:" + camParams.isAutoExposureLockSupported());
		// Log.i(TAG, "is auto whitebalance lock supported:" + camParams.isAutoWhiteBalanceLockSupported());
		// camParams.setAutoExposureLock(true);
		// camParams.setAutoWhiteBalanceLock(true);
		mOpenCvCameraView.setCameraParameters(camParams);
	}

	@Override
	public void onCameraViewStopped() {
		businessLogic.onStop();
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		Mat tmp = businessLogic.analyseImage(inputFrame);

		if (currentState == State.RECORDING)
			businessLogic.saveBoardState();

		return tmp;
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

}
