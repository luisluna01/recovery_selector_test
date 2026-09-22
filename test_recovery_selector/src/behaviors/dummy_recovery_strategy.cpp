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
    BT::BidirectionalPort<std::vector<FailureCase>>("failure_state", "vector of failure cases present in failure state")
  };
}


BT::NodeStatus DummyRecoveryStrategy::onStart()
{
  // ---------- Verify bidirectional port is valid ---------- //
  // Verify [failure_state] port is valid
  BT::Expected<std::vector<FailureCase>> maybe_failure_state =
    getInput<std::vector<FailureCase>>("failure_state");
  if(!maybe_failure_state)
  {
    throw BT::RuntimeError(
      "invalid input port [failure_state]: ", maybe_failure_state.error());
  }
  // ---------- Verify bidirectional port is valid ---------- //

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
    // Remove all values of 'failure_case_' from [failure_state] port
    // Note: Read and write failure_state value directly while ensuring thread safety
    if(auto failure_state_locked = getLockedPortContent("failure_state"))
    {
      auto* failure_state_ptr = failure_state_locked->castPtr<std::vector<FailureCase>>();
      
      failure_state_ptr->erase(
        std::remove(failure_state_ptr->begin(), failure_state_ptr->end(), failure_case_),
        failure_state_ptr->end());
    }
    
    RCLCPP_INFO(
      node_->get_logger(),
      "[%s] successfully recovered from failure: %s!",
      this->name().c_str(), failure_case_string_.c_str());

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
