#pragma once

#include <chrono>
#include <string>

#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include <rclcpp/rclcpp.hpp>

#include <std_msgs/msg/bool.hpp> // Bool message


namespace recovery_selector_test::nodes
{

// Publishes a false Bool message (a failure) when the 'Enter' key is pressed in the terminal
// The topic is set by the [failure_bool_topic] ROS parameter (default: "/failure_bool")
class SimpleDummyFailurePublisher : public rclcpp::Node
{
public:
  SimpleDummyFailurePublisher();

  virtual ~SimpleDummyFailurePublisher() override;


private:
  void timer_callback();

  // Non-blocking single-byte read. Returns true only if a byte was consumed.
  bool read_key(char& c);

  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher;
  rclcpp::TimerBase::SharedPtr timer;

  termios original_termios{};
  bool termios_modified = false;

  std::string failure_bool_topic_;
};

}  // namespace recovery_selector_test::nodes
