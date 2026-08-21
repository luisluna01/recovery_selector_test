#include "test_recovery_selector/behaviors/create_dummy_failure.hpp"


namespace test_recovery_selector
{

CreateDummyFailure::CreateDummyFailure(
  const std::string& name, const BT::NodeConfig& config, rclcpp::Node::SharedPtr node
):
  BT::SyncActionNode(name, config), node_(node)
{
  // Create subscriber_callback
  auto subscriber_callback = [this](const std_msgs::msg::String& message)
  {
    std::lock_guard<std::mutex> lock(data_mutex_);

    last_message_ = message; // Record the last message
    last_subscription_time_ = node_->now(); // Record the time
  };

  // Create subscriber with '/failure_source' topic
  subscriber_ = node_->create_subscription<std_msgs::msg::String>(
    "/failure_source", 10, subscriber_callback
  );
}


BT::PortsList CreateDummyFailure::providedPorts()
{
  return {
    BT::InputPort<double>("timeout", 5.0, "time to wait for subsriber to recieve message from topic"),
    BT::OutputPort<std::string>("failure_state", "failure state outputted as a string")
  };
}


BT::NodeStatus CreateDummyFailure::tick()
{
  
  // Validate timeout port
  BT::Expected<double> maybe_timeout = getInput<double>("timeout");
  if (!maybe_timeout)
  {
    throw BT::RuntimeError("missing required input port [timeout]: ",
      maybe_timeout.error());
  }

  // Initialize timeout port
  rclcpp::Duration timeout = rclcpp::Duration::from_seconds(maybe_timeout.value());

  // Protect last message and its time stamp from race condition
  std_msgs::msg::String last_message_copy;
  rclcpp::Time last_subscription_time_copy;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);

    last_message_copy = last_message_;
    last_subscription_time_copy = last_subscription_time_;
  }

  // Output message std::string data if last subscription was within desired time
  rclcpp::Duration elapsed_time = node_->now() - last_subscription_time_copy;
  if (elapsed_time < timeout)
  {
    // Output string message to blackboard
    setOutput("failure_state", last_message_copy.data);

    return BT::NodeStatus::SUCCESS;
  }
  
  RCLCPP_ERROR_STREAM(node_->get_logger(),
    "[CreateDummyFailure] has not received a new message within " << timeout.seconds() << "s...");

  return BT::NodeStatus::FAILURE;
}

} // namespace test_recovery_selector
