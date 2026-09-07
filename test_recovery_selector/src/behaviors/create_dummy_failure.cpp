#include "test_recovery_selector/behaviors/create_dummy_failure.hpp"


namespace test_recovery_selector::behaviors
{

CreateDummyFailure::CreateDummyFailure(
  const std::string& name, const BT::NodeConfig& config, const rclcpp::Node::SharedPtr& node
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
    BT::OutputPort<std::string>("failure_state", "string data to represent failure state")
  };
}


BT::NodeStatus CreateDummyFailure::onStart()
{
  // Drain the messages from the qeue and clear last_message_
  // Note: max_duration = 0ms means no limit to how long node executer can spin
  executor_.spin_some(std::chrono::milliseconds(0));
  last_message_.reset(); // Clear any messages recieved after spin_some()
  
  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus CreateDummyFailure::onRunning()
{
  executor_.spin_some(std::chrono::milliseconds(0));

  if (last_message_)
  {
    setOutput("failure_state", last_message_->data);

    RCLCPP_DEBUG_THROTTLE(
      node_->get_logger(), *node_->get_clock(), 1000,
      "[CreateDummyFailure] outputted %s to [failure_state] port", last_message_->data.c_str());

    last_message_.reset(); // Clear message remaining
  }

  return BT::NodeStatus::RUNNING;
}


void CreateDummyFailure::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[CreateDummyFailure] halted");
}

} // namespace test_recovery_selector::behaviors
