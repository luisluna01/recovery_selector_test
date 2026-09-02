#include <chrono>

#include <rclcpp/rclcpp.hpp>

#include <std_msgs/msg/string.hpp>


namespace test_recovery_selector::nodes
{

class DummyFailurePublisher : public rclcpp::Node
{
public:
  DummyFailurePublisher()
  : rclcpp::Node("dummy_failure_publisher")
  {
    publisher = this->create_publisher<std_msgs::msg::String>("/failure_source", rclcpp::QoS(10));

    timer = this->create_wall_timer(
        std::chrono::milliseconds(500),
        std::bind(&DummyFailurePublisher::timer_callback, this)
      );
  };


private:
  void timer_callback() 
  {
    std_msgs::msg::String message;
    message.data = "a";

    publisher->publish(message);

    RCLCPP_INFO(this->get_logger(), "Publishing: \"%s\"", message.data.c_str());
  }
  
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher;
  rclcpp::TimerBase::SharedPtr timer;
};

} // namespace test_recovery_selector::nodes


int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<test_recovery_selector::nodes::DummyFailurePublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
