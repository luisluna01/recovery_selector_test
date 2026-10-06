#pragma once

#include <behaviortree_cpp/condition_node.h>
#include <rclcpp/rclcpp.hpp>

#include "recovery_selector_test/failure_case_type.hpp"

namespace recovery_selector_test::behaviors
{

/**
 * @brief Condition node that reports whether any failure cases are currently present.
 *
 * Returns SUCCESS when no failures are present, FAILURE otherwise.
 */
class NoFailuresPresent : public BT::ConditionNode
{
public:
  NoFailuresPresent(
    const std::string & name, const BT::NodeConfig & config, rclcpp::Node::SharedPtr node);

  static BT::PortsList providedPorts();

  BT::NodeStatus tick() override;

private:
  rclcpp::Node::SharedPtr node_;
};

}  // namespace recovery_selector_test::behaviors
