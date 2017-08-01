// MeasureProject.cpp : Defines the entry point for the console application.
// TODO: MAKE SURE ZOOM STICKS THROUGH CLICKS
#include "stdafx.h"

// Namespaces
using namespace std;
using namespace cv;

// Global variables
bool DEBUG_VAR = false;							// Debug to check version and dependencies
bool REPEAT = true;							// Loop var to repeat main loop
bool IS_ZOOM = false;						// Check if TEMP is zoomed in
Mat image;									// Original image
Mat* TEMP = new Mat();						// Copy of current shown iteration. Used as a WORKSPACE and displayed.
string IMAGE_DIRECTORY;						// Input image
const string INPUT_CONSTANT = "input/";		// Input folder string
const string OUTPUT_CONSTANT = "output/";	// Output folder string
vector<Coordinates*> POINTS;				// Pixel coordinates
vector<Mat*> IMAGES;						// Vector of images for gfx, used for full resolution tagging. Backend, drawn on, but not displayed
int LOCATION = 0;							// Iteration index
double IMG_SCALE = 0.64;
int X_POS = -1;								// Mouse position for x
int Y_POS = -1;								// Mouse position for y
const int ZOOM_SCALE = 200;					// Scale to zoom in
int ZOOM_TEMP_X = -1;						// Temp variables to keep zoom in the same area
int ZOOM_TEMP_Y = -1;
int ZOOM_RECT_X = -1;						// x,y coordinates of zoomed in box
int ZOOM_RECT_Y = -1;
const double PI = 3.141592653589793238463;	// Pi constant

int el_width_x1 = 0;
int el_width_y1 = 0;
int el_width_x2 = 0;
int el_width_y2 = 0;

int el_height_x1 = 0;
int el_height_y1 = 0;
int el_height_x2 = 0;
int el_height_y2 = 0;

double el_center_x = 0;
double el_center_y = 0;

// Forward declarations
static void setup_vector();
static void check_directories();
static void load_image();
static void print_controls();
static void show_image();
void mouse_callback(int event, int x, int y, int flags, void* userdata);
static void undo();
static void zoom(int x, int y);
static void save();
static void prompt_new();

// Main method
int main()
{
	cout << "Measure Project\nAuthor: Andy Thai" << endl;
	cout << "Using OpenCV version " << CV_VERSION << endl << endl;
	if (DEBUG_VAR) {
		cout << getBuildInformation();
	}

	while (REPEAT) {
		setup_vector();
		check_directories();
		load_image();
		print_controls();
		show_image();
		save();
		prompt_new();
	}

	return 0;
}

// Initializes points for use in tracking ratios and angles
static void setup_vector() {
	// Reset if re-iterated
	if (POINTS.size() != 0) {
		for (int i = 0; i < 7; i++) {
			POINTS[i]->set_coords(-1, -1);
			delete IMAGES[i];
			IMAGES[i] = new Mat();
		}
		delete IMAGES[7];
		IMAGES[7] = new Mat();
	}
	// Otherwise initialize
	else {
		for (int i = 0; i < 7; i++) {
			POINTS.push_back(new Coordinates());
			IMAGES.push_back(new Mat());
		}
		IMAGES.push_back(new Mat()); // 1 more
	}
}

// Check if specific directories exist
static void check_directories() {
    struct stat st;
    if (stat("input/",&st) == 0) {
        if ((st.st_mode) & (S_IFDIR != 0)) {
            //std::cout << "Contains input directory!" << std::endl;
        }
    }
    else {
        std::cout << "No input directory detected!" << std::endl <<
        "Creating input directory..." << std::endl <<
        "The program will shut off. Please move your input files into the input folder." << std::endl;
        mkdir("input",0777);
        cin.ignore();
        exit(1);
    }
    
    if (stat("output/",&st) == 0) {
        if ((st.st_mode) & (S_IFDIR != 0)) {
            //std::cout << "Contains output directory!" << std::endl;
        }
    }
    else {
        std::cout << "No output directory detected!" << std::endl <<
        "Creating output directory..." << std::endl;
        mkdir("output",0777);
    }
    
    /** WINDOWS
	// Check if input and output directories exist
	if (!fs::exists("input")) { // Check if input folder exists
		cout << "\nNo input folder detected! Creating input folder." << endl <<
			"FrameGrabber will shut down. Please move your input images to the input folder." << endl;
		fs::create_directory("input"); // create input folder
		cin.ignore();
		exit(EXIT_SUCCESS);
	}
	if (!fs::exists("output")) { // Check if output folder exists
		cout << "\nCreating output folder..." << endl;
		fs::create_directory("output"); // create output folder
	}
     **/
}

// Loads image
static void load_image() {
	// Get video file location
	bool valid_input = false;
	cout << "Input image file: ";

	while (!valid_input) {
		getline(cin, IMAGE_DIRECTORY);

		// Concatenate to add input folder directory to beginning
		string image_input = INPUT_CONSTANT + IMAGE_DIRECTORY;

		// Initialize and load image
		image = imread(image_input, CV_LOAD_IMAGE_UNCHANGED);
		if (image.data) {
			valid_input = true;
		}
		else {
			cout << "Unable to load " << IMAGE_DIRECTORY << "! Please re-enter image filename: ";
		}
	}
	
    resize(image, image, Size(), IMG_SCALE, IMG_SCALE); // Fit to screen
    //image.resize(Size(1400,1080));
    
	// Sets image vector
	for (int i = 0; i < 8; i++) {
		image.copyTo(*IMAGES[i]);
	}
}

// Prints controls for the program
static void print_controls() {
	
	cout << endl << "CONTROLS:" << endl;
    cout << "\nClick on the left, right, top, and bottom corners of the rat's eye (respectively for width and height)." << endl;
    cout << "Then select three anchor points, the first, middle point, then end point." << endl << endl;
    cout << "\tLeft click to select parts (7 total)." << endl << endl;
    cout << "\tSelect two points for the horizontal (width)" << endl << "\tdistance for the rat's eye. This is represented in green points." << endl << endl;
    cout << "\tThen, select two points for the vertical (height)" << endl << "\tdistance for the rat's eye. This is represented in blue points." << endl << endl;
    cout << "\tFinally, select three anchor points as the angle to be calculated" << endl << "\tThis is represented in red points." << endl << "\t(First, middle, and last point in order)" << endl << endl;
	cout << "\tOnce all parts are selected, press x/X to exit." << endl;
	cout << "\tIf you make a mistake, press u/U to UNDO your action." << endl << endl;
}

// Shows image
static void show_image() {
	namedWindow(IMAGE_DIRECTORY, WINDOW_AUTOSIZE);				// Create a window for display.
	setMouseCallback(IMAGE_DIRECTORY, mouse_callback, NULL);	// Sets callback function for mouse
	IMAGES[0]->copyTo(*TEMP);									// Initialize TEMP
	imshow(IMAGE_DIRECTORY, *TEMP);								// Show our image inside it.
	char keystroke = 0;

	// Continue until conditions met
	while (1) {
		keystroke = waitKey(1);		// Wait for a keystroke in the window

		// Undo
		if (keystroke == 'u' || keystroke == 'U') {
			undo();
		}

		// Zoom
		else if (keystroke == 'z') {
			zoom(X_POS, Y_POS);
		}
		else if (keystroke == 'Z') { // Zoom out to original
			IS_ZOOM = false;
			IMAGES[LOCATION]->copyTo(*TEMP);
			imshow(IMAGE_DIRECTORY, *TEMP);
		}

		// Exit
		else if ((keystroke == 'x' || keystroke == 'X') && (LOCATION != 7)) {
			cout << "Please pick all points before concluding." << endl;
			cout << LOCATION << " / 7 points chosen." << endl << endl;
		}
		else if ((keystroke == 'x' || keystroke == 'X') && (LOCATION == 7)) {
			break;
		}
	}
	cout << "Point input concluded!" << endl << endl;
	destroyAllWindows();
}

// Mouse callback function
void mouse_callback(int event, int x, int y, int flags, void* userdata)
{
	// Get width
	X_POS = x;
	Y_POS = y;

	if (IS_ZOOM) {
		x = (((double)x / TEMP->size().width) * ZOOM_SCALE) + ZOOM_RECT_X;
		y = (((double)y / TEMP->size().height) * ZOOM_SCALE) + ZOOM_RECT_Y;
	}

	if (LOCATION < 2) {
		if (event == EVENT_LBUTTONDOWN || event == EVENT_RBUTTONDOWN) {

			// Left side
			if (LOCATION == 0) {
				cout << "Width side 1 of rat eye selected at (" << x << ", " << y << ")" << endl;
                
                el_width_x1 = x;
                el_width_y1 = y;
                
				POINTS[LOCATION]->set_coords(x, y);
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
				LOCATION++;
				circle(*IMAGES[LOCATION], Point(x, y), 2, Scalar(0, 255, 0, 1), -1);
				IMAGES[LOCATION]->copyTo(*TEMP);
				if (IS_ZOOM) {
					zoom(ZOOM_TEMP_X, ZOOM_TEMP_Y);
				}
				else {
					imshow(IMAGE_DIRECTORY, *TEMP);
				}
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
			}

			// Right side
			else if (LOCATION == 1) {
				cout << "Width side 2 of rat eye selected at (" << x << ", " << y << ")" << endl;
                
                el_width_x2 = x;
                el_width_y2 = y;
                
				POINTS[LOCATION]->set_coords(x, y);
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
				LOCATION++;
				circle(*IMAGES[LOCATION], Point(x, y), 2, Scalar(0, 255, 0, 1), -1);
				line(*IMAGES[LOCATION], Point(POINTS[LOCATION - 2]->get_x(), POINTS[LOCATION - 2]->get_y()), Point(x, y), Scalar(0, 255, 0, 1), 1);
				IMAGES[LOCATION]->copyTo(*TEMP);
				if (IS_ZOOM) {
					zoom(ZOOM_TEMP_X, ZOOM_TEMP_Y);
				}
				else {
					imshow(IMAGE_DIRECTORY, *TEMP);
				}
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
			}
		}
	}

	// Get height
	else if (LOCATION < 4) {
		if (event == EVENT_LBUTTONDOWN || event == EVENT_RBUTTONDOWN) {

			// Top side
			if (LOCATION == 2) {
				cout << "Height side 1 of rat eye selected at (" << x << ", " << y << ")" << endl;
                
                el_height_x1 = x;
                el_height_y1 = y;
                
				POINTS[LOCATION]->set_coords(x, y);
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
				LOCATION++;
				circle(*IMAGES[LOCATION], Point(x, y), 2, Scalar(255, 0, 0, 1), -1);
				IMAGES[LOCATION]->copyTo(*TEMP);
				if (IS_ZOOM) {
					zoom(ZOOM_TEMP_X, ZOOM_TEMP_Y);
				}
				else {
					imshow(IMAGE_DIRECTORY, *TEMP);
				}
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
			}

			// Bottom side
			else if (LOCATION == 3) {
				cout << "Height side 2 of rat eye selected at (" << x << ", " << y << ")" << endl;
                
                el_height_x2 = x;
                el_height_y2 = y;
                
                el_center_x = (el_width_x1 + el_width_x2) / 2.0;
                el_center_y = (el_height_y1 + el_height_y2) / 2.0;
                
                double el_center_size_x = abs(el_width_x2 - el_width_x1);
                double el_center_size_y = abs(el_height_y2 - el_height_y1);
                
				POINTS[LOCATION]->set_coords(x, y);
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
				LOCATION++;
				circle(*IMAGES[LOCATION], Point(x, y), 2, Scalar(255, 0, 0, 1), -1);
				line(*IMAGES[LOCATION], Point(POINTS[LOCATION - 2]->get_x(), POINTS[LOCATION - 2]->get_y()), Point(x, y), Scalar(255, 0, 0, 1), 1);
                
                ellipse(*IMAGES[LOCATION], Point(el_center_x, el_center_y), Size(el_center_size_x / 2.0, el_center_size_y / 2.0), 0, 0, 360, Scalar(255,0,255), 1.5);
                
				IMAGES[LOCATION]->copyTo(*TEMP);
				if (IS_ZOOM) {
					zoom(ZOOM_TEMP_X, ZOOM_TEMP_Y);
				}
				else {
					imshow(IMAGE_DIRECTORY, *TEMP);
				}
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
			}
		}
	}

	// Get angle
	else if (LOCATION < 7) {
		if (event == EVENT_LBUTTONDOWN || event == EVENT_RBUTTONDOWN) {

			// First anchor point
			if (LOCATION == 4) {
				cout << "First anchor point selected at (" << x << ", " << y << ")" << endl;
				POINTS[LOCATION]->set_coords(x, y);
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
				LOCATION++;
				circle(*IMAGES[LOCATION], Point(x, y), 2, Scalar(0, 0, 255, 1), -1);
				IMAGES[LOCATION]->copyTo(*TEMP);
				if (IS_ZOOM) {
					zoom(ZOOM_TEMP_X, ZOOM_TEMP_Y);
				}
				else {
					imshow(IMAGE_DIRECTORY, *TEMP);
				}
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
			}

			// Second anchor point
			else if (LOCATION == 5) {
				cout << "Second (middle) anchor point selected at (" << x << ", " << y << ")" << endl;
				POINTS[LOCATION]->set_coords(x, y);
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
				LOCATION++;
				circle(*IMAGES[LOCATION], Point(x, y), 2, Scalar(0, 0, 255, 1), -1);
				line(*IMAGES[LOCATION], Point(POINTS[LOCATION - 2]->get_x(), POINTS[LOCATION - 2]->get_y()), Point(x, y), Scalar(0, 0, 255, 255), 2);
				IMAGES[LOCATION]->copyTo(*TEMP);
				if (IS_ZOOM) {
					zoom(ZOOM_TEMP_X, ZOOM_TEMP_Y);
				}
				else {
					imshow(IMAGE_DIRECTORY, *TEMP);
				}
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
			}

			// Third anchor point
			else if (LOCATION == 6) {
				cout << "Last anchor point selected at (" << x << ", " << y << ")" << endl;
				POINTS[LOCATION]->set_coords(x, y);
				IMAGES[LOCATION]->copyTo(*IMAGES[LOCATION + 1]);
				LOCATION++;
				circle(*IMAGES[LOCATION], Point(x, y), 2, Scalar(0, 0, 255, 1), -1);
				line(*IMAGES[LOCATION], Point(POINTS[LOCATION - 2]->get_x(), POINTS[LOCATION - 2]->get_y()), Point(x, y), Scalar(0, 0, 255, 255), 2);
				IMAGES[LOCATION]->copyTo(*TEMP);
				if (IS_ZOOM) {
					zoom(ZOOM_TEMP_X, ZOOM_TEMP_Y);
				}
				else {
					imshow(IMAGE_DIRECTORY, *TEMP);
				}
			}
		}
	}

	else if (LOCATION >= 7) {
		if (event == EVENT_LBUTTONDOWN || event == EVENT_RBUTTONDOWN) {
			cout << "All points have been selected. Please press x/X to conclude or u/U if you need to undo an action." << endl;
		}
	}
}

// Undo command
static void undo() {
	if (LOCATION > 0) {
		cout << "UNDO ";
		if (LOCATION == 1) {
			cout << "width side 1." << endl;
		}
		else if (LOCATION == 2) {
			cout << "width side 2." << endl;
		}
		else if (LOCATION == 3) {
			cout << "height side 1." << endl;
		}
		else if (LOCATION == 4) {
			cout << "height side 2." << endl;
		}
		else if (LOCATION == 5) {
			cout << "first anchor." << endl;
		}
		else if (LOCATION == 6) {
			cout << "second anchor (middle)." << endl;
		}
		else if (LOCATION == 7) {
			cout << "third anchor." << endl;
		}
		LOCATION--;
		IMAGES[LOCATION]->copyTo(*TEMP);
		if (IS_ZOOM) {
			zoom(ZOOM_TEMP_X, ZOOM_TEMP_Y);
		}
		else {
			imshow(IMAGE_DIRECTORY, *TEMP);
		}
	}
}

// Zoom in
static void zoom(int x, int y) {

	// Saves temp variables for keeping zoom consistent throughout iterations
	ZOOM_TEMP_X = x;
	ZOOM_TEMP_Y = y;
	IS_ZOOM = true;

	int width = ZOOM_SCALE;
	int height = ZOOM_SCALE;

	int ptoX = x - (ZOOM_SCALE / 2);
	int ptoY = y - (ZOOM_SCALE / 2);
	Mat imagen = *TEMP;

	/*Verifica que el ROI este dentro de la la imagen*/
	if ((x + (ZOOM_SCALE / 2)) > imagen.size().width)
		width = width - ((x + (ZOOM_SCALE / 2)) - imagen.size().width);

	if ((y + (ZOOM_SCALE / 2)) > imagen.size().height)
		height = height - ((y + (ZOOM_SCALE / 2)) - imagen.size().height);

	if ((x - (ZOOM_SCALE / 2)) < 0)
		ptoX = 0;

	if ((y - (ZOOM_SCALE / 2)) < 0)
		ptoY = 0;

	Rect roi = Rect(ptoX, ptoY, width, height);

	ZOOM_RECT_X = ptoX;
	ZOOM_RECT_Y = ptoY;

	Mat imagen_roi = imagen(roi);
	resize(imagen_roi, imagen_roi, Size(IMAGES[0]->size().width, IMAGES[0]->size().height), 0, 0, CV_INTER_AREA);
	imshow(IMAGE_DIRECTORY, imagen_roi);
}

// Writes to a log for image info
static void save() {
	// Find distance for width selections
	int w_x1 = POINTS[0]->get_x();
	int w_y1 = POINTS[0]->get_y();
	int w_x2 = POINTS[1]->get_x();
	int w_y2 = POINTS[1]->get_y();
	double width = sqrt(pow((w_x2 - w_x1), 2) + pow((w_y2 - w_y1), 2));

	// Find distance for height selections
	int h_x1 = POINTS[2]->get_x();
	int h_y1 = POINTS[2]->get_y();
	int h_x2 = POINTS[3]->get_x();
	int h_y2 = POINTS[3]->get_y();
	double height = sqrt(pow((h_x2 - h_x1), 2) + pow((h_y2 - h_y1), 2));

	// Calculate ratio (height/width)
	double height_width_ratio = height / width;

	// Find angle given three anchor points with Law of Cosines
	int anchor1_x = POINTS[4]->get_x();
	int anchor1_y = POINTS[4]->get_y();
	int anchor2_x = POINTS[5]->get_x();
	int anchor2_y = POINTS[5]->get_y();
	int anchor3_x = POINTS[6]->get_x();
	int anchor3_y = POINTS[6]->get_y();
	double a = sqrt(pow((anchor2_x - anchor1_x), 2) + pow((anchor2_y - anchor1_y), 2)); // Anchor 1 to 2
	double b = sqrt(pow((anchor3_x - anchor2_x), 2) + pow((anchor3_y - anchor2_y), 2)); // Anchor 2 to 3
	double c = sqrt(pow((anchor1_x - anchor3_x), 2) + pow((anchor1_y - anchor3_y), 2)); // Anchor 1 to 3
	double angle = abs(acos((pow(c, 2) - pow(a, 2) - pow(b, 2)) / (-2 * a * b)) * (180.0 / PI));

	// Prints results to console
	cout << "Height: " << height << " pixels" << endl;
	cout << "Width: " << width << " pixels" << endl;
	cout << "Height / Width ratio: " << height_width_ratio << endl;
	cout << "Angle: " << angle << " degrees" << endl << endl;

	// Save into data log
	string output = OUTPUT_CONSTANT + IMAGE_DIRECTORY.substr(0, IMAGE_DIRECTORY.find(".")) + ".csv";
	ofstream output_file;
	output_file.open(output, ios::out | ios::binary | ios::trunc);
	output_file << IMAGE_DIRECTORY << ",," << endl;
	output_file << "ratio," << height_width_ratio << "," << endl;
	output_file << "heightpoint1," << h_x1 << "," << h_y1 << endl;
	output_file << "heightpoint2," << h_x2 << "," << h_y2 << endl;
	output_file << "widthpoint1," << w_x1 << "," << w_y1 << endl;
	output_file << "widthpoint2," << w_x2 << "," << w_y2 << endl;
	output_file << "angle," << angle << "," << endl;
	output_file << "angleanchor1," << anchor1_x << "," << anchor1_y << endl;
	output_file << "angleanchor2," << anchor2_x << "," << anchor2_y << endl;
	output_file << "angleanchor3," << anchor3_x << "," << anchor3_y << endl;
	output_file.close();
}

// Prompt to run another image
static void prompt_new() {
	cout << "Analysis of " << IMAGE_DIRECTORY << " concluded!" << endl << endl;
	string response;
	bool loop = true;
	while (loop) {
		cout << "Would you like to run another image? (y/n) ";
		getline(cin, response);
		if (response == "y" || response == "Y") {
			loop = false;
			REPEAT = true;
			LOCATION = 0;
            IS_ZOOM = false;
			cout << endl;
		}
		else if (response == "n" || response == "N") {
			loop = false;
			REPEAT = false;
		}
		else {
			cout << "Invalid response." << endl;
		}
	}
}
