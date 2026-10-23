#include "recovery_selector_test/behaviors/request_user_intervention.hpp"

#include <algorithm>


namespace recovery_selector_test::behaviors
{

RequestUserIntervention::RequestUserIntervention(
  const std::string& name,
  const BT::NodeConfig& config,
  const rclcpp::Node::SharedPtr& node,
  const std::vector<FailureCase>& unregistered_cases
):
  StatefulActionNode(name, config),
  node_(node),
  unregistered_cases_(unregistered_cases)
{}


BT::PortsList RequestUserIntervention::providedPorts()
{
  return{
    BT::InputPort<std::string>("service_name", "/user_intervention",
      "Trigger service the user calls to resolve the failure"),
    BT::BidirectionalPort<std::vector<FailureCase>>("failure_state",
      "vector of failure cases present in failure state")
  };
}


BT::NodeStatus RequestUserIntervention::onStart()
{
  signal_received_ = false;

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

  // Return FAILURE if no unregistered failure case is present in [failure_state]
  std::optional<FailureCase> target_case = findTargetCase(maybe_failure_state.value());
  if(!target_case)
  {
    RCLCPP_WARN(
      node_->get_logger(), "[%s] no unregistered failure case present in [failure_state]",
      this->name().c_str());

    return BT::NodeStatus::FAILURE;
  }

  service_ = node_->create_service<std_srvs::srv::Trigger>(
    service_name_,
    std::bind(
      &RequestUserIntervention::triggerCallback, this, std::placeholders::_1, std::placeholders::_2),
    rmw_qos_profile_services_default,
    callback_group_
  );

  last_print_time_ = node_->now();
  RCLCPP_INFO(
    node_->get_logger(), "[%s] waiting for user intervention on service \"%s\" to resolve %s...",
    this->name().c_str(), service_name_.c_str(), failureCaseToString(*target_case).c_str());

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus RequestUserIntervention::onRunning()
{
  // Check for a service call
  executor_.spin_some(std::chrono::milliseconds(0));

  // Remove the highest priority unregistered failure case if the user intervened
  if(signal_received_)
  {
    service_.reset();

    std::optional<FailureCase> resolved_case;

    // Remove first occurrence of target case from [failure_state] port
    // Note: Read and write failure_state value directly while ensuring thread safety
    if(auto failure_state_locked = getLockedPortContent("failure_state"))
    {
      auto* failure_state_ptr = failure_state_locked->castPtr<std::vector<FailureCase>>();

      resolved_case = findTargetCase(*failure_state_ptr);
      if(resolved_case)
      {
        failure_state_ptr->erase(
          std::find(failure_state_ptr->begin(), failure_state_ptr->end(), *resolved_case));
      }
    }

    if(resolved_case)
    {
      RCLCPP_INFO(
        node_->get_logger(), "[%s] user intervention resolved %s!",
        this->name().c_str(), failureCaseToString(*resolved_case).c_str());
    }
    else
    {
      RCLCPP_INFO(
        node_->get_logger(), "[%s] user intervention complete!", this->name().c_str());
    }

    return BT::NodeStatus::SUCCESS;
  }

  // Print every print_period_
  if((node_->now() - last_print_time_) >= print_period_)
  {
    last_print_time_ = node_->now();

    std::string target_case_string = "unregistered failure";
    auto maybe_failure_state = getInput<std::vector<FailureCase>>("failure_state");
    if(maybe_failure_state)
    {
      if(std::optional<FailureCase> target_case = findTargetCase(maybe_failure_state.value()))
      {
        target_case_string = failureCaseToString(*target_case);
      }
    }

    RCLCPP_INFO(
      node_->get_logger(), "[%s] waiting for user intervention on service \"%s\" to resolve %s...",
      this->name().c_str(), service_name_.c_str(), target_case_string.c_str());
  }

  return BT::NodeStatus::RUNNING;
}


void RequestUserIntervention::onHalted()
{
  service_.reset();

  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str());
}

// ------------------------------------------------------------
// Helper Member Functions
// ------------------------------------------------------------

void RequestUserIntervention::triggerCallback(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> /*request*/,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  signal_received_ = true;

  response->success = true;
  response->message = "user intervention received";
}


std::optional<FailureCase> RequestUserIntervention::findTargetCase(
  const std::vector<FailureCase>& failure_state) const
{
  // Return the first unregistered failure case (highest priority) found in the failure state
  for(const FailureCase& unregistered_case : unregistered_cases_)
  {
    if(std::find(failure_state.begin(), failure_state.end(), unregistered_case) !=
       failure_state.end())
    {
      return unregistered_case;
    }
  }

  return std::nullopt;
}

} // namespace recovery_selector_test::behaviors
