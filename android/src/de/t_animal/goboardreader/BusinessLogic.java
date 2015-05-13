package de.t_animal.goboardreader;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfPoint2f;
import org.opencv.core.MatOfPoint3f;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;

import android.content.Context;

public class BusinessLogic {

	private static final Scalar RED = new Scalar(255, 0, 0, 255);
	private static final Scalar DARK_RED = new Scalar(180, 0, 0, 255);
	private static final Scalar WHITE = new Scalar(255, 255, 255, 255);
	private static final Scalar DARK_GRAY = new Scalar(80, 80, 80, 255);
	private static final Scalar LIGHT_GRAY = new Scalar(180, 180, 180, 20);
	private static final Scalar YELLOW = new Scalar(255, 255, 0, 20);

	private Mat grayImage, prevGrayImage, colorImage, detectionImage, output;
	private MatOfPoint2f prevIntersections;
	private boolean saveNextImage = false;

	private byte[] blackPieceProb = new byte[81];
	private byte[] whitePieceProb = new byte[81];

	private List<String> game = new ArrayList<String>();

	private Context context;

	public BusinessLogic(Context context) {
		this.context = context;
	}

	public void onStart() {
		detectionImage = new Mat();
	}

	public void onResume() {
		if (prevGrayImage != null) {
			prevGrayImage.release();
			prevGrayImage = null;
		}
	}

	public void onPause() {

	}

	public void onStop() {
		if (grayImage != null)
			grayImage.release();

		if (prevGrayImage != null)
			prevGrayImage.release();

		if (prevIntersections != null)
			prevIntersections.release();

		if (colorImage != null)
			colorImage.release();

		if (detectionImage != null)
			detectionImage.release();

		if (output != null)
			output.release();
	}

	public void saveNextImage() {
		synchronized (this) {
			saveNextImage = true;
		}
	}

	public void saveBoardState() {
		String board = String.copyValueOf(getCurrentProbableBoard());

		if (game.size() == 0 || !board.equals(game.get(game.size() - 1))) {
			game.add(board);
		}
	}

	public List<String> getGame() {
		return game;
	}

	public void clearGame() {
		game.clear();
	}

	public Mat analyseImage(CvCameraViewFrame inputFrame) {
		// allocate and initialise the necessary memory
		MatOfPoint2f intersections = new MatOfPoint2f();
		MatOfPoint2f selectedIntersections = new MatOfPoint2f();
		MatOfPoint2f filledIntersections = new MatOfPoint2f();
		MatOfPoint3f darkCircles = new MatOfPoint3f();
		MatOfPoint3f lightCircles = new MatOfPoint3f();

		char[] measuredBoard = new char[81];
		for (int i = 0; i < 81; i++) {
			measuredBoard[i] = '0';
		}

		// release the previous output in case it has not been released
		if (output != null)
			output.release();

		// in the first frame only save the image
		if (prevGrayImage == null) {
			prevGrayImage = inputFrame.gray();
			return prevGrayImage;
		}

		colorImage = inputFrame.rgba();
		grayImage = inputFrame.gray();
		Mat diff = new Mat(grayImage.size(), grayImage.type());
		Core.subtract(grayImage, prevGrayImage, diff);
		Imgproc.threshold(diff, diff, 100, 255, Imgproc.THRESH_BINARY);
		prevGrayImage.release();
		prevGrayImage = grayImage;

		// if there's too much movement, don't detect or output anything
		if (Core.countNonZero(diff) > 20) {
			intersections.release();
			selectedIntersections.release();
			darkCircles.release();
			lightCircles.release();
			diff.release();

			output = createOutput(colorImage, intersections, selectedIntersections, filledIntersections, darkCircles,
					lightCircles, measuredBoard);

			colorImage.release();

			return output;
		}

		colorImage.convertTo(detectionImage, CvType.CV_32FC4);

		// check if saving was requested and perform it
		boolean saveThisImage = false;
		synchronized (this) {
			if (saveNextImage) {
				saveThisImage = true;
				saveNextImage = false;
			}
		}
		if (saveThisImage)
			saveImage(detectionImage, true, true);

		// perform the actual detection
		if (prevIntersections == null) {
			detect(detectionImage.getNativeObjAddr(), intersections.getNativeObjAddr(),
					selectedIntersections.getNativeObjAddr(), filledIntersections.getNativeObjAddr(),
					darkCircles.getNativeObjAddr(), lightCircles.getNativeObjAddr(), measuredBoard, 0);
		} else {
			detect(detectionImage.getNativeObjAddr(), intersections.getNativeObjAddr(),
					selectedIntersections.getNativeObjAddr(), filledIntersections.getNativeObjAddr(),
					darkCircles.getNativeObjAddr(), lightCircles.getNativeObjAddr(), measuredBoard,
					0);
			prevIntersections.release();
		}
		prevIntersections = filledIntersections;

		updateProbableBoard(measuredBoard);
		char board[] = getCurrentProbableBoard();

		// create an output image
		output = createOutput(colorImage, intersections, selectedIntersections, filledIntersections, darkCircles,
				lightCircles, board);

		// save the output if requested
		if (saveThisImage) {
			saveImage(output, false, false);
		}

		// release the allocated memory and return
		diff.release();
		intersections.release();
		selectedIntersections.release();
		darkCircles.release();
		lightCircles.release();

		colorImage.release();
		detectionImage.release();

		return output;
	}

	private void updateProbableBoard(char currentMeasurement[]) {
		for (int i = 0; i < currentMeasurement.length; i++) {
			if (currentMeasurement[i] == 'b') {
				whitePieceProb[i] -= whitePieceProb[i] == 0 ? 0 : 1;
				blackPieceProb[i] += blackPieceProb[i] == 10 ? 0 : 1;
			} else if (currentMeasurement[i] == 'w') {
				whitePieceProb[i] += whitePieceProb[i] == 10 ? 0 : 1;
				blackPieceProb[i] -= blackPieceProb[i] == 0 ? 0 : 1;
			} else if (currentMeasurement[i] == '0') {
				whitePieceProb[i] -= whitePieceProb[i] == 0 ? 0 : 1;
				blackPieceProb[i] -= blackPieceProb[i] == 0 ? 0 : 1;
			}

		}
		System.out.println("===");
	}

	private char[] getCurrentProbableBoard() {
		char[] board = new char[81];

		for (int i = 0; i < board.length; i++) {
			if (whitePieceProb[i] > 5) {
				board[i] = 'w';
			} else if (blackPieceProb[i] > 5) {
				board[i] = 'b';
			} else {
				board[i] = '0';
			}
		}

		return board;
	}

	private Mat createOutput(Mat colorImage, MatOfPoint2f intersections, MatOfPoint2f selectedIntersections,
			MatOfPoint2f filledIntersections, MatOfPoint3f darkCircles, MatOfPoint3f lightCircles, char board[]) {
		for (int i = 0; i < intersections.rows(); i++) {
			double[] p = intersections.get(i, 0);
			Core.circle(colorImage, new Point(p[0], p[1]), 10, LIGHT_GRAY, 1);
		}

		for (int i = 0; i < selectedIntersections.rows(); i++) {
			double[] p = selectedIntersections.get(i, 0);
			Core.circle(colorImage, new Point(p[0], p[1]), 10, YELLOW, 1);
		}

		for (int i = 0; i < darkCircles.rows(); i++) {
			double[] p = darkCircles.get(i, 0);
			Core.circle(colorImage,
					new Point(p[0], p[1]), (int) p[2], DARK_GRAY, 1);
		}

		for (int i = 0; i < lightCircles.rows(); i++) {
			double[] p = lightCircles.get(i, 0);
			Core.circle(colorImage,
					new Point(p[0], p[1]), (int) p[2], WHITE, 1);
		}

		for (int i = 0; i < filledIntersections.rows(); i++) {
			double[] p = filledIntersections.get(i, 0);
			Core.circle(colorImage, new Point(p[0], p[1]), 10, DARK_RED, 3);
		}

		Core.circle(colorImage, new Point(colorImage.width() / 2, colorImage.height() / 2), 10, RED, 3);

		Size oldSize = colorImage.size();

		output = new Mat(oldSize, colorImage.type(), new Scalar(120, 120, 120));
		Mat leftOutput = new Mat(output, new
				Rect(0, 0, (int) oldSize.width * 3 / 5, (int) oldSize.height));
		Mat rightOutput = new Mat(output, new
				Rect((int) oldSize.width * 3 / 5, 0, (int) oldSize.width * 2 / 5, (int) oldSize.height));

		int colOffset = (int) (oldSize.width / 5);
		Mat subImage = colorImage.submat(0, leftOutput.rows(), colOffset,
				leftOutput.cols() + colOffset);
		subImage.copyTo(leftOutput);
		subImage.release();

		Mat boardOutput = new Mat(rightOutput, new Rect(2, (rightOutput.rows() - rightOutput.cols()) / 2 + 2,
				rightOutput.cols() - 4, rightOutput.cols() - 4));
		Scalar black = new Scalar(0, 0, 0);
		Scalar white = new
				Scalar(255, 255, 255);
		int randAbstand = boardOutput.cols() / 18;

		for (int i = 0; i < 9; i++) {
			Core.line(boardOutput, new Point(randAbstand + boardOutput.cols() * i / 9,
					randAbstand),
					new Point(randAbstand + boardOutput.cols() * i / 9, boardOutput.rows() - randAbstand), new
					Scalar(70, 70, 70), 2);
			Core.line(boardOutput, new Point(randAbstand, randAbstand + boardOutput.rows() * i /
					9), new Point(boardOutput.cols() - randAbstand, randAbstand + boardOutput.rows() * i / 9),
					new Scalar(70, 70,
							70), 2);
		}
		for (int i = 0; i < 81; i++) {
			if (board[i] == '0')
				continue;
			int row = i / 9;
			int col = i % 9;
			int radius = boardOutput.cols() / 18 - 10 - 2;// 10=abstand; 2=borderwidth
			Core.circle(boardOutput, new
					Point(boardOutput.cols() * col / 9 + randAbstand, boardOutput.rows() * row / 9 + randAbstand),
					radius,
					board[i] == 'w' ? white : black, 2);
		}
		Core.rectangle(boardOutput, new Point(0, 0), new
				Point(boardOutput.cols(), boardOutput.cols()), new Scalar(70, 70, 70), 2);

		leftOutput.release();
		rightOutput.release();
		boardOutput.release();

		return output;
	}

	private void saveImage(Mat img, boolean saveYML, boolean unprocessed) {
		String filename = android.os.Build.PRODUCT.replace(" ", "-") + "__"
				+ android.os.Build.MANUFACTURER.replace(" ", "-") + "-"
				+ android.os.Build.MODEL.replace(" ", "-") + "__"
				+ android.os.Build.DISPLAY.replace(" ", "-") + "_"
				+ new SimpleDateFormat("yyyy-MM-dd--HH:mm:ss.SSS").format(new Date());
		File storagePath = context.getExternalFilesDir(null);

		Mat rgbImage = new Mat();
		Imgproc.cvtColor(img, rgbImage, Imgproc.COLOR_BGR2RGB);
		rgbImage.convertTo(rgbImage, CvType.CV_8UC3);

		Highgui.imwrite(new File(storagePath, filename + (unprocessed ? "_un" : "_") + "processed.png").getPath(),
				rgbImage);

		rgbImage.release();
		if (saveYML)
			saveAsYAML(img.getNativeObjAddr(), new File(storagePath, filename +
					(unprocessed ? "_un" : "_") + "processed.yml").getPath());

	}

	private native void detect(long mgray, long intersections, long selectedIntersections, long filledIntersections,
			long darkCircles, long lightCircles, char board[], long prevIntersections);

	private native void saveAsYAML(long image, String filename);

}
