#include "recovery_selector_test/behaviors/dummy_graceful_shutdown.hpp"


namespace recovery_selector_test::behaviors
{

DummyGracefulShutdown::DummyGracefulShutdown(
  const std::string& name,
  const BT::NodeConfig& config,
  const rclcpp::Node::SharedPtr node,
  double completion_time
):
  StatefulActionNode(name, config), node_(node), completion_time_(completion_time)
{}


BT::PortsList DummyGracefulShutdown::providedPorts()
{ 
  // Provide empty port to prevent compilation error
  return {};
}


BT::NodeStatus DummyGracefulShutdown::onStart()
{
  // Record time behavior should complete using ROS time
  completion_time_ros_ = node_->now() + rclcpp::Duration::from_seconds(completion_time_);

  last_print_time_ = node_->now();
  RCLCPP_WARN(
    node_->get_logger(),
    "[%s] failure is undefined! Performing graceful shutdown...",
    this->name().c_str());

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus DummyGracefulShutdown::onRunning()
{
  // Return SUCCESS if completion time reached
  if(node_->now() >= completion_time_ros_)
  {
    RCLCPP_INFO(node_->get_logger(), "[%s] Graceful shutdown complete!", this->name().c_str());
    return BT::NodeStatus::SUCCESS;
  }

  // Print every print_period_
  if((node_->now() - last_print_time_) >= print_period_)
  {
    last_print_time_ = node_->now();
    RCLCPP_WARN(
      node_->get_logger(),
      "[%s] failure is undefined! Performing graceful shutdown...",
      this->name().c_str());
  }

  return BT::NodeStatus::RUNNING;
}


void DummyGracefulShutdown::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str());
}

} // namespace recovery_selector_test::behaviors
