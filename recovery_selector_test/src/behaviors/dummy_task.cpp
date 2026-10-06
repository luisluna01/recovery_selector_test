#include "recovery_selector_test/behaviors/dummy_task.hpp"


namespace recovery_selector_test::behaviors
{

DummyTask::DummyTask(
  const std::string& name,
  const BT::NodeConfig& config, 
  const rclcpp::Node::SharedPtr& node,
  const std::string& topic,
  double completion_time,
  bool use_result_flag,
  bool use_subscriber
):
  StatefulActionNode(name, config),
  node_(node),
  topic_(topic),
  completion_time_(completion_time),
  use_result_flag_(use_result_flag),
  use_subscriber_(use_subscriber)
{
  // [failure_state] port is only meaningful when failures are read from the subscriber
  const bool failure_state_in_xml =
    config.input_ports.count("failure_state") > 0 ||
    config.output_ports.count("failure_state") > 0;

  if(!use_subscriber_ && failure_state_in_xml)
  {
    throw BT::RuntimeError(
      "[", name, "]: port [failure_state] is set in the XML but use_subscriber is false");
  }
}


BT::PortsList DummyTask::providedPorts()
{
  return{
    /* Implemented for condition nodes */
    BT::OutputPort<bool>("result", "Whether or not the task completed successfully"),
    BT::BidirectionalPort<std::vector<FailureCase>>("failure_state",
      "failure cases present in the blackboard")
  };
}


BT::NodeStatus DummyTask::onStart()
{
  // Record time behavior should complete using ROS time
  completion_time_ros_ = node_->now() + rclcpp::Duration::from_seconds(completion_time_);

  if(use_subscriber_)
  {
    // Validate [failure_state] port
    auto maybe_failure_state = getInput<std::vector<FailureCase>>("failure_state");
    if(!maybe_failure_state)
    {
      throw BT::RuntimeError(
        "invalid input port [failure_state]: ", maybe_failure_state.error());
    }
    failure_state_ = maybe_failure_state.value();

    // Create subscriber on first tick only
    if (!subscriber_)
    {
      createSubscriber();
    }

    // Drain the messages from the queue and clear last_message_
    executor_.spin_some(std::chrono::milliseconds(0));
    failure_case_vec_.clear(); // Clear any messages received after spin_some()
  }

  last_print_time_ = node_->now();
  RCLCPP_INFO(node_->get_logger(), "[%s] performing task...", this->name().c_str());

  if(use_result_flag_)
  {
    setOutput("result", false);
  }
  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus DummyTask::onRunning()
{
  // Return SUCCESS if completion time reached
  if(node_->now() >= completion_time_ros_)
  {
    RCLCPP_INFO(node_->get_logger(), "[%s] task complete!", this->name().c_str());

    if(use_result_flag_)
    {
      setOutput("result", true);
    }
    return BT::NodeStatus::SUCCESS;
  }

  if(use_subscriber_)
  {
    // Subscribe all messages from the queue
    executor_.spin_some(std::chrono::milliseconds(0));
  }

  // Output first failure_case read from subscriber
  if(use_subscriber_ && !failure_case_vec_.empty())
  {
    // Get latest value in failure_state blackboard key
    failure_state_ = getInput<std::vector<FailureCase>>("failure_state").value();

    // Add new case to failure_state
    failure_state_.push_back(failure_case_vec_.front());
    setOutput("failure_state", failure_state_);

    // Log what failure case was added for debugging
    std::string failure_case_str = failureCaseToString(failure_case_vec_.front());
    RCLCPP_DEBUG(
      node_->get_logger(),
      "[%s] added %s to [failure_state] port",
      this->name().c_str(), failure_case_str.c_str());

    failure_case_vec_.clear(); // Clear failure cases remaining

    return BT::NodeStatus::FAILURE;
  }

  // Print every print_period_
  if((node_->now() - last_print_time_) >= print_period_)
  {
    last_print_time_ = node_->now();
    RCLCPP_INFO(node_->get_logger(), "[%s] performing task...", this->name().c_str());
  }

  return BT::NodeStatus::RUNNING;
}


void DummyTask::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str()); 
}

// ------------------------------------------------------------
// Helper Member Functions
// ------------------------------------------------------------

void DummyTask::createSubscriber()
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
    std::bind(&DummyTask::listenerCallback, this, std::placeholders::_1),
    subscriber_options
  );
}


void DummyTask::listenerCallback(const recovery_selector_test_msgs::msg::FailureCase& msg)
{
  // Map the case ID to the FailureCase enumerator
  FailureCase failure_case = static_cast<FailureCase>(msg.case_id);

  failure_case_vec_.push_back(failure_case);

  return;
}

} // namespace recovery_selector_test::behaviors
