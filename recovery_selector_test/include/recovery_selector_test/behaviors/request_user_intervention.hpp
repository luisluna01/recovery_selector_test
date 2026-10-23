#pragma once

#include <optional>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/action_node.h>

#include <std_srvs/srv/trigger.hpp> // Trigger service

// Import the FAILURE_CASE type
#include "recovery_selector_test/failure_case_type.hpp"


namespace recovery_selector_test::behaviors
{

// This behavior pretends to request help from a user for failure cases not registered by the
// [case_n] input ports of the RecoverySelector. It creates a std_srvs/srv/Trigger service and
// returns RUNNING until the service is called. Once called, it removes the first occurrence of the
// highest priority unregistered failure case from the [failure_state] port and returns SUCCESS. The
// unregistered failure cases are listed in order of priority in the constructor during registration
// of this behavior
// Note: Prints message every second
class RequestUserIntervention : public BT::StatefulActionNode
{
public:
  RequestUserIntervention(
    const std::string& name,
    const BT::NodeConfig& config,
    const rclcpp::Node::SharedPtr& node,
    const std::vector<FailureCase>& unregistered_cases
  );

  static BT::PortsList providedPorts();

  virtual BT::NodeStatus onStart() override;

  virtual BT::NodeStatus onRunning() override;

  virtual void onHalted() override;


private:
  // Service callback which records that the user has intervened
  void triggerCallback(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response);

  // Returns the highest priority unregistered failure case present in the failure state
  std::optional<FailureCase> findTargetCase(const std::vector<FailureCase>& failure_state) const;

  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr callback_group_;
  rclcpp::executors::SingleThreadedExecutor executor_;

  // Created on each start and reset on halt/resolution
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr service_;

  rclcpp::Time last_print_time_;
  rclcpp::Duration print_period_ = rclcpp::Duration::from_seconds(1.0);

  std::vector<FailureCase> unregistered_cases_; // Unregistered failure cases in order of priority

  std::string service_name_; // Name of the service the user calls
  bool signal_received_ = false; // Whether the service has been called
};

} // namespace recovery_selector_test::behaviors
