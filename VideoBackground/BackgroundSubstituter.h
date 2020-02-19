#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <vector>



class BackgroundSubstituter {
public:

	BackgroundSubstituter(const std::string& mainVideoPath, const int randomFramesCount, const std::vector<std::string>& otherVideoPaths);

	void process() const;

	static int computeMedian(std::vector<int> elements);

	static cv::Mat computeMedianImage(std::vector<cv::Mat> images);

	// subtracts background from `video` and saves in passed image `background`
	static void subtractBackground(cv::VideoCapture& video, cv::Mat& background, int randomFramesCount);

	// substitutes the passed `background` image to `second` video and saves the result in `output` video
	static void substituteBackground(cv::Mat& background, cv::VideoCapture& video, cv::VideoCapture& output, int randomFramesCount, const std::string& videoName);

	

	virtual ~BackgroundSubstituter();

private:

	std::string mainVideoPath_m;

	int randomFramesCount_m;
	
	std::vector<std::string> otherVideoPaths_m;
};
