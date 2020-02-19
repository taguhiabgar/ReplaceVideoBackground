# Video Background Editor

This program allows to **substitute background** of one video
to another.

# Config file
You need to specify the paths of videos you want to run the program on. To do that, go to **VideoBackground/config.json** file and edit it.
Example file can have contents like this:

    {
    	"main_video": "video1.mp4",
    	"other_videos": ["video2.mp4", "video3.mp4"],
    	"random_frames": 7
    }

The background of **main_video** will be subtracted, and replaced with the backgrounds of **other_videos**, and the resulting files will be saved in VideoBackground directory.
The configuration parameter **random_frames** specify how many frames to use, to compute the median image, i.e. background of video. The higher this number is, the higher will be the accuracy, and the slower the program will work, and vice versa.

# Example Videos and Results

For example, I used the following configuration:

        {
        	"main_video": "testVideo.mp4",
        	"other_videos": ["speakingMan.mp4"],
        	"random_frames": 7
        }

The resulting video can be downloaded from [here](https://www.dropbox.com/s/l0r3bt42y66l10d/outcpp.avi?dl=0)

## Libraries used

OpenCV

Also used header file "json.h", which is single-include file from [here](https://github.com/nlohmann/json)
