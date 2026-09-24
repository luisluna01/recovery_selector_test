#pragma once

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/action_node.h>

#include "recovery_selector_test/failure_case_type.hpp" // FailureCase type
#include "recovery_selector_test_msgs/msg/failure_case.hpp" // FailureCase message


namespace recovery_selector_test::behaviors
{

// This behavior pretends to perform a task by running for a desired amount of time before
// completion
// Note: Prints message every second
class DummyTask : public BT::StatefulActionNode
{
public:
  DummyTask(
    const std::string& name,
    const BT::NodeConfig& config,
    const rclcpp::Node::SharedPtr& node,
    const std::string& topic = "/failure_source",
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

  // Subscriber callback which reads and copies the FailureCase message
  void listenerCallback(const recovery_selector_test_msgs::msg::FailureCase& msg);

  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr callback_group_;
  rclcpp::executors::SingleThreadedExecutor executor_;

  // Static to ensure only one subscriber is created across all DummyTask nodes
  rclcpp::Subscription<recovery_selector_test_msgs::msg::FailureCase>::SharedPtr subscriber_;

  rclcpp::Time completion_time_ros_;
  rclcpp::Time last_print_time_;
  rclcpp::Duration print_period_ = rclcpp::Duration::from_seconds(1.0);

  std::vector<FailureCase> failure_case_vec_;
  std::vector<FailureCase> failure_state_;

  std::string topic_; // Topic name used for the subscriber
  double completion_time_; // Time for this behavior to run in seconds
  bool use_result_flag_; // Whether or not result port will be used
};

} // namespace recovery_selector_test::behaviors
