#include "recovery_selector_test/simple_dummy_failure_publisher.hpp"


namespace recovery_selector_test::nodes
{

// -- Member Function Definitions of SimpleDummyFailurePublisher Class -- //
SimpleDummyFailurePublisher::SimpleDummyFailurePublisher()
: rclcpp::Node("simple_dummy_failure_publisher")
{
  failure_bool_topic_ = this->declare_parameter<std::string>("failure_bool_topic", "/failure_bool");

  publisher = this->create_publisher<std_msgs::msg::Bool>(
    failure_bool_topic_,
    rclcpp::QoS(10));

  if (!isatty(STDIN_FILENO))
  {
    RCLCPP_ERROR(
      this->get_logger(),
      "stdin is not a TTY. Run this node with 'ros2 run' in its own terminal, not from a launch "
      "file."
    );

    throw std::runtime_error("stdin is not a terminal");
  }

  // Stash the current terminal settings so the destructor can put them back.
  if (tcgetattr(STDIN_FILENO, &original_termios) != 0)
  {
    throw std::runtime_error("tcgetattr(STDIN_FILENO) failed");
  }

  termios raw = original_termios;
  // ICANON off  -> deliver bytes as soon as they are typed, no Enter required.
  // ECHO   off  -> keystrokes are not printed back to the terminal.
  // ISIG stays ON so Ctrl-C still raises SIGINT and rclcpp's handler works.
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 0;   // read() never blocks waiting for a minimum byte count
  raw.c_cc[VTIME] = 0;  // ...and never waits on a timer either

  if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0)
  {
    throw std::runtime_error("tcsetattr(STDIN_FILENO) failed");
  }
  termios_modified = true;

  timer = this->create_wall_timer(
    std::chrono::milliseconds(20),
    std::bind(&SimpleDummyFailurePublisher::timer_callback, this));

  RCLCPP_INFO(
    this->get_logger(),
    "Reading keys. Press 'Enter' to publish false (meaning failure) on %s.\n"
    "Ctrl-C to quit.",
    failure_bool_topic_.c_str()
  );
}


SimpleDummyFailurePublisher::~SimpleDummyFailurePublisher()
{
  // Revert to original terminal settings
  if (termios_modified) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
  }
}


void SimpleDummyFailurePublisher::timer_callback()
{
  char c = 0;
  if (!read_key(c))
  {
    return;  // nothing typed this tick
  }

  // Ensure only the 'Enter' key is allowed
  if (c != '\n' && c != '\r')
  {
    RCLCPP_WARN(
      this->get_logger(),
      "Ignoring key (code %d): must be 'Enter'", static_cast<int>(c));
    return;
  }

  // Create and populate Bool message
  auto msg = std_msgs::msg::Bool();
  msg.data = false;

  publisher->publish(msg);

  RCLCPP_INFO(
    this->get_logger(),
    "Published \'%s\' on \'%s\'",
    msg.data ? "true" : "false", failure_bool_topic_.c_str());

  return;
}


bool SimpleDummyFailurePublisher::read_key(char& c)
{
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(STDIN_FILENO, &read_fds);

  timeval timeout{0, 0};  // zero timeout: poll and return, never stall the executor

  if (select(STDIN_FILENO + 1, &read_fds, nullptr, nullptr, &timeout) <= 0)
  {
    return false;
  }

  return read(STDIN_FILENO, &c, 1) == 1;
}
// -- Member Function Definitions of SimpleDummyFailurePublisher Class -- //

} //namespace recovery_selector_test::nodes


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<recovery_selector_test::nodes::SimpleDummyFailurePublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
