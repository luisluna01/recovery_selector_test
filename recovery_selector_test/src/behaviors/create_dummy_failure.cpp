#include "recovery_selector_test/behaviors/create_dummy_failure.hpp"


namespace recovery_selector_test::behaviors
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
    BT::BidirectionalPort<std::vector<FailureCase>>("failure_state", "failures present in the blackboard")
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
  failure_case_queue_.clear(); // Clear any messages recieved after spin_some()
  
  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus CreateDummyFailure::onRunning()
{
  // Add failure case to failure state and write to blackboard
  // Note: Guarantees thread safety by getting the data in the failure_state key in the blackboard
  // and protecting it with a mutex
  if(auto failure_state_locked = getLockedPortContent("failure_state"))
  {
    // If "failure_state" key has not been initialized, assign empty vector
    if(failure_state_locked->empty()) 
    {
      failure_state_locked.assign(std::vector<FailureCase>({})); // Assign empty vector
    }
    // Note: castPtr() access the value by pointer
    else if(std::vector<FailureCase>* failure_state_ptr = failure_state_locked->castPtr<std::vector<FailureCase>>())
    {
      executor_.spin_some(std::chrono::milliseconds(0));

      // Add failure cases subscribed to failure state
      for(const FailureCase& failure_case : failure_case_queue_)
      {
        // Only add it if it isn't already present
        if(std::find(failure_state_ptr->begin(), failure_state_ptr->end(), failure_case) == failure_state_ptr->end())
        {
          failure_state_ptr->push_back(failure_case);
          
          // Log what failure case was added for debugging
          std::string failure_case_str = failureCaseToString(failure_case);
          RCLCPP_DEBUG(
            node_->get_logger(),
            "[%s] added %s to [failure_state] port",
            this->name().c_str(), failure_case_str.c_str());
        }
      } 
    }
  }

  failure_case_queue_.clear(); // Clear message remaining

  return BT::NodeStatus::RUNNING;
}


void CreateDummyFailure::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str());
}


// ------------------------------------------------------------
// Subscriber Member Funtions
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

  subscriber_ = node_->create_subscription<recovery_selector_test_msgs::msg::FailureCase>(
    topic,
    rclcpp::QoS(10), /* For now only hold 10 failures at a time for now */
    std::bind(&CreateDummyFailure::listenerCallback, this, std::placeholders::_1),
    subscriber_options
  );
}


void CreateDummyFailure::listenerCallback(const recovery_selector_test_msgs::msg::FailureCase& msg)
{
  // Map the case id to the FailureCase enumerator
  FailureCase failure_case = static_cast<FailureCase>(msg.case_id);

  failure_case_queue_.push_back(failure_case);

  return;
}

} // namespace recovery_selector_test::behaviors
