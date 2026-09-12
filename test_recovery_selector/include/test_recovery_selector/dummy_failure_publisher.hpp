#pragma once

#include <chrono>

#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <magic_enum.hpp>

#include <rclcpp/rclcpp.hpp>

#include <std_msgs/msg/string.hpp>
#include "test_recovery_selector_msgs/msg/failure_case.hpp" // FailureCase message

#include "test_recovery_selector/failure_case_type.hpp" // FailureCase type


namespace test_recovery_selector::nodes
{

class DummyFailurePublisher : public rclcpp::Node
{
public:
  DummyFailurePublisher();

  virtual ~DummyFailurePublisher() override;

  
private:
  void timer_callback();

  // Non-blocking single-byte read. Returns true only if a byte was consumed.
  bool read_key(char& c);

  rclcpp::Publisher<test_recovery_selector_msgs::msg::FailureCase>::SharedPtr publisher;
  rclcpp::TimerBase::SharedPtr timer;

  termios original_termios{};
  bool termios_modified = false;

  size_t failure_case_count_;
};

}  // namespace test_recovery_selector::nodes
