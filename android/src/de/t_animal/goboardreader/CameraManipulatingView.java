package de.t_animal.goboardreader;

import org.opencv.android.JavaCameraView;

import android.content.Context;
import android.hardware.Camera;
import android.util.AttributeSet;

public class CameraManipulatingView extends JavaCameraView {

	public CameraManipulatingView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public CameraManipulatingView(Context context, int cameraId) {
		super(context, cameraId);
	}

	public void setCameraParameters(Camera.Parameters params) {
		mCamera.setParameters(params);
	}

	public Camera.Parameters getCameraParameters() {
		return mCamera.getParameters();
	}

}
