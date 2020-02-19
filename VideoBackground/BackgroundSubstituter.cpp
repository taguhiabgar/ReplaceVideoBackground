#include "BackgroundSubstituter.h"
#include <random>

// helper function

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


// constructor

BackgroundSubstituter::BackgroundSubstituter(const std::string& mainVideoPath, const int randomFramesCount, const std::vector<std::string>& otherVideoPaths)
	: mainVideoPath_m(mainVideoPath), randomFramesCount_m(randomFramesCount), otherVideoPaths_m(otherVideoPaths) 
{

}

// destructor

BackgroundSubstituter::~BackgroundSubstituter() {

}

// methods


int BackgroundSubstituter::computeMedian(std::vector<int> elements) {
	nth_element(elements.begin(), elements.begin() + elements.size() / 2, elements.end());
	// sort(elements.begin(),elements.end());
	return elements[elements.size() / 2];
}

cv::Mat BackgroundSubstituter::computeMedianImage(std::vector<cv::Mat> images) {
	int height = images[0].rows;
	int width = images[0].cols;
	cv::Mat medianImg(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

	for (int row = 0; row < height; row++) {
		if (row % 20 == 0) {
			std::cout << "processed: " << (int)(row * 100.0 / height) << "%" << std::endl;
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

// subtracts background from `video` and saves in passed image `background`
void BackgroundSubstituter::subtractBackground(cv::VideoCapture& video, cv::Mat& background, int randomFramesCount) {
	if (!video.isOpened()) {
		std::cerr << "Error opening video file" << std::endl;
	}

	// select frames randomly
	std::default_random_engine randomEngine;
	std::uniform_int_distribution<int> distribution(0, video.get(cv::CAP_PROP_FRAME_COUNT));

	std::vector<cv::Mat> frames;

	for (int i = 0; i < randomFramesCount; i++) {
		int fid = distribution(randomEngine);
		video.set(cv::CAP_PROP_POS_FRAMES, fid);
		cv::Mat frame;
		video >> frame;
		if (frame.empty()) {
			continue;
		}
		frames.push_back(frame);
	}

	// compute median image of randomly selected frames
	background = computeMedianImage(frames);
}

// substitutes the passed `background` image to `second` video and saves the result in `output` video
// does not modify `video`
void BackgroundSubstituter::substituteBackground(cv::Mat& background, cv::VideoCapture& video, cv::VideoCapture& output, int randomFramesCount, const std::string& videoName) {
	cv::Mat videoBackground;

	// subtract the background of `video` and save to `videoBackground` image
	subtractBackground(video, videoBackground, randomFramesCount);

	output = video;

	int frameWidth = output.get(CV_CAP_PROP_FRAME_WIDTH);
	int frameHeight = output.get(CV_CAP_PROP_FRAME_HEIGHT);

	// reset frame counter
	output.set(cv::CAP_PROP_POS_FRAMES, 0);

	std::cout << "writing the video " << videoName << ".avi ..." << std::endl;

	cv::VideoWriter videoWriter(videoName + ".avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, cv::Size(frameWidth, frameHeight));

	while (true) {
		cv::Mat frame;

		output >> frame;

		if (frame.empty()) {
			break;
		}

		// compute absolute difference of current `frame` and the background of video
		cv::Mat diffFrame;
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
				}
				else {
					frame.at<cv::Vec3b>(row, col) = frame.at<cv::Vec3b>(row, col);
				}
			}
		}

		videoWriter.write(frame);
	}

	// release the write object
	videoWriter.release();
}

// the driver program
void BackgroundSubstituter::process() const {
	cv::VideoCapture firstVideo(mainVideoPath_m);
	std::vector<cv::VideoCapture> videos;

	for (const std::string& path : otherVideoPaths_m) {
		videos.push_back(cv::VideoCapture(path));
	}

	cv::Mat firstVideoBackground;

	std::cout << "Subtracting background of main video" << std::endl;

	// subtract the video background and save to `background`
	subtractBackground(firstVideo, firstVideoBackground, randomFramesCount_m); // todo: add bool print progress = true

	// save median image to file 

	imwrite("first_video_median_image.jpg", firstVideoBackground);

	int videoNumber = 0;
	for (cv::VideoCapture& elem : videos) {
		cv::VideoCapture output;
		std::cout << "** Substituting background of video #" << videoNumber << std::endl;
		std::string videoName = "video_" + std::to_string(videoNumber++);
		substituteBackground(firstVideoBackground, elem, output, randomFramesCount_m, videoName);
	}

	// close all videos
	firstVideo.release();
	for (auto& video : videos) {
		video.release();
	}
}


