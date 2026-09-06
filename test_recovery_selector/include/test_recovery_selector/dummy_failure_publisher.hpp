#include <chrono>

#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include <rclcpp/rclcpp.hpp>

#include <std_msgs/msg/string.hpp>


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

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher;
  rclcpp::TimerBase::SharedPtr timer;

  termios original_termios{};
  bool termios_modified = false;
};

}  // namespace test_recovery_selector::nodes
