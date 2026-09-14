#include "test_recovery_selector/behaviors/dummy_recovery_strategy.hpp"


namespace test_recovery_selector::behaviors
{

DummyRecoveryStrategy::DummyRecoveryStrategy(
  const std::string& name,
  const BT::NodeConfig& config,
  const rclcpp::Node::SharedPtr& node,
  FailureCase failure_case,
  double completion_time
):
  BT::StatefulActionNode(name, config),
  node_(node),
  failure_case_(failure_case),
  completion_time_(completion_time)
{}


BT::PortsList DummyRecoveryStrategy::providedPorts()
{
  return {
    BT::BidirectionalPort<FailureCase>("failure_state", "string representing failure state")
  };
}


BT::NodeStatus DummyRecoveryStrategy::onStart()
{
  // ---------- Verify input and bidirectional ports are valid ---------- //
  // Verify [failure_state] port is valid
  BT::Expected<FailureCase> maybe_failure_state = getInput<FailureCase>("failure_state");
  if(!maybe_failure_state)
  {
    throw BT::RuntimeError(
      "invalid input port [failure_state]: ", maybe_failure_state.error());
  }
  failure_state_ = maybe_failure_state.value();
  // ---------- Verify input and bidirectional ports are valid ---------- //

  failure_case_string_ = failureCaseToString(failure_case_);

  // Record time behavior should complete using ROS time
  completion_time_ros_ = node_->now() + rclcpp::Duration::from_seconds(completion_time_);

  last_print_time_ = node_->now();
  RCLCPP_INFO(
    node_->get_logger(),
    "[%s] recovering from failure: %s", this->name().c_str(), failure_case_string_.c_str());

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus DummyRecoveryStrategy::onRunning()
{
  // Return SUCCESS if completion time reached
  if(node_->now() >= completion_time_ros_)
  {
    failure_state_ = FailureCase::NO_FAILURE; // Remove failue case from failure_state

    setOutput("failure_state", failure_state_); // Output updated failure state

    RCLCPP_INFO(
      node_->get_logger(),
      "[%s] successfully recovered from failure: %s!", this->name().c_str(), failure_case_string_.c_str());

    return BT::NodeStatus::SUCCESS;
  }

  // Print every print_period_
  if((node_->now() - last_print_time_) >= print_period_)
  {
    last_print_time_ = node_->now();
    RCLCPP_INFO(
      node_->get_logger(),
      "[%s] recovering from failure: %s", this->name().c_str(), failure_case_string_.c_str());
  }
  
  return BT::NodeStatus::RUNNING;
}


void DummyRecoveryStrategy:: onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str());
}

} // namespace test_recovery_selector::behaviors
