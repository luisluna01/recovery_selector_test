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
  return { BT::InputPort<bool>("condition_bool", false, "Boolean determines if condition is true")};
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
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::FAILURE;
}

} // namespace recovery_selector_test::behaviors
