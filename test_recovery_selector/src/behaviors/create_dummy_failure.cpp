#include "test_recovery_selector/behaviors/create_dummy_failure.hpp"


namespace test_recovery_selector::behaviors
{

CreateDummyFailure::CreateDummyFailure(
  const std::string& name, const BT::NodeConfig& config, const rclcpp::Node::SharedPtr& node
):
  BT::StatefulActionNode(name, config), node_(node)
{}


BT::PortsList CreateDummyFailure::providedPorts()
{
  return {
    BT::InputPort<std::string>("topic", "/failure_source", "topic to subscribe to"),
    BT::OutputPort<FailureCase>("failure_state", "string data to represent failure state")
  };
}


BT::NodeStatus CreateDummyFailure::onStart()
{
  // Create the subscriber on first tick only
  if (!subscriber_)
  {
    createSubscriber();
  }

  // Drain the messages from the qeue and clear last_message_
  // Note: max_duration = 0ms means no limit to how long node executer can spin
  executor_.spin_some(std::chrono::milliseconds(0));
  last_failure_case_.reset(); // Clear any messages recieved after spin_some()
  
  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus CreateDummyFailure::onRunning()
{
  executor_.spin_some(std::chrono::milliseconds(0));

  if (last_failure_case_)
  {
    setOutput("failure_state", last_failure_case_.value());
    
    // Only for debugging
    std::string last_failure_case_str_ = failureCaseToString(last_failure_case_.value());
    RCLCPP_DEBUG_THROTTLE(
      node_->get_logger(), *node_->get_clock(), 1000,
      "[%s] outputted %s to [failure_state] port",
      this->name().c_str(), last_failure_case_str_.c_str());

    last_failure_case_.reset(); // Clear message remaining
  }

  return BT::NodeStatus::RUNNING;
}


void CreateDummyFailure::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str());
}


// ------------------------------------------------------------
// ------------------------------------------------------------


void CreateDummyFailure::createSubscriber()
{
  // Validate [topic] port
  BT::Expected<std::string> maybe_topic = getInput<std::string>("topic");
  if(!maybe_topic)
  {
    throw BT::RuntimeError(
      "invalid input port [topic]: ", maybe_topic.error());
  }
  std::string topic = maybe_topic.value();

  // Create callback group for subscriber. It will be used in a separate thread from the default ros
  // node executor's to avoid having to gaurd against concurrency
  callback_group_ =
    node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive, false);

  rclcpp::SubscriptionOptions subscriber_options;
  subscriber_options.callback_group = callback_group_;

  executor_.add_callback_group(callback_group_, node_->get_node_base_interface());

  subscriber_ = node_->create_subscription<test_recovery_selector_msgs::msg::FailureCase>(
    topic,
    rclcpp::QoS(10),
    std::bind(&CreateDummyFailure::timerCallback, this, std::placeholders::_1),
    subscriber_options
  );
}


void CreateDummyFailure::timerCallback(const test_recovery_selector_msgs::msg::FailureCase& msg)
{
  // Map the case id to the FailureCasee enumerator
  last_failure_case_ = static_cast<FailureCase>(msg.case_id);

  return;
}

} // namespace test_recovery_selector::behaviors
