#include "recovery_selector_test/behaviors/dummy_condition.hpp"


namespace recovery_selector_test::behaviors
{

DummyCondition::DummyCondition(
  const std::string& name, const BT::NodeConfig& config, rclcpp::Node::SharedPtr node
):
  BT::ConditionNode(name, config), node_(node)
{}


BT::PortsList DummyCondition::providedPorts()
{
  return { BT::InputPort<bool>("condition_bool", "Boolean determines if condition is true")};
}


BT::NodeStatus DummyCondition::tick()
{
  // Verify input port [condition_bool]
  BT::Expected<bool> maybe_condition_bool = getInput<bool>("condition_bool");
  if (!maybe_condition_bool)
  {
    throw BT::RuntimeError("invalid input port [condition_bool]:", maybe_condition_bool.error());
  }
  bool condition_bool = maybe_condition_bool.value();

  if(condition_bool)
  {
    RCLCPP_DEBUG_THROTTLE(
      node_->get_logger(), *node_->get_clock(), 1000,
      "[%s] condition is true", this->name().c_str());
    return BT::NodeStatus::SUCCESS;
  }

  RCLCPP_DEBUG_THROTTLE(
    node_->get_logger(), *node_->get_clock(), 1000,
    "[%s] condition is false", this->name().c_str());
  return BT::NodeStatus::FAILURE;
}

} // namespace recovery_selector_test::behaviors
