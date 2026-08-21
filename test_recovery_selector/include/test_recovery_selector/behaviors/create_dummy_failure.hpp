#pragma once

#include <mutex>
#include <chrono>

#include <behaviortree_cpp/action_node.h>
#include <rclcpp/rclcpp.hpp>

#include <std_msgs/msg/string.hpp>


namespace test_recovery_selector
{

// This node creates a fake failure state that is written on the blackboard. It reads the failure
// state as a string message provided by a topic in ROS2 and writes the failure state onto a string
// on the blackboard
class CreateDummyFailure : public BT::SyncActionNode
{

public:
  CreateDummyFailure(
    const std::string& name, const BT::NodeConfig& config, rclcpp::Node::SharedPtr
  );

  static BT::PortsList providedPorts();

  virtual BT::NodeStatus tick() override;


private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber_;

  std_msgs::msg::String last_message_;
  rclcpp::Time last_subscription_time_;

  std::mutex data_mutex_;
};

} // namespace test_recovery_selector
