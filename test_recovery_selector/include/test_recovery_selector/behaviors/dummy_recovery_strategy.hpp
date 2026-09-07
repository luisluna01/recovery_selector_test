#pragma once

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/action_node.h>


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
    const std::string& failure_case
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

  std::string name_; // Name of behavior
  std::string failure_state_; // Failure state set from input port
  std::string failure_case_; // Failure case dedicated to behavior instance
};

} // namespace test_recovery_selector::behaviors
