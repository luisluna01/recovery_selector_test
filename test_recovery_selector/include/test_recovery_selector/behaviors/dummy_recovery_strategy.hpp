#pragma once

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/action_node.h>

// Import the FAILURE_CASE type
#include "test_recovery_selector/failure_case_type.hpp"


namespace test_recovery_selector::behaviors
{

// This behavior acts as a fake child of the RecoverySelector that performs a process to recover
// from a failure. The failure case will be removed from the current failure state (what stores all
// failure cases) recorded by the Behavior Tree. The failure case it will recover from is listed in
// the constructor during registration of this behavior
class DummyRecoveryStrategy : public BT::StatefulActionNode
{
public:
  DummyRecoveryStrategy(
    const std::string& name,
    const BT::NodeConfig& config,
    const rclcpp::Node::SharedPtr& node,
    FailureCase failure_case,
    double completion_time = 10.0
  );

  static BT::PortsList providedPorts();

  virtual BT::NodeStatus onStart() override;

  virtual BT::NodeStatus onRunning() override;

  virtual void onHalted() override;


private:
  rclcpp::Node::SharedPtr node_;

  rclcpp::Time completion_time_ros_;
  rclcpp::Time last_print_time_;
  rclcpp::Duration print_period_ = rclcpp::Duration::from_seconds(1.0);

  std::vector<FailureCase> failure_state_; // Failure state set from input port
  FailureCase failure_case_; // Failure case dedicated to behavior instance
  double completion_time_; // Time for this behavior to run in seconds, set in constructor

  std::string failure_case_string_;
};

} // namespace test_recovery_selector::behaviors
