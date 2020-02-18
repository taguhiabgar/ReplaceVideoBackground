#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "json.h"

using namespace std;
using namespace cv;
using json = nlohmann::json;

void displayVideo(VideoCapture& video);

int computeMedian(vector<int> elements);

cv::Mat computeMedianImage(std::vector<cv::Mat> vec);

// subtracts background from `video` and saves in passed image `background`
void subtractBackground(VideoCapture& video, Mat& background);

// substitutes the passed `background` image to `second` video and saves the result in `output` video
void substituteBackground(Mat& background, VideoCapture& video, VideoCapture& output);

bool almostBlack(cv::Vec3b& color);

int main() {

	// ** read video paths from JSON

	std::ifstream configFile("config.json");
	json config;
	configFile >> config;

	std::string mainVideoPath = config["main_video"];
	std::vector<std::string> otherVideoPaths;
	for (const std::string& elem : config["other_videos"]) {
		otherVideoPaths.push_back(elem);
	}

	// ** driver program

	VideoCapture firstVideo(mainVideoPath);
	std::vector<VideoCapture> videos;

	for (const std::string& path : otherVideoPaths) {
		videos.push_back(VideoCapture(path));
	}

	Mat firstVideoBackground;

	// subtract the video background and save to `background`
	subtractBackground(firstVideo, firstVideoBackground); // todo: add bool print progress = true

	// save median image to file 

	imwrite("first_video_median_image.jpg", firstVideoBackground);

	for (VideoCapture& elem : videos) {
		VideoCapture output;
		substituteBackground(firstVideoBackground, elem, output);
	}

	// close all videos
	firstVideo.release();
	for (auto& video : videos) {
		video.release();
	}

	return 0;
}

// substitutes the passed `background` image to `second` video and saves the result in `output` video
// does not modify `video`
void substituteBackground(Mat& background, VideoCapture& video, VideoCapture& output) {

	Mat videoBackground;

	// subtract the background of `video` and save to `videoBackground` image
	subtractBackground(video, videoBackground);
	
	output = video;

	int frameWidth = output.get(CV_CAP_PROP_FRAME_WIDTH);
	int frameHeight = output.get(CV_CAP_PROP_FRAME_HEIGHT);

	// reset frame counter
	output.set(CAP_PROP_POS_FRAMES, 0);

	VideoWriter videoWriter("outcpp.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, Size(frameWidth, frameHeight));

	while (true) {
		Mat frame;

		output >> frame;

		if (frame.empty()) {
			break;
		}

		// compute absolute difference of current `frame` and the background of video
		Mat diffFrame;
		absdiff(frame, videoBackground, diffFrame);

		int height = frame.rows;
		int width = frame.cols;

		// replace all (almost) black pixels of `diffFrame` with the corresponding pixels of `background`
		// non-black pixels indicate the places where objects are moving
		for (int row = 0; row < height; ++row) {
			for (int col = 0; col < width; ++col) {
				cv::Vec3b color = diffFrame.at<cv::Vec3b>(row, col);
				if (almostBlack(color)) {
					frame.at<cv::Vec3b>(row, col) = background.at<cv::Vec3b>(row, col);
				} else {
					frame.at<cv::Vec3b>(row, col) = frame.at<cv::Vec3b>(row, col);
				}
			}
		}

		videoWriter.write(frame);
	}

	// release the write object
	videoWriter.release();
}

bool almostBlack(cv::Vec3b& color) {
	int b = color[0];
	int g = color[1];
	int r = color[2];

	double hsp = sqrt(
		0.299 * (r * r) +
		0.587 * (g * g) +
		0.114 * (b * b)
	);

	return hsp < 30;
}

// subtracts background from the video and saves in passed image `background`
void subtractBackground(VideoCapture& video, Mat& background) {
	if (!video.isOpened()) {
		cerr << "Error opening video file" << endl;
	}

	// select frames randomly
	default_random_engine randomEngine;
	uniform_int_distribution<int> distribution(0, video.get(CAP_PROP_FRAME_COUNT));

	vector<Mat> frames;
	
	for (int i = 0; i < 7; i++) {
		int fid = distribution(randomEngine);
		video.set(CAP_PROP_POS_FRAMES, fid);
		Mat frame;
		video >> frame;
		if (frame.empty()) {
			continue;
		}
		frames.push_back(frame);
	}

	// compute median image of randomly selected frames
	background = computeMedianImage(frames);
}

void displayVideo(VideoCapture& video) {
	// reset frame number to 0
	video.set(CAP_PROP_POS_FRAMES, 0);

	while (true) {
		Mat frame;
		video >> frame;

		if (frame.empty()) {
			break;
		}

		// show the resulting frame
		imshow("Frame", frame);
	}
}

int computeMedian(vector<int> elements) {
	nth_element(elements.begin(), elements.begin() + elements.size() / 2, elements.end());
	// sort(elements.begin(),elements.end());
	return elements[elements.size() / 2];
}

cv::Mat computeMedianImage(std::vector<cv::Mat> images) {
	int height = images[0].rows;
	int width = images[0].cols;
	cv::Mat medianImg(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

	for (int row = 0; row < height; row++) {
		if (row % 20 == 0) {
			cout << "processed: " << (int)(row * 100.0 / height) << "%" << endl;
		}
		for (int col = 0; col < width; col++) {
			std::vector<int> blues, greens, reds;

			for (int imgIndex = 0; imgIndex < images.size(); ++imgIndex) {
				blues.push_back(images[imgIndex].at<cv::Vec3b>(row, col)[0]);
				greens.push_back(images[imgIndex].at<cv::Vec3b>(row, col)[1]);
				reds.push_back(images[imgIndex].at<cv::Vec3b>(row, col)[2]);
			}

			medianImg.at<cv::Vec3b>(row, col)[0] = computeMedian(blues);
			medianImg.at<cv::Vec3b>(row, col)[1] = computeMedian(greens);
			medianImg.at<cv::Vec3b>(row, col)[2] = computeMedian(reds);
		}
	}
	return medianImg;
}


