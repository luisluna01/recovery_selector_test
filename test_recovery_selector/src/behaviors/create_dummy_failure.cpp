#include "test_recovery_selector/behaviors/create_dummy_failure.hpp"


namespace test_recovery_selector::behaviors
{

CreateDummyFailure::CreateDummyFailure(
  const std::string& name, const BT::NodeConfig& config, rclcpp::Node::SharedPtr node
):
  BT::StatefulActionNode(name, config), node_(node)
{
  // Validate [topic] port
  BT::Expected<std::string> maybe_topic = getInput<std::string>("topic");
  if(!maybe_topic)
  {
    throw BT::RuntimeError(
      "[CreateDummyFailure] invalid input port [topic]: ", maybe_topic.error());
  }
  std::string topic = maybe_topic.value();

  // Create callback group for subscriber. It will be used in a separate thread from the default ros
  // node executor's to avoid having to gaurd against concurrency
  callback_group_ = 
    node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive, false);
  
  rclcpp::SubscriptionOptions subscriber_options;
  subscriber_options.callback_group = callback_group_;
  
  executor_.add_callback_group(callback_group_, node_->get_node_base_interface());
  
  subscriber_ = node_->create_subscription<std_msgs::msg::String>(
    topic,
    rclcpp::QoS(10),
    [this](const std_msgs::msg::String& message) { last_message_ = message; },
    subscriber_options
  );
}


BT::PortsList CreateDummyFailure::providedPorts()
{
  return {
    BT::InputPort<std::string>("topic", "/failure_source", "topic to subscribe to"),
    BT::InputPort<double>("timeout", 5.0, "time to wait for subscriber to recieve message from topic in seconds"),
    BT::OutputPort<std::string>("failure_state", "string data to represent failure state")
  };
}


BT::NodeStatus CreateDummyFailure::onStart()
{
  // Verify [timeout] port
  BT::Expected<double> maybe_timeout = getInput<double>("timeout");
  if(!maybe_timeout)
  {
    throw BT::RuntimeError(
      "[CreateDummyFailure] invalid input port [timeout]: ", maybe_timeout.error());
  }
  double timeout = maybe_timeout.value();

  // Drain the messages from the qeue and clear last_message_
  // Note: max_duration = 0ms means no limit to how long node executer can spin
  executor_.spin_some(std::chrono::milliseconds(0));
  last_message_.reset(); // Clear any messages recieved after spin_some()
  
  // Create timeout
  timeout_set_ =  timeout > 0.0;
  if (timeout_set_)
  {
    timeout_end_ = node_->now() + rclcpp::Duration::from_seconds(timeout);
  }

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus CreateDummyFailure::onRunning()
{
  executor_.spin_some(std::chrono::milliseconds(0));

  if (last_message_)
  {
    setOutput("failure_state", last_message_->data);

    return BT::NodeStatus::SUCCESS;
  }

  if (timeout_set_ && node_->now() >= timeout_end_)
  {
    RCLCPP_ERROR(node_->get_logger(), "[CreateDummyFailure]: timed out waiting for message");

    return BT::NodeStatus::FAILURE;
  }

  return BT::NodeStatus::RUNNING;
}


void CreateDummyFailure::onHalted() {}

} // namespace test_recovery_selector::behaviors
