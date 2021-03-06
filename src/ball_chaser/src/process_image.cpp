#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ROS_INFO_STREAM("chasing the white ball");

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the DriveToTarget service and pass the required velocities.
    if (!client.call(srv))
        ROS_ERROR("Failed to call service DriveToTarget");

}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    int white_pixel_position = -1;
    for (int i = 0; i < img.height * img.step; i++) {
        // Check if we could find a white pixel by checking all RGB data.
        if (img.data[i] == white_pixel 
		and img.data[i+1] == white_pixel 
		and  img.data[i+2] == white_pixel ) {
            white_pixel_position = i;
            break;
        }
    }
    if (white_pixel_position != -1 )
    {
      int segment = (white_pixel_position % img.step) / (img.step/3);
      // Check if the white pixel is in the left segment. If yes, turn left.
      if (segment  == 0) {
         drive_robot(0,0.5); 
      }
      // Check if the white pixel is in the center segment. If yes, move ahead.
      else if (segment == 1 ) {
         drive_robot(0.5, 0);
      }
      // Check if the white pixel is in the right segment. If yes, turn right.
      else {
         drive_robot(0,-0.5);
      }
    }
    // If no white pixel found, stop.
    else {
       drive_robot(0, 0);
    }

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
