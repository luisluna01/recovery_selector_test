#pragma once

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/action_node.h>

#include <std_srvs/srv/trigger.hpp> // Trigger service


namespace recovery_selector_test::behaviors
{

// This behavior pretends to request help from a user. It creates a std_srvs/srv/Trigger service and
// returns RUNNING until the service is called. Once called, it outputs true on the
// [condition_resolved] port and returns SUCCESS on the following tick, so a condition node checked
// before this behavior is prioritized
// Note: Prints message every second
class RequestUserIntervention : public BT::StatefulActionNode
{
public:
  RequestUserIntervention(
    const std::string& name,
    const BT::NodeConfig& config,
    const rclcpp::Node::SharedPtr& node
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

  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr callback_group_;
  rclcpp::executors::SingleThreadedExecutor executor_;

  // Created on each start and reset on halt/resolution
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr service_;

  rclcpp::Time last_print_time_;
  rclcpp::Duration print_period_ = rclcpp::Duration::from_seconds(1.0);

  std::string service_name_; // Name of the service the user calls
  bool signal_received_ = false; // Whether the service has been called
  bool condition_resolved_ = false; // Whether [condition_resolved] has been output
};

} // namespace recovery_selector_test::behaviors
