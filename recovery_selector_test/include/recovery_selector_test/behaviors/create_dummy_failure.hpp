#pragma once

#include <chrono>
#include <functional>

#include <behaviortree_cpp/action_node.h>
#include <rclcpp/rclcpp.hpp>

// FailureCase type
#include "recovery_selector_test/failure_case_type.hpp"

// FailureCase message
#include "recovery_selector_test_msgs/msg/failure_case.hpp"


namespace recovery_selector_test::behaviors
{

// This behavior subscribes a fake failure state as a FailureCase enum class type provided by a
// topic and writes it onto the blackboard
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
  void listenerCallback(const recovery_selector_test_msgs::msg::FailureCase& msg);

  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr callback_group_;
  rclcpp::executors::SingleThreadedExecutor executor_; // Create SingleThreadedExecutor
  rclcpp::Subscription<recovery_selector_test_msgs::msg::FailureCase>::SharedPtr subscriber_;

  std::vector<FailureCase> failure_case_queue_;
};

} // namespace recovery_selector_test::behaviors
