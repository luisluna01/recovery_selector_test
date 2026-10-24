#include "recovery_selector_test/behaviors/dummy_recovery_strategy.hpp"


namespace recovery_selector_test::behaviors
{

DummyRecoveryStrategy::DummyRecoveryStrategy(
  const std::string& name,
  const BT::NodeConfig& config,
  const rclcpp::Node::SharedPtr& node,
  FailureCase failure_case,
  const std::string& topic,
  double completion_time
):
  BT::StatefulActionNode(name, config),
  node_(node),
  topic_(topic),
  failure_case_(failure_case),
  completion_time_(completion_time)
{}


BT::PortsList DummyRecoveryStrategy::providedPorts()
{
  return {
    BT::BidirectionalPort<std::vector<FailureCase>>("failure_state", "vector of failure cases present in failure state"),
    BT::OutputPort<bool>("result",
      "(Optional) Set to true when the recovery strategy completes successfully"),
    BT::OutputPort<bool>("condition_undone",
      "(Optional) Set to false on the first onRunning() call to mark the condition as undone")
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

  // Create subscriber on first tick only
  if(!subscriber_)
  {
    createSubscriber();
  }

  // Drain all messages queued while idle
  executor_.spin_all(std::chrono::milliseconds(0));
  failure_case_vec_.clear(); // Clear any messages received after spin_some()

  // Record time behavior should complete using ROS time
  completion_time_ros_ = node_->now() + rclcpp::Duration::from_seconds(completion_time_);

  last_print_time_ = node_->now();
  RCLCPP_INFO(
    node_->get_logger(),
    "[%s] recovering from failure: %s", this->name().c_str(), failure_case_string_.c_str());

  first_run_ = true;

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus DummyRecoveryStrategy::onRunning()
{
  // Mark the condition as undone on the first run only if the optional port is used
  // Note:  Not done in onStart() so a behavior halted right after starting leaves the blackboard
  //        unchanged
  if(first_run_)
  {
    first_run_ = false;

    if(setOutput("condition_undone", false))
    {
      RCLCPP_DEBUG(
        node_->get_logger(), "[%s] set [condition_undone] port to false", this->name().c_str());
    }
  }

  // Subscribe all messages from the queue
  executor_.spin_some(std::chrono::milliseconds(0));

  // Output first failure_case read from subscriber
  if(!failure_case_vec_.empty())
  {
    FailureCase new_failure_case = failure_case_vec_.front();

    // Add new case to [failure_state]
    // Note: Read and write failure_state value directly while ensuring thread safety
    if(auto failure_state_locked = getLockedPortContent("failure_state"))
    {
      auto* failure_state_ptr = failure_state_locked->castPtr<std::vector<FailureCase>>();
      failure_state_ptr->push_back(new_failure_case);
    }

    std::string new_failure_case_str = failureCaseToString(new_failure_case);
    RCLCPP_DEBUG(
      node_->get_logger(),
      "[%s] added %s to [failure_state] port",
      this->name().c_str(), new_failure_case_str.c_str());
    RCLCPP_WARN(
      node_->get_logger(),
      "[%s] failed to recover from %s", this->name().c_str(), failure_case_string_.c_str());

    failure_case_vec_.clear(); // Clear failure cases remaining

    return BT::NodeStatus::FAILURE;
  }

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
    
    // Set result to true if the optional port is used
    if(setOutput("result", true))
    {
      RCLCPP_DEBUG(
        node_->get_logger(), "[%s] set [result] port to true", this->name().c_str());
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

// ------------------------------------------------------------
// Helper Member Functions
// ------------------------------------------------------------

void DummyRecoveryStrategy::createSubscriber()
{
  // Create callback group for subscriber
  // Note: Invoked in current thread to avoid having to guard data in callback against concurrency
  callback_group_ =
    node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive, false);

  rclcpp::SubscriptionOptions subscriber_options;
  subscriber_options.callback_group = callback_group_;

  executor_.add_callback_group(callback_group_, node_->get_node_base_interface());

  subscriber_ = node_->create_subscription<recovery_selector_test_msgs::msg::FailureCase>(
    topic_,
    rclcpp::QoS(10), /* For now only hold 10 failures at a time */
    std::bind(&DummyRecoveryStrategy::listenerCallback, this, std::placeholders::_1),
    subscriber_options
  );
}


void DummyRecoveryStrategy::listenerCallback(
  const recovery_selector_test_msgs::msg::FailureCase& msg)
{
  // Map the case ID to the FailureCase enumerator
  failure_case_vec_.push_back(static_cast<FailureCase>(msg.case_id));
}

} // namespace recovery_selector_test::behaviors
