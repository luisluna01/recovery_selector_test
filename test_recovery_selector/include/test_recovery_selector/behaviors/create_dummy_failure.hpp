#pragma once

#include <chrono>

#include <behaviortree_cpp/action_node.h>
#include <rclcpp/rclcpp.hpp>

#include <std_msgs/msg/string.hpp>


namespace test_recovery_selector::behaviors
{

// This behavior subscribes a fake failure state as a String provided by a topic and writes it
// onto the blackboard.
// Note: This behavior is meant to run forever
class CreateDummyFailure : public BT::StatefulActionNode
{

public:
  CreateDummyFailure(
    const std::string& name, const BT::NodeConfig& config, const rclcpp::Node::SharedPtr& node
  );

  static BT::PortsList providedPorts();

  virtual BT::NodeStatus onStart() override;

  virtual BT::NodeStatus onRunning() override;

  virtual void onHalted() override;


private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr callback_group_;
  rclcpp::executors::SingleThreadedExecutor executor_; // Create SingleThreadedExecutor
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber_;

  std::optional<std_msgs::msg::String> last_message_;
  rclcpp::Time timeout_end_;

  bool timeout_set_ = false;
};

} // namespace test_recovery_selector
