#include "rosbot_control/rosbot_class.h"
#include "geometry_msgs/Twist.h"
#include "nav_msgs/Odometry.h"
#include "sensor_msgs/LaserScan.h"
#include "std_msgs/Float32.h"
#include "std_msgs/Int16.h"
#include "unistd.h"
#include <ros/ros.h>

#include <list>
#include <string>

using namespace std;

// RosbotClass constructor
RosbotClass::RosbotClass() {
  n = ros::NodeHandle("~");
  laser_topic = "/scan";
  laser_sub = n.subscribe(laser_topic, 10, &RosbotClass::laser_callback, this);
  vel_topic = "/cmd_vel";
  vel_pub = n.advertise<geometry_msgs::Twist>(n.resolveName(vel_topic), 1);
  odom_topic = "/odom";
  odom_sub = n.subscribe(odom_topic, 10, &RosbotClass::odom_callback, this);
  ROS_INFO("Initializing node .................................");
  usleep(2000000);
}

void RosbotClass::laser_callback(
    const sensor_msgs::LaserScan::ConstPtr &laser_msg) {
  laser_range = laser_msg->ranges;
  // ROS_INFO("Laser value: %f", laser_range);
}
void RosbotClass::odom_callback(const nav_msgs::Odometry::ConstPtr &odom_msg) {
  x_pos = odom_msg->pose.pose.position.x;
  y_pos = odom_msg->pose.pose.position.y;
  z_pos = odom_msg->pose.pose.position.z;
  // ROS_INFO_STREAM("Odometry: x=" << x_pos << " y=" << y_pos << " z=" <<
  // z_pos);
}
void RosbotClass::move() {
  // Rate of publishing
  ros::Rate rate(10);

  ros::Time start_time = ros::Time::now();
  ros::Duration timeout(2.0); // Timeout of 2 seconds
  while (ros::Time::now() - start_time < timeout) {
    ros::spinOnce();
    vel_msg.linear.x = +0.5;
    vel_msg.angular.z = 0.0;
    vel_pub.publish(vel_msg);
    rate.sleep();
  }
  vel_msg.linear.x = 0.0;
  vel_msg.angular.z = 0.0;
  vel_pub.publish(vel_msg);
}

void RosbotClass::move_forward(int time) {
  // Rate of publishing
  ros::Rate rate(10);

  ros::Time start_time = ros::Time::now();
  ros::Duration timeout(time);
  while (ros::Time::now() - start_time < timeout) {
    ROS_INFO_STREAM("Moving forward ........... ");
    ros::spinOnce();
    vel_msg.linear.x = 0.4;
    vel_msg.angular.z = 0.0;
    vel_pub.publish(vel_msg);
    rate.sleep();
  }
  vel_msg.linear.x = 0.0;
  vel_msg.angular.z = 0.0;
  vel_pub.publish(vel_msg);
}

void RosbotClass::move_backwards(int time) {
  // Rate of publishing
  ros::Rate rate(10);

  ros::Time start_time = ros::Time::now();
  ros::Duration timeout(time);
  while (ros::Time::now() - start_time < timeout) {
    ROS_INFO_STREAM("Moving backwards ........... ");
    ros::spinOnce();
    vel_msg.linear.x = -0.5;
    vel_msg.angular.z = 0.0;
    vel_pub.publish(vel_msg);
    rate.sleep();
  }
  vel_msg.linear.x = 0.0;
  vel_msg.angular.z = 0.0;
  vel_pub.publish(vel_msg);
}

void RosbotClass::turn(string clock, int n_secs) {
  ros::Rate rate(10);
  ros::Time start_time = ros::Time::now();
  ros::Duration timeout(n_secs);

  double WZ = 0.0;
  if (clock == "clockwise") {
    ROS_INFO_STREAM("Turning clockwise..............");
    WZ = -2.5;
  } else if (clock == "counterclockwise") {
    ROS_INFO_STREAM("Turning counterclockwiseeee ........... ");
    WZ = 2.5;
  }

  while (ros::Time::now() - start_time < timeout) {
    ros::spinOnce();
    vel_msg.linear.x = 0.5;
    vel_msg.angular.z = WZ;
    vel_pub.publish(vel_msg);
    rate.sleep();
  }
  vel_msg.linear.x = 0.0;
  vel_msg.angular.z = 0.0;
  vel_pub.publish(vel_msg);
}

void RosbotClass::stop_moving() {
  ROS_INFO_STREAM("Stopping the robot ........... ");
  vel_msg.linear.x = 0.0;
  vel_msg.angular.z = 0.0;
  vel_pub.publish(vel_msg);
}

float RosbotClass::get_position(int param) {
  if (param == 1) {
    return this->x_pos;
  } else if (param == 2) {
    return this->y_pos;
  } else if (param == 3) {
    return this->z_pos;
  }
  return 0;
}

list<float> RosbotClass::get_position_full() {
  list<float> coordinates({this->x_pos, this->y_pos, this->z_pos});
  return coordinates;
}

double RosbotClass::get_time() {
  double secs = ros::Time::now().toSec();
  return secs;
}

float RosbotClass::get_laser(int index) { return laser_range[index]; }

float *RosbotClass::get_laser_full() {
  float *laser_range_pointer = laser_range.data();
  return laser_range_pointer;
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "rosbot_class_node");

  RosbotClass rosbot;

  rosbot.move();

  float coordinate = rosbot.get_position(1);

  ROS_INFO_STREAM(coordinate);

  return 0;
}