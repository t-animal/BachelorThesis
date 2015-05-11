package de.t_animal.goboardreader;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;

public class GameDisplayView extends View {
	RectF mBounds;
	String gameState = null;

	public GameDisplayView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public GameDisplayView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public GameDisplayView(Context context) {
		super(context);
	}

	@SuppressLint("DrawAllocation")
	@Override
	protected void onDraw(Canvas canvas) {
		canvas.getMaximumBitmapWidth();
		canvas.drawRGB(120, 120, 120);

		Paint gridPaint = new Paint();
		gridPaint.setARGB(255, 80, 80, 80);
		gridPaint.setStyle(Style.STROKE);
		gridPaint.setStrokeWidth(3);

		Paint whitePaint = new Paint();
		whitePaint.setARGB(255, 255, 255, 255);
		whitePaint.setStrokeWidth(3);

		Paint blackPaint = new Paint();
		blackPaint.setARGB(255, 0, 0, 0);
		blackPaint.setStrokeWidth(3);

		float width = mBounds.height() - 40;
		RectF gridOuterLine = new RectF(
				(mBounds.width() - width) / 2,
				20,
				mBounds.width() - (mBounds.width() - width) / 2,
				mBounds.height() - 20);

		canvas.drawRect(gridOuterLine, gridPaint);
		for (int i = 0; i < 9; i++) {
			canvas.drawLine(gridOuterLine.left + 20 + (width + 20) * i / 9,
					gridOuterLine.top + 20,
					gridOuterLine.left + 20 + (width + 20) * i / 9,
					gridOuterLine.bottom - 20,
					gridPaint);

			canvas.drawLine(gridOuterLine.left + 20,
					gridOuterLine.top + 20 + (width + 20) * i / 9,
					gridOuterLine.right - 20 + 3,
					gridOuterLine.top + 20 + (width + 20) * i / 9,
					gridPaint);
		}

		if (gameState != null) {
			for (int i = 0; i < gameState.length(); i++) {
				int col = i % 9;
				int row = i / 9;
				if (gameState.charAt(i) == 'w') {
					canvas.drawCircle(gridOuterLine.left + 20 + (width + 20) * col / 9,
							gridOuterLine.top + 20 + (width + 20) * row / 9,
							width / 25,
							whitePaint);
				} else if (gameState.charAt(i) == 'b') {
					canvas.drawCircle(gridOuterLine.left + 20 + (width + 20) * col / 9,
							gridOuterLine.top + 20 + (width + 20) * row / 9,
							width / 25,
							blackPaint);
				}
			}
		}
	}

	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
		mBounds = new RectF(0, 0, w, h);
	}

	public void setGame(String gameState) {
		this.gameState = gameState;
	}

}
