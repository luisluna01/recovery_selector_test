#pragma once

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/action_node.h>


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
    double completion_time = 10.0,
    bool use_result_flag = false
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

  double completion_time_; // Time for this behavior to run in seconds, set in constructor
  bool use_result_flag_; // Whether or not result port will be used
};

} // namespace recovery_selector_test::behaviors
