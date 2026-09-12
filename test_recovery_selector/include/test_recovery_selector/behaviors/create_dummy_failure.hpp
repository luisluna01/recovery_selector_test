#pragma once

#include <chrono>
#include <functional>

#include <behaviortree_cpp/action_node.h>
#include <rclcpp/rclcpp.hpp>

// FailureCase type
#include "test_recovery_selector/failure_case_type.hpp"

// FailureCase message
#include "test_recovery_selector_msgs/msg/failure_case.hpp"


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
  // Reads [topic] port and creates subscriber. Meant to be called on the first tick only, so
  // a halt/re-tick cycle does not create duplicate subscriber
  void createSubscriber();

  // Subscriber callback which reads and copies the FailureCase message
  void timerCallback(const test_recovery_selector_msgs::msg::FailureCase& msg);

  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr callback_group_;
  rclcpp::executors::SingleThreadedExecutor executor_; // Create SingleThreadedExecutor
  rclcpp::Subscription<test_recovery_selector_msgs::msg::FailureCase>::SharedPtr subscriber_;

  std::optional<FailureCase> last_failure_case_;
};

} // namespace test_recovery_selector
