#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include "json.h"
#include "BackgroundSubstituter.h"

using json = nlohmann::json;

int main() {

	// read video paths from JSON

	std::ifstream configFile("config.json");
	json config;
	configFile >> config;

	const std::string mainVideoPath = config["main_video"];
	const int randomFramesCount = config["random_frames"];
	std::vector<std::string> otherVideoPaths;
	
	for (const std::string& elem : config["other_videos"]) {
		otherVideoPaths.push_back(elem);
	}

	// create BackgroundSubstituter object
	BackgroundSubstituter backgroundSubstituter(mainVideoPath, randomFramesCount, otherVideoPaths);
	
	// substitute background of video at `mainVideoPath` with backgrounds of videos at `otherVideoPaths`
	backgroundSubstituter.process();

	return 0;
}




