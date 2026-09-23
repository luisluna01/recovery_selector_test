#pragma once

#include <chrono>

#include <behaviortree_cpp/condition_node.h>
#include <rclcpp/rclcpp.hpp>


namespace recovery_selector_test::behaviors
{

class DummyCondition : public BT::ConditionNode
{
public:
  DummyCondition(
    const std::string& name, const BT::NodeConfig& config, rclcpp::Node::SharedPtr node
  );

  static BT::PortsList providedPorts();

  virtual BT::NodeStatus tick() override;


private:
  rclcpp::Node::SharedPtr node_;
};

} // namespace recovery_selector_test::behaviors
