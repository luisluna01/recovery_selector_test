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
  DummyFailurePublisher() : rclcpp::Node("dummy_failure_publisher")
  {
    publisher = this->create_publisher<std_msgs::msg::String>("/failure_source", rclcpp::QoS(10));

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

    RCLCPP_INFO(
      this->get_logger(),
      "Reading keys. Each keypress is published on /failure_source. Ctrl-C to quit."
    );
  }

  ~DummyFailurePublisher() override
  {
    // Revert to original terminal settings
    if (termios_modified) {
      tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
    }
  }

  
private:
  void timer_callback()
  {
    char c = 0;
    if (!read_key(c))
    {
      return;  // nothing typed this tick
    }

    std_msgs::msg::String message;
    message.data = std::string(1, c);

    publisher->publish(message);

    RCLCPP_INFO(this->get_logger(), "Publishing dummy failure as \"%s\"", message.data.c_str());
  }

  // Non-blocking single-byte read. Returns true only if a byte was consumed.
  bool read_key(char & c)
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

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher;
  rclcpp::TimerBase::SharedPtr timer;

  termios original_termios{};
  bool termios_modified{false};
};

}  // namespace test_recovery_selector::nodes


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);  
  auto node = std::make_shared<test_recovery_selector::nodes::DummyFailurePublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  
  return 0;
}
