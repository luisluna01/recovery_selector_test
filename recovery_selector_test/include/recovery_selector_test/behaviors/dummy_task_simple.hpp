#pragma once

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/action_node.h>

#include <std_msgs/msg/bool.hpp> // Bool message


namespace recovery_selector_test::behaviors
{

// This behavior pretends to perform a task by running for a desired amount of time before
// completion. It reads a std_msgs/msg/Bool topic and outputs the latest value on the
// [failure_case_bool] port. The behavior returns FAILURE if the latest Bool message is true
// Note: Prints message every second
class DummyTaskSimple : public BT::StatefulActionNode
{
public:
  DummyTaskSimple(
    const std::string& name,
    const BT::NodeConfig& config,
    const rclcpp::Node::SharedPtr& node,
    const std::string& topic = "/failure_bool",
    double completion_time = 10.0,
    bool use_result_flag = false
  );

  static BT::PortsList providedPorts();

  virtual BT::NodeStatus onStart() override;

  virtual BT::NodeStatus onRunning() override;

  virtual void onHalted() override;


private:
  // Creates subscriber. Meant to be called on the first tick only, so a halt/re-tick cycle does not
  // create duplicate subscriber
  void createSubscriber();

  // Subscriber callback which reads and copies the Bool message
  void listenerCallback(const std_msgs::msg::Bool& msg);

  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr callback_group_;
  rclcpp::executors::SingleThreadedExecutor executor_;

  // Created once on the first tick
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subscriber_;

  rclcpp::Time completion_time_ros_;
  rclcpp::Time last_print_time_;
  rclcpp::Duration print_period_ = rclcpp::Duration::from_seconds(1.0);

  bool failure_case_bool_ = false; // Data of the latest Bool message received

  std::string topic_; // Topic name used for the subscriber
  double completion_time_; // Time for this behavior to run in seconds
  bool task_complete_; // Whether task has completed
  bool use_result_flag_; // Whether or not result port will be used
};

} // namespace recovery_selector_test::behaviors
