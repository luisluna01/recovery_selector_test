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
    BT::InputPort<double>("completion_time", "60", "time for this behavior to run in seconds"),
    BT::InputPort<bool>("use_completion_flag", "false", "Whether or not to use completion_flag port"),
    BT::OutputPort<bool>("completion_flag", "Whether or not the task completed successfully")
  };
}


BT::NodeStatus DummyTask::onStart()
{
  // ---------- Verify Input Ports ---------- //
  // Verify input port [completion_time]
  BT::Expected<double> maybe_completion_time = getInput<double>("completion_time");
  if(!maybe_completion_time)
  {
    throw BT::RuntimeError(
      "invalid input port [completion_time]: ", maybe_completion_time.error());
  }
  double completion_time = maybe_completion_time.value();

  // Verify input port [use_completion_flag]
  BT::Expected<bool> maybe_use_completion_flag = getInput<bool>("use_completion_flag");
  if(!maybe_use_completion_flag)
  {
    throw BT::RuntimeError(
      "invalid input port [use_completion_flag]: ", maybe_use_completion_flag.error());
  }
  use_completion_flag_ = maybe_use_completion_flag.value();
  // ---------- Verify Input Ports ---------- //

  // Record time behavior should complete using ROS time
  completion_time_ros_ = node_->now() + rclcpp::Duration::from_seconds(completion_time);

  last_print_time_ = node_->now();
  RCLCPP_INFO(node_->get_logger(), "[%s] performing task...", this->name().c_str());

  if(use_completion_flag_)
  {
    setOutput("completion_flag", false);
  }
  return BT::NodeStatus::RUNNING;
}


BT::NodeStatus DummyTask::onRunning()
{
  // Return SUCCESS if completion time reached
  if(node_->now() >= completion_time_ros_)
  {
    RCLCPP_INFO(node_->get_logger(), "[%s] task complete!", this->name().c_str());

    if(use_completion_flag_)
    {
      setOutput("completion_flag", false);
    }
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
