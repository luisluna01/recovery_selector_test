#include "test_recovery_selector/dummy_failure_publisher.hpp"


namespace test_recovery_selector::nodes
{

// -- Member Function Definitions of DummyFailurePublisher Class -- //
DummyFailurePublisher::DummyFailurePublisher()
: rclcpp::Node("dummy_failure_publisher")
{
  publisher = this->create_publisher<test_recovery_selector_msgs::msg::FailureCase>(
    "/failure_source",
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
    std::bind(&DummyFailurePublisher::timer_callback, this));
  
  failure_case_count_ = magic_enum::enum_count<FailureCase>();
  RCLCPP_INFO(
    this->get_logger(),
    "Reading keys. Can only use keys from '0' to '%zu'.\n"
    "Each keypress is published on /failure_source. Ctrl-C to quit.",
    failure_case_count_-1
  );
}


DummyFailurePublisher::~DummyFailurePublisher()
{
  // Revert to original terminal settings
  if (termios_modified) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
  }
}


void DummyFailurePublisher::timer_callback()
{
  char c = 0;
  if (!read_key(c))
  {
    return;  // nothing typed this tick
  }

  // Ensure only keys pressed from '0' to the last index number of the enumerator are allowed
  // Note: Code block will need to be modified if enum class has greater than 10 enumerators
  if (c < '0' || c >= '0' + static_cast<char>(failure_case_count_))
  {
    RCLCPP_WARN(
      this->get_logger(),
      "Ignoring key '%c': must be a digit 0-%zu", c, failure_case_count_ - 1);
    return;
  }

  uint8_t case_id = static_cast<uint8_t>(c - '0'); // Convert from key pressed to integer

  // Create and populate FailureCase message
  auto msg = test_recovery_selector_msgs::msg::FailureCase();  
  msg.case_id = case_id;

  FailureCase failure_case = static_cast<FailureCase>(case_id);
  std::string failure_case_string = failureCaseToString(failure_case);

  publisher->publish(msg);

  RCLCPP_INFO(
    this->get_logger(),
    "Published int '%d' which should map to failure case: %s",
    msg.case_id, failure_case_string.c_str());

  return;
}


bool DummyFailurePublisher::read_key(char& c)
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
// -- Member Function Definitions of DummyFailurePublisher Class -- //

} //namespace test_recovery_selector::nodes


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);  
  auto node = std::make_shared<test_recovery_selector::nodes::DummyFailurePublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  
  return 0;
}
