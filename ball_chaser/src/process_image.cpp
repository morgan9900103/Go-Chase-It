#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

class ProcessImage
{
private:
	ros::ServiceClient client;
	ros::Subscriber sub1;

public:
	ProcessImage(ros::NodeHandle *n) {
		// Define a client that is capable of requesting services from /ball_chaser/command_robot
		client = n->serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");
		
		// Define a subscriber to subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image callback function
		sub1 = n->subscribe("/camera/rgb/image_raw", 10, &ProcessImage::process_image_callback, this);
	}
	
	// Function of calling the command_robot service to drive the robot in the specified direction
	void drive_robot(float lin_x, float ang_z) {
		ROS_INFO_STREAM("Moving the robot");
		
		// Request a service and pass the velocities to it to drive the robot
		ball_chaser::DriveToTarget srv;
		srv.request.linear_x = lin_x;
		srv.request.angular_z = ang_z;
	
		if (!client.call(srv))
			ROS_ERROR("Failed to call service drive_robot");
	}
	
	// Callback function continuously executes and reds the image data
	void process_image_callback(const sensor_msgs::Image img) {
		int white_pixel = 255;
		int left = img.step/3;
		int right = 2 * img.step/3;
		int left_count = 0;
		int right_count = 0;
		int mid_count = 0;
	
		// Loop through each pixels in the image and check if the color is white
		for (int i = 0; i < img.height; i++) {
			for (int j = 0; j < img.step; j += 3) {
				int index = j + (i * img.step);
				if (img.data[index] == white_pixel && img.data[index+1] == white_pixel && img.data[index+2] == white_pixel) {
					if (j <= left)
						left_count++;
					else if (j >= right)
						right_count++;
					else
						mid_count++;
				}
				else
					continue;
			}
		}
	
		if (left_count + mid_count + right_count == 0)
			drive_robot(0.0, 0.0);
		else if (left_count > right_count)
			drive_robot(0.2, 0.5);
		else if (right_count > left_count)
			drive_robot(0.2, -0.5);
		else
			drive_robot(0.2, 0.0);
	}
};

int main(int argc, char**argv) {
	ros::init(argc, argv, "process_image");
	ros::NodeHandle n;
	ProcessImage process_image = ProcessImage(&n);
	
	ros::spin();
	
	return 0;
}
