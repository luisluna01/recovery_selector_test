#include "recovery_selector_test/behaviors/simple_request_user_intervention.hpp"


namespace recovery_selector_test::behaviors
{

SimpleRequestUserIntervention::SimpleRequestUserIntervention(
  const std::string& name,
  const BT::NodeConfig& config,
  const rclcpp::Node::SharedPtr& node
):
  StatefulActionNode(name, config),
  node_(node)
{}


BT::PortsList SimpleRequestUserIntervention::providedPorts()
{
  return{
    BT::InputPort<std::string>("service_name", "/user_intervention",
      "Trigger service the user calls to resolve the failure"),
    BT::OutputPort<bool>("condition_resolved",
      "Optional. Set to true once the user resolves the failure")
  };
}


BT::NodeStatus SimpleRequestUserIntervention::onStart()
{
  signal_received_ = false;
  condition_resolved_ = false;

  // Create callback group on first tick only
  // Note: Invoked in current thread to avoid having to guard data in callback against concurrency
  if (!callback_group_)
  {
    callback_group_ =
      node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive, false);
    executor_.add_callback_group(callback_group_, node_->get_node_base_interface());
  }

  // Validate [service_name] port
  auto maybe_service_name = getInput<std::string>("service_name");
  if(!maybe_service_name)
  {
    throw BT::RuntimeError(
      "invalid input port [service_name]: ", maybe_service_name.error());
  }
  service_name_ = maybe_service_name.value();

  service_ = node_->create_service<std_srvs::srv::Trigger>(
    service_name_,
    std::bind(
      &SimpleRequestUserIntervention::triggerCallback, this, std::placeholders::_1, std::placeholders::_2),
    rmw_qos_profile_services_default,
    callback_group_
  );

  last_print_time_ = node_->now();
  RCLCPP_INFO(
    node_->get_logger(), "[%s] waiting for user intervention on service \"%s\"...",
    this->name().c_str(), service_name_.c_str());

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus SimpleRequestUserIntervention::onRunning()
{
  // Return SUCCESS if the user has intervened
  // Note:  Placed before setting [condition_resolved] to 'true' to make behavior return SUCCESS
  //        after one more tick
  if(condition_resolved_ == true)
  {
    RCLCPP_INFO(node_->get_logger(), "[%s] user intervention complete!", this->name().c_str());

    return BT::NodeStatus::SUCCESS;
  }

  // Check for a service call
  executor_.spin_some(std::chrono::milliseconds(0));

  // Output [condition_resolved] as 'true' if the user intervened
  if(signal_received_)
  {
    setOutput("condition_resolved", true);
    service_.reset();
    condition_resolved_ = true;

    return BT::NodeStatus::RUNNING;
  }

  // Print every print_period_
  if((node_->now() - last_print_time_) >= print_period_)
  {
    last_print_time_ = node_->now();
    RCLCPP_INFO(
      node_->get_logger(), "[%s] waiting for user intervention on service \"%s\"...",
      this->name().c_str(), service_name_.c_str());
  }

  return BT::NodeStatus::RUNNING;
}


void SimpleRequestUserIntervention::onHalted()
{
  service_.reset();

  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str());
}

// ------------------------------------------------------------
// Helper Member Functions
// ------------------------------------------------------------

void SimpleRequestUserIntervention::triggerCallback(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> /*request*/,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  signal_received_ = true;

  response->success = true;
  response->message = "user intervention received";
}

} // namespace recovery_selector_test::behaviors
