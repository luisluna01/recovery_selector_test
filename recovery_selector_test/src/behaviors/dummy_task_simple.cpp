#include "recovery_selector_test/behaviors/dummy_task_simple.hpp"


namespace recovery_selector_test::behaviors
{

DummyTaskSimple::DummyTaskSimple(
  const std::string& name,
  const BT::NodeConfig& config,
  const rclcpp::Node::SharedPtr& node,
  const std::string& topic,
  double completion_time,
  bool use_result_flag
):
  StatefulActionNode(name, config),
  node_(node),
  topic_(topic),
  completion_time_(completion_time),
  use_result_flag_(use_result_flag)
{}


BT::PortsList DummyTaskSimple::providedPorts()
{
  return{
    /* Implemented for condition nodes */
    BT::OutputPort<bool>("result", "Whether or not the task completed successfully"),
    BT::OutputPort<bool>("failure_case_bool",
      "Outputs false when a false Bool message is received. Never written otherwise")
  };
}


BT::NodeStatus DummyTaskSimple::onStart()
{
  task_complete_ = false;

  // Record time behavior should complete using ROS time
  completion_time_ros_ = node_->now() + rclcpp::Duration::from_seconds(completion_time_);

  // Create subscriber on first tick only
  if (!subscriber_)
  {
    createSubscriber();
  }

  // Drain the message on the topic and discard any false message received before this start
  executor_.spin_some(std::chrono::milliseconds(0));
  false_received_ = false;

  last_print_time_ = node_->now();
  RCLCPP_INFO(node_->get_logger(), "[%s] performing task...", this->name().c_str());

  if(use_result_flag_)
  {
    setOutput("result", false);
  }

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus DummyTaskSimple::onRunning()
{
  // Return SUCCESS if completion time reached
  // Note:  Placed before setting [result] to 'true' to make behavior return SUCCESS after one
  //        more tick
  if(task_complete_ == true)
  {
    RCLCPP_INFO(node_->get_logger(), "[%s] task complete!", this->name().c_str());

    return BT::NodeStatus::SUCCESS;
  }

  // Output result to 'true' if completion time reached
  if(node_->now() >= completion_time_ros_)
  {
    if(use_result_flag_)
    {
      setOutput("result", true);
    }

    task_complete_ = true;
  }

  // Read the current message on the topic
  executor_.spin_some(std::chrono::milliseconds(0));

  // Output false and return FAILURE if a false Bool message was received
  if(false_received_)
  {
    false_received_ = false;
    setOutput("failure_case_bool", false);

    RCLCPP_DEBUG(
      node_->get_logger(),
      "[%s] received false, set [failure_case_bool] port to false", this->name().c_str());

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


void DummyTaskSimple::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str());
}

// ------------------------------------------------------------
// Helper Member Functions
// ------------------------------------------------------------

void DummyTaskSimple::createSubscriber()
{
  // Create callback group for subscriber
  // Note: Invoked in current thread to avoid having to guard data in callback against concurrency
  callback_group_ =
    node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive, false);

  rclcpp::SubscriptionOptions subscriber_options;
  subscriber_options.callback_group = callback_group_;

  executor_.add_callback_group(callback_group_, node_->get_node_base_interface());

  subscriber_ = node_->create_subscription<std_msgs::msg::Bool>(
    topic_,
    rclcpp::QoS(1), /* Only the current Bool message is needed */
    std::bind(&DummyTaskSimple::listenerCallback, this, std::placeholders::_1),
    subscriber_options
  );
}


void DummyTaskSimple::listenerCallback(const std_msgs::msg::Bool& msg)
{
  // Only false messages are of interest. True messages are ignored
  if(!msg.data)
  {
    false_received_ = true;
  }

  return;
}

} // namespace recovery_selector_test::behaviors
