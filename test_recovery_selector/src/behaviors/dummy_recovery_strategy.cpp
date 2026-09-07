#include "test_recovery_selector/behaviors/dummy_recovery_strategy.hpp"

namespace test_recovery_selector::behaviors
{

DummyRecoveryStrategy::DummyRecoveryStrategy(
  const std::string& name,
  const BT::NodeConfig& config,
  const rclcpp::Node::SharedPtr& node,
  const std::string& failure_case
):
  BT::StatefulActionNode(name, config), node_(node), name_(name), failure_case_(failure_case)
{}


BT::PortsList DummyRecoveryStrategy::providedPorts()
{
  return {
    BT::BidirectionalPort<std::string>("failure_state", "string representing failure state"),
    BT::InputPort<double>("completion_time", "time for this behavior to run in seconds")
  };
}


BT::NodeStatus DummyRecoveryStrategy::onStart()
{
  // ---------- Verify input and bidirectional ports are valid ---------- //
  // Verify [failure_state] port is valid
  BT::Expected<std::string> maybe_failure_state = getInput<std::string>("failure_state");
  if(!maybe_failure_state)
  {
    throw BT::RuntimeError(
      "[", name_, "] invalid input port [failure_state]: ", maybe_failure_state.error());
  }
  failure_state_ = maybe_failure_state.value();

  // Verify [completion_time] port is valid
  BT::Expected<double> maybe_completion_time = getInput<double>("completion_time");
  if(!maybe_completion_time)
  {
    throw BT::RuntimeError(
      "[", name_, "] invalid input port [completion_time]: ", maybe_completion_time.error());
  }
  double completion_time = maybe_completion_time.value();
  // ---------- Verify input and bidirectional ports are valid ---------- //

  // Record time behavior should complete using ROS time
  completion_time_ros_ = node_->now() + rclcpp::Duration::from_seconds(completion_time);

  last_print_time_ = node_->now();
  RCLCPP_INFO(
    node_->get_logger(),
    "[%s] recovering from failure: \"%s\"", name_.c_str(), failure_case_.c_str());

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus DummyRecoveryStrategy::onRunning()
{
  // Return SUCCESS if completion time reached
  if(node_->now() >= completion_time_ros_)
  {
    failure_state_.clear(); // Remove failue case from failure_state
    RCLCPP_WARN(node_->get_logger(), "TESTING:%s", failure_state_.c_str());

    setOutput("failure_state", failure_state_); // Output updated failure state

    RCLCPP_INFO(node_->get_logger(), "[%s] successfully recovered!", name_.c_str());

    return BT::NodeStatus::SUCCESS;
  }

  // Print every print_period_
  if((node_->now() - last_print_time_) >= print_period_)
  {
    last_print_time_ = node_->now();
    RCLCPP_INFO(
      node_->get_logger(),
      "[%s] recovering from failure: \"%s\"", name_.c_str(), failure_case_.c_str());
  }
  
  return BT::NodeStatus::RUNNING;
}


void DummyRecoveryStrategy:: onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[%s] halted", name_.c_str());
}

} // namespace test_recovery_selector::behaviors
