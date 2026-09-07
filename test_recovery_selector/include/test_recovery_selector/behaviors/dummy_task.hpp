#pragma once

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/action_node.h>


namespace test_recovery_selector::behaviors
{

// This behavior pretends to perform a task by running for a desired amount of time before
// completion
// Note: Prints message every second
class DummyTask : public BT::StatefulActionNode
{
public:
  DummyTask(
    const std::string& name, const BT::NodeConfig& config, const rclcpp::Node::SharedPtr& node);

  static BT::PortsList providedPorts();

  virtual BT::NodeStatus onStart() override;

  virtual BT::NodeStatus onRunning() override;

  virtual void onHalted() override;


private:
  rclcpp::Node::SharedPtr node_;

  rclcpp::Time completion_time_ros_;
  rclcpp::Time last_print_time_;
  rclcpp::Duration print_period_ = rclcpp::Duration::from_seconds(1.0);
};

} // namespace test_recovery_selector::behaviors
