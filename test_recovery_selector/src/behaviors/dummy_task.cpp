#include "test_recovery_selector/behaviors/dummy_task.hpp"


namespace test_recovery_selector::behaviors
{

DummyTask::DummyTask(
  const std::string& name, const BT::NodeConfig& config, const rclcpp::Node::SharedPtr& node
):
  StatefulActionNode(name, config), node_(node)
{}


BT::PortsList DummyTask::providedPorts()
{
  return{
    BT::InputPort<double>("completion_time", "60", "time for this behavior to run in seconds")
  };
}


BT::NodeStatus DummyTask::onStart()
{
  // Verify input port is valid
  BT::Expected<double> maybe_completion_time = getInput<double>("completion_time");
  if(!maybe_completion_time)
  {
    throw BT::RuntimeError(
      "invalid input port [completion_time]: ", maybe_completion_time.error());
  }
  double completion_time = maybe_completion_time.value();

  // Record time behavior should complete using ROS time
  completion_time_ros_ = node_->now() + rclcpp::Duration::from_seconds(completion_time);

  last_print_time_ = node_->now();
  RCLCPP_INFO(node_->get_logger(), "[%s] performing task...", this->name().c_str());

  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus DummyTask::onRunning()
{
  // Return SUCCESS if completion time reached
  if(node_->now() >= completion_time_ros_)
  {
    RCLCPP_INFO(node_->get_logger(), "[%s] task complete!", this->name().c_str());
    return BT::NodeStatus::SUCCESS;
  }

  // Print every print_period_
  if((node_->now() - last_print_time_) >= print_period_)
  {
    last_print_time_ = node_->now();
    RCLCPP_INFO(node_->get_logger(), "[%s] performing task...", this->name().c_str());
  }

  return BT::NodeStatus::RUNNING;
}


void DummyTask::onHalted()
{
  RCLCPP_WARN(node_->get_logger(), "[%s] halted", this->name().c_str()); 
}

} // namespace test_recovery_selector::behaviors
