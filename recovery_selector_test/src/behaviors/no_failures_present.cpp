#include "recovery_selector_test/behaviors/no_failures_present.hpp"

namespace recovery_selector_test::behaviors
{

NoFailuresPresent::NoFailuresPresent(
  const std::string & name, const BT::NodeConfig & config, rclcpp::Node::SharedPtr node)
: BT::ConditionNode(name, config), node_(node)
{
}

BT::PortsList NoFailuresPresent::providedPorts()
{
  return {
    BT::InputPort<std::vector<FailureCase>>("failure_state", "Current failure cases outputted by behaviors")
  };
}

BT::NodeStatus NoFailuresPresent::tick()
{
  auto maybe_failure_state = getInput<std::vector<FailureCase>>("failure_state");
  if (!maybe_failure_state)
  {
    BT::RuntimeError("Invalid input port [failure_state]:", maybe_failure_state.error());
  }
  std::vector<FailureCase> failure_state = maybe_failure_state.value();

  if (failure_state.empty())
  {
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::FAILURE;
}

}  // namespace recovery_selector_test::behaviors
