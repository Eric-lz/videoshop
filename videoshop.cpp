#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;

static void on_trackbar(int, void*);
void getOperation(int input_key);
void printMenu();

typedef struct t_Ops {
	int CONTRAST = 0;
	int BRIGHTNESS = 0;
	int GRAYSCALE = 0;
	int INVERT = 0;
	int GAUSS = 0;
	int CANNY = 0;
	int SOBEL = 0;
	int MIRRORV = 0;
	int MIRRORH = 0;
	int ROTATE = 0;
	int RESIZE = 0;
} Operations;

// Keep track of which operations were selected and their parameters
Operations ops;
int slider;
bool recording = false;

int main(int argc, char** argv)
{
	int camera = 0;
	VideoCapture cap;

	// Open the default camera, use something different from 0 otherwise;
	// Check VideoCapture documentation.
	if (!cap.open(camera))
		return 0;

	// Create window to show original video
	String w_original_name = "Original video";
	namedWindow(w_original_name, WINDOW_AUTOSIZE);

	// Create window to show modified video
	String w_modified_name = "Modified video";
	namedWindow(w_modified_name, WINDOW_AUTOSIZE);

	// Create trackbar
	const int slider_max = 30;
	String TrackbarName = "Value";
	createTrackbar(TrackbarName, w_modified_name, &slider, slider_max, on_trackbar);

	// Create video writer
	VideoWriter writer;
	int codec = writer.fourcc('m', 'p', '4', 'v');
	int fps = 30;
	Size frame_size(640, 480);

	// Print help message
	std::cout << "Press Q to display the list of available commands" << std::endl;

	while (true) {
		// Original video
		Mat f_original;
		cap >> f_original;

		// Show original video
		if (f_original.empty()) break; // end of video stream
		imshow(w_original_name, f_original);

		// Modified video (copy of original)
		Mat f_modified;
		f_original.convertTo(f_modified, -1);

		// Poll key pressed by user
		int key_pressed = pollKey();
		getOperation(key_pressed);

		// Perform selected operations
		if (ops.CONTRAST != 0) {
			f_modified.convertTo(f_modified, -1, ops.CONTRAST, 0);
		}
		if (ops.BRIGHTNESS != 0) {
			f_modified.convertTo(f_modified, -1, 1.0, ops.BRIGHTNESS);
		}
		if (ops.GRAYSCALE != 0) {
			if(f_modified.channels() == 3) // check if image is already grayscale
				cvtColor(f_modified, f_modified, COLOR_BGR2GRAY);
			// convert back to RGB to allow recording
			cvtColor(f_modified, f_modified, COLOR_GRAY2RGB);
		}
		if (ops.INVERT != 0) {
			f_modified.convertTo(f_modified, -1, -1.0, 255);
		}
		if (ops.GAUSS != 0) {
			GaussianBlur(f_modified, f_modified, Size(ops.GAUSS, ops.GAUSS), 0);
		}
		if (ops.SOBEL != 0) {
			// Create two frames for X and Y directions
			Mat sobel_x;
			Mat sobel_y;
			if (f_modified.channels() == 3)	// check if image is already grayscale
				cvtColor(f_modified, f_modified, COLOR_BGR2GRAY);
			// Run Sobel in X and Y directions
			Sobel(f_modified, sobel_x, 3, 1, 0);
			Sobel(f_modified, sobel_y, 3, 0, 1);
			// Scale and add both gradients into output frame
			convertScaleAbs(sobel_x, sobel_x);
			convertScaleAbs(sobel_y, sobel_y);
			addWeighted(sobel_x, 0.5, sobel_y, 0.5, 0, f_modified);
			// convert back to RGB to allow recording
			cvtColor(f_modified, f_modified, COLOR_GRAY2RGB);
		}
		if (ops.CANNY != 0) {
			if (f_modified.channels() == 3)	// check if image is already grayscale
				cvtColor(f_modified, f_modified, COLOR_BGR2GRAY);
			Canny(f_modified, f_modified, 50, 200);
			// convert back to RGB to allow recording
			cvtColor(f_modified, f_modified, COLOR_GRAY2RGB);
		}
		if (ops.MIRRORH != 0) {
			flip(f_modified, f_modified, 1);
		}
		if (ops.MIRRORV != 0) {
			flip(f_modified, f_modified, 0);
		}
		if (ops.ROTATE != 0) {
			if (!writer.isOpened())	// Block rotation while recording
				rotate(f_modified, f_modified, ROTATE_90_CLOCKWISE);
		}
		if (ops.RESIZE != 0) {
			if (!writer.isOpened())	// Block rotation while recording
				resize(f_modified, f_modified, Size(), 0.5, 0.5);
		}
		if (recording) {	// Start recording
			// open writer if it isn't already
			if (!writer.isOpened()) {
				std::cout << "Recording started" << std::endl;
				writer.open("output.mp4", codec, fps, frame_size);
			}

			// write new frame
			writer << f_modified;
		}
		else {	// Finish recording
			if (writer.isOpened()) {
				std::cout << "Recording finished" << std::endl;
				writer.release();
			}
		}
		
		// Show modified video on new window
		imshow(w_modified_name, f_modified);

		// Display help when user presses Q
		if (key_pressed == 'q') printMenu();

		// stop capturing by pressing ESC
		if (key_pressed == 27) break;
	}

	// release the VideoCapture object
	cap.release();

	return 0;
}

static void on_trackbar(int, void*) {
	//std::cout << "Trackbar updated\n";
}

void getOperation(int input_key) {
	int key = toupper(input_key);

	switch (key)
	{
	case 'O':
		ops = { 0 };
		break;
	case 'C':
		if (ops.CONTRAST == 0) {
			ops.CONTRAST = slider;
		}
		else {
			ops.CONTRAST = 0;
		}
		break;
	case 'B':
		if (ops.BRIGHTNESS == 0) {
			ops.BRIGHTNESS = slider;
		}
		else {
			ops.BRIGHTNESS = 0;
		}
		break;
	case 'G':
		if (ops.GAUSS == 0 && slider != 0) {
			// Make sure gauss size is odd
			ops.GAUSS = slider % 2 ? slider : slider - 1;
		}
		else {
			ops.GAUSS = 0;
		}
		break;
	case 'Y':
		ops.GRAYSCALE = !ops.GRAYSCALE;
		break;
	case 'T':
		ops.INVERT = !ops.INVERT;
		break;
	case 'E':
		ops.SOBEL = !ops.SOBEL;
		break;
	case 'N':
		ops.CANNY = !ops.CANNY;
		break;
	case 'H':
		ops.MIRRORH = !ops.MIRRORH;
		break;
	case 'V':
		ops.MIRRORV = !ops.MIRRORV;
		break;
	case 'R':
		ops.ROTATE = !ops.ROTATE;
		break;
	case 'Z':
		ops.RESIZE = !ops.RESIZE;
		break;
	case 'S':
		recording = !recording;
		break;
	default:
		break;
	}
}

void printMenu()
{
	system("cls");
	std::cout << "O - Revert to original video\n";
	std::cout << "C - Adjust contrast\n";
	std::cout << "B - Adjust brightness\n";
	std::cout << "Y - Grayscale\n";
	std::cout << "T - Grayscale\n";
	std::cout << "H - Mirror horizontally\n";
	std::cout << "V - Mirror vertically\n";
	std::cout << "G - Apply Gaussian blur\n";
	std::cout << "E - Apply Sobel filter\n";
	std::cout << "N - Apply Canny filter\n";
	std::cout << "R - Rotate 90 degrees clockwise\n";
	std::cout << "Z - Resize\n";
	std::cout << "S - Start/Stop recording video\n";

	std::cout << "Brightness, Contrast and Gaussian blur operations use the slider as parameter\n";
	std::cout << "Rotate and Resize operations are unavailable while recording\n";
	std::cout << std::endl;
}
