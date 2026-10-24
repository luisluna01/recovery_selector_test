#pragma once

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/action_node.h>

// Import the FAILURE_CASE type
#include "recovery_selector_test/failure_case_type.hpp"
#include "recovery_selector_test_msgs/msg/failure_case.hpp" // FailureCase message


namespace recovery_selector_test::behaviors
{

// This behavior acts as a fake child of the RecoverySelector that performs a process to recover
// from a failure. The failure case will be removed from the current failure state (what stores all
// failure cases) recorded by the Behavior Tree. The failure case it will recover from is listed in
// the constructor during registration of this behavior
// If a FailureCase message is received on the subscribed topic while running, the failure case is
// added to the failure state and the behavior returns FAILURE
class DummyRecoveryStrategy : public BT::StatefulActionNode
{
public:
  DummyRecoveryStrategy(
    const std::string& name,
    const BT::NodeConfig& config,
    const rclcpp::Node::SharedPtr& node,
    FailureCase failure_case,
    const std::string& topic = "/failure_source",
    double completion_time = 10.0
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
  rclcpp::Subscription<recovery_selector_test_msgs::msg::FailureCase>::SharedPtr subscriber_;

  rclcpp::Time completion_time_ros_;
  rclcpp::Time last_print_time_;
  rclcpp::Duration print_period_ = rclcpp::Duration::from_seconds(1.0);

  std::vector<FailureCase> failure_case_vec_; // Failure cases received by subscriber

  std::string topic_; // Topic name used for the subscriber
  FailureCase failure_case_; // Failure case dedicated to behavior instance
  double completion_time_; // Time for this behavior to run in seconds, set in constructor
  bool first_run_; // Whether onRunning() has not yet run since the last onStart()

  std::string failure_case_string_;
};

} // namespace recovery_selector_test::behaviors
