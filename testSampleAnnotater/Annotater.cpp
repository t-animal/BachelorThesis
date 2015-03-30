#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

using namespace std;
using namespace cv;

#define ESC_KEY 1048603
#define SHIFT_KEY 1114081
#define CTRL_KEY 1114083
#define TAB_KEY 1048585

#define EMPTY 0
#define WHITE 1
#define BLACK 2

int colorSelect = 0;
vector<Point2f> emptyIntersects;
vector<Point2f> blackPieces;
vector<Point2f> whitePieces;
vector<Point2f> *allPieces[] = {&emptyIntersects, &whitePieces, &blackPieces};
vector<Point2f> *newestPoint = &emptyIntersects;
Scalar colors[] = {Scalar(160,160,160), Scalar(255,255,255), Scalar(0,0,0)};

void drawCirclesAndDisplay(Mat &src){
	Mat paintImage;
	src.copyTo(paintImage);

	int index = 0;
	for(auto v : allPieces){
		for(auto e : *v){
			circle(paintImage, e, 7, Scalar(100,100,100), 4);
			circle(paintImage, e, 7, colors[index], 2.5);
		}
		index++;
	}

	imshow("Annotate", paintImage);
}

void mouseCallback(int event, int x, int y, int flags, void* userdata){
	Mat src = *((Mat*)userdata);
	Mat paintImage;
	src.copyTo(paintImage);

	if(event == EVENT_MBUTTONUP){
		colorSelect = (colorSelect+1)%3;
		circle(paintImage, Point(x,y), 7, Scalar(100,100,100), 4);
		circle(paintImage, Point(x,y), 7, colors[colorSelect], 2.5);
	}

	if(event == EVENT_MOUSEMOVE){
		circle(paintImage, Point(x,y), 7, Scalar(100,100,100), 4);
		circle(paintImage, Point(x,y), 7, colors[colorSelect], 2.5);
	}

	if(event == EVENT_LBUTTONUP || event == EVENT_RBUTTONUP){
		int closestIndex = -1;

		double closestDistance = INT_MAX;
		vector<Point2f> *whichColor;

		int index;
		for(vector<Point2f> *v : allPieces){
			index = 0;
			for(auto e : *v){
				double distance = norm(e-Point2f(x,y));
				if(distance < closestDistance){
					whichColor = v;
					closestIndex = index;
					closestDistance = distance;
				}
				index++;
			}
		}

		if(closestDistance < 5){
			whichColor->erase(whichColor->begin()+closestIndex);
		}
	}

	if(event == EVENT_LBUTTONUP){
		allPieces[colorSelect]->push_back(Point2f(x,y));
		newestPoint = allPieces[colorSelect];
	}

	drawCirclesAndDisplay(paintImage);
}

void loadAndProcessImage(char *filename){
	Mat4f input;
	Mat image;

	char annotFilename[strlen(filename)+7];
	strcpy(annotFilename, filename);
	strcpy(&annotFilename[strlen(filename)-4], "_annot.yml");

	FileStorage readStorage(annotFilename, FileStorage::READ);

	readStorage["emptyIntersects"] >> emptyIntersects;
	readStorage["blackPieces"] >> blackPieces;
	readStorage["whitePieces"] >> whitePieces;

	readStorage.release();

	if (filename[strlen(filename) - 1] == 'l') {
		FileStorage fs(filename, FileStorage::READ);
		fs["matrix"] >> input;
		fs.release();

		input.convertTo(image, CV_8UC4);
		cvtColor(image, image, COLOR_RGB2BGR);
	} else {
		//load source image and store "as is" (rgb or bgr?) with alpha
		input = imread(filename, -1);
	}


	namedWindow("Annotate",  CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO );
	resizeWindow("Annotate", 1200, 850);
	setMouseCallback("Annotate", mouseCallback, ((void*)&image));

	drawCirclesAndDisplay(image);

	int keyCode=-1;
	while((keyCode = waitKey(0)) != ESC_KEY){
		switch(keyCode){
		case SHIFT_KEY:
			colorSelect = 1;
			break;
		case CTRL_KEY:
			colorSelect = 2;
			break;
		case TAB_KEY:
			colorSelect = 0;
			break;
		case 1048691: //s-key
			colorSelect = (colorSelect+1)%3;
			break;
		case 1048673: //a-key
			colorSelect = (colorSelect+2)%3;
			break;
		case 1113938: //up
			newestPoint->back().y--;
			drawCirclesAndDisplay(image);
			break;
		case 1113937: //left
			newestPoint->back().x--;
			drawCirclesAndDisplay(image);
			break;
		case 1113940: //down
			newestPoint->back().y++;
			drawCirclesAndDisplay(image);
			break;
		case 1113939: //right
			newestPoint->back().x++;
			drawCirclesAndDisplay(image);
			break;
		}
	}


	FileStorage writeStorage(annotFilename, FileStorage::WRITE);

	writeStorage << "emptyIntersects" << emptyIntersects;
	writeStorage << "blackPieces" << blackPieces;
	writeStorage << "whitePieces" << whitePieces;

	writeStorage.release();

	destroyAllWindows();

}

int main(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		loadAndProcessImage(argv[i]);
	}

	exit(0);
}
