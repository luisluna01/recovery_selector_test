#pragma once

#include <chrono>

#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <magic_enum.hpp>

#include <rclcpp/rclcpp.hpp>

#include "recovery_selector_test_msgs/msg/failure_case.hpp" // FailureCase message

#include "recovery_selector_test/failure_case_type.hpp" // FailureCase type


namespace recovery_selector_test::nodes
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

  rclcpp::Publisher<recovery_selector_test_msgs::msg::FailureCase>::SharedPtr publisher;
  rclcpp::TimerBase::SharedPtr timer;

  termios original_termios{};
  bool termios_modified = false;

  size_t failure_case_count_;
};

}  // namespace recovery_selector_test::nodes
