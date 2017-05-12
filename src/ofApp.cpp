#include "ofApp.h"
#include "opencv2/objdetect/objdetect.hpp"

using namespace cv;

//--------------------------------------------------------------
void ofApp::setup() {
	// frame in which the camera signal will be shown
	cam.setup(320, 240);
	ofBackground(ofColor::white);



  colorImage.allocate(320, 240);
  grayImage.allocate(320, 240);
  grayBg.allocate(320, 240);
  grayDiff.allocate(320, 240);



	//set framerate (speed) of the travelling pixel
	ofSetFrameRate(60);
	windowHeight = ofGetWindowHeight();
	windowWidth = ofGetWindowWidth();
	pixelSize = 1;
	spiralSize = pixelSize;
	spiralDirection = 0;
	startX = ceil(windowWidth / 2.0);
	startY = ceil(windowHeight / 2.0);

	// find minimal recognized pixel size
	cam.update();
	img = cam.getPixels();
	colorImage = cam.getPixels();
  colorImage.convertToGrayscalePlanarImage(grayBg, 1);

	detectPixelSize(colorImage);
	startX -= ceil(pixelSize / 2.0);
	startY -= ceil(pixelSize / 2.0);
	x = startX;
	y = startY;
	std::cout << "done with setup.\n";

	maxSize = max(ofGetWindowHeight(), ofGetWindowWidth());


	
}

//--------------------------------------------------------------
void ofApp::update() {
	// static update of the received Signal
	cam.update();
	img = cam.getPixels();
	colorImage = cam.getPixels();

  
	
	// Two variantes to copy the colored image into a grayscale one
	//grayImage.setFromimg(img);
	colorImage.convertToGrayscalePlanarImage(grayImage,1);

	img.setImageType(OF_IMAGE_GRAYSCALE);



	if (!cam.isFrameNew()) {
		detectPixelSize(colorImage);
	}
	else {
		calculateNextSpiralPosition();
	}


	// Detect the square in the image
	// for debug: used a sheet of paper with a black square drawn on it
	// TODO: find way to detect the blob in update() function instead of in draw()

	// copy the gray image so that it can be pre-processed before trying to find
	//// the pixel in its
	//contourImage = grayImage;
	//// first blur the image and threshold it, to get a real black & white image
	//// for easier processing; thresholding inverts colours:
	////square -> white, background -> black, needed to find blob
	//contourImage.blurGaussian();
	//contourImage.threshold(50, true); // threshold chosen arbitrarily

  // Take the absolute value of the difference 
  // between the background and incoming images.
  grayDiff.absDiff(grayBg, grayImage);

  // Perform an in-place thresholding of the difference image.
  grayDiff.threshold(80);

  // Find contours whose areas are betweeen 20 and 25000 pixels.
  // "Find holes" is true, so we'll also get interior contours.
  contourFinder.findContours(grayDiff, 20, 500, 10, true);

}

//--------------------------------------------------------------
// Make a white plane on which one or more Pixel(s) are travelling 
void ofApp::draw() {

  colorImage.draw(20, 20);    // The incoming color image
  grayImage.draw(360, 20);  // A gray version of the incoming video
  grayBg.draw(20, 280);     // The stored background image
  grayDiff.draw(360, 280);  // The thresholded difference image



	// draw the grayscale image in the upper left corner of the window
	//grayImage.draw(0, 0);
	

	// for debug purposes: draw the blurred and thresholded image roughly in the
	// middle of the screen; delete later
	//contourImage.draw(810, 0);

	////TODO: not quite stable yet, vulnerable to scale of square and surrounding lightning

	//contourFinder.findContours(contourImage, 1,
		//100, 1, false, true);
	cout << "Found " << contourFinder.nBlobs << " blobs. \n";
	// if a blob was found, draw its bounding box
	/*if (contourFinder.nBlobs != 0) {
		blob = contourFinder.blobs.at(0);
		ofSetColor(ofColor::yellow);
		contourFinder.draw();
	}*/

  

  // Draw each blob individually from the blobs vector
  int numBlobs = contourFinder.nBlobs;
  for (int i = 0; i<numBlobs; i++) {
    contourFinder.blobs[i].draw(20, 20);
  }

	// get screen resolution
	// horizon = ofGetWindowWidth();
	// vert = ofGetWindowHeight();

	// output of screen resolution for testing
	//string text = "width " + to_string(horizon) + " height " + to_string(vert);
	//ofDrawBitmapStringHighlight(text,ofPoint(10,10,0.0),ofColor::white,ofColor::black );

	// draw the travelling pixel
	ofSetColor(ofColor::black);
	ofDrawRectangle(x, y, pixelSize, pixelSize);
	ofSetColor(ofColor::white);
	

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {


}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

//--------------------------------------------------------------
void ofApp::detectPixelSize(ofxCvColorImage &image) {

  ofPixels pix= image.getPixels();
	// go over all pixels
	for (auto & pixel : pix.getPixelsIter()) {
		// and detect their colour
		pixel.getColor();
		// for debug purposes: if the colour is white something was detected
		// later: if a black pixel was recognized on a white screen
		if ((pixel.getColor() == ofColor::white)) {
			recog = true;
		}
		else {
			// else increase the pixel size by one
			pixelSize++;
		}
	}
	// check for pixelSize exceeding the window size
	if (pixelSize >= maxSize/10) {
		std::cout << "Error! pixelSize exceeds the size of the window!\n";
		pixelSize = ceil(maxSize / 10.0) ;
		std::cout << "Setting pixelSize to the maxSize: " << pixelSize;
	}
	std::cout << "Found pixelSize = " << pixelSize << ".\n";
}

//--------------------------------------------------------------
void ofApp::calculateNextSpiralPosition() {
	switch (spiralDirection) {
		case 0: std::cout << "in case 0\n";
			if (y < 0) {                        // reaching upper windowborder
				x = startX - spiralSize;          // change x to left spiral side
				spiralDirection = 2;              // -> 180� turn of direction
			}
			y--;
			if (y <= startY - spiralSize) {     // dy > spiralsize will lead to 
				spiralDirection = 1;              // change of direction to left
			};
			break;
		case 1: std::cout << "in case 1\n";
			if (x < 0) {                        // reaching left windowborder
				y = startY + spiralSize;          // change x to upper spiral side
				spiralDirection = 3;              // -> 180� turn of direction
			}
			x--;
			if (x <= startX - spiralSize) {     // dx < spiralsize will lead to 
				spiralDirection = 2;              // change of direction to left
				spiralSize += pixelSize;
			};
			break;
		case 2: std::cout << "in case 2\n";
			if (y > ofGetWindowHeight()) {
				x = startX + spiralSize;
				spiralDirection = 0;
			}
			y++;
			if (y >= startY + spiralSize) {
				spiralDirection = 3; // change direction to left
			};
			break;
		case 3: std::cout << "in case 3\n";
			if (x > ofGetWindowWidth()) {
				y = startY - spiralSize;
				spiralDirection = 1;
			}
			x++;
			if (x >= startX + spiralSize) {
				spiralDirection = 0; // change direction to left
				spiralSize += pixelSize;
			};
			break;
	}
	if (x > windowWidth) {
		std::cout << "reseting spiral\n";
		x = startX;
		y = startY;
		spiralSize = pixelSize;
	}
}