package de.t_animal.goboardreader;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

public class ReplayActivity extends Activity implements OnItemSelectedListener {

	Spinner mFileChooser;
	GameDisplayView mGameDisplayView;
	ArrayAdapter<String> files;
	List<String> game;
	int curState = 0;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.replay_activity);

		mFileChooser = (Spinner) findViewById(R.id.fileChooser);
		mGameDisplayView = (GameDisplayView) findViewById(R.id.gameDisplayView1);

		files = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
		for (File f : getExternalFilesDir(null).listFiles()) {
			if (f.getName().startsWith("GoGame")) {
				files.add(f.getName());
			}
		}

		mFileChooser.setAdapter(files);
		mFileChooser.setOnItemSelectedListener(this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.replay_activity_menu_actions, menu);
		return true;
	}

	public void goToRecord(MenuItem m) {
		startActivity(new Intent(getApplicationContext(), DetectorActivity.class));
	}

	void drawState(int no) {
		if (no >= game.size())
			no = game.size() - 1;

		mGameDisplayView.setGame(game.get(no));
		mGameDisplayView.invalidate();
	}

	public void nextState(View v) {
		curState = ++curState >= game.size() ? game.size() - 1 : curState;
		drawState(curState);
	}

	public void prevState(View v) {
		curState = --curState < 0 ? 0 : curState;
		drawState(curState);
	}

	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
		File file = new File(getExternalFilesDir(null), files.getItem(position));
		try {
			InputStream fis = new FileInputStream(file);
			InputStreamReader isr = new InputStreamReader(fis, Charset.forName("UTF-8"));
			BufferedReader bfr = new BufferedReader(isr);

			game = new ArrayList<String>();
			String line;
			while ((line = bfr.readLine()) != null) {
				game.add(line);
			}

			bfr.close();

			curState = 0;
			drawState(0);

		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
		// TODO Auto-generated method stub

	}
}
