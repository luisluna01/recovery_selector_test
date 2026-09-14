#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/loggers/groot2_publisher.h>

#include <ament_index_cpp/get_package_share_directory.hpp>

// test_recovery_selector FailureCase type
#include "test_recovery_selector/failure_case_type.hpp"

// RecoverySelector behavior
#include "recovery_selector/recovery_selector.hpp"

// test_recovery_selector behaviors
#include "test_recovery_selector/behaviors/create_dummy_failure.hpp"
#include "test_recovery_selector/behaviors/dummy_task.hpp"
#include "test_recovery_selector/behaviors/dummy_recovery_strategy.hpp"
#include "test_recovery_selector/behaviors/dummy_condition.hpp"

// nrg_behaviors
#include "nrg_behaviors/nrg_behaviors.hpp"

using FailureCase = test_recovery_selector::FailureCase;

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::Node::SharedPtr ros_node = std::make_shared<rclcpp::Node>("test_recovery_selector_bt");

  BT::BehaviorTreeFactory factory; // Initialize BT factory which registers the tree and tree nodes

  // Register RecoverySelector to read 3 failure cases from the FailureCase type
  factory.registerNodeType<recovery_selector::RecoverySelector<FailureCase, 3>>("RecoverySelector");
  
  // ---------- Register test_recovery_selector behaviors ---------- //
  factory.registerNodeType<test_recovery_selector::behaviors::CreateDummyFailure>(
    "CreateDummyFailure", ros_node);
  
  factory.registerNodeType<test_recovery_selector::behaviors::DummyTask>("DummyTask", ros_node);

  factory.registerNodeType<test_recovery_selector::behaviors::DummyRecoveryStrategy>(
    "DummyRecoveryStrategyA", ros_node, FailureCase::LOW_BATTERY);
  
  factory.registerNodeType<test_recovery_selector::behaviors::DummyRecoveryStrategy>(
    "DummyRecoveryStrategyB", ros_node, FailureCase::MOTOR_FAILURE);

  factory.registerNodeType<test_recovery_selector::behaviors::DummyRecoveryStrategy>(
    "DummyRecoveryStrategyC", ros_node, FailureCase::FAILED_GRASP);

  factory.registerNodeType<test_recovery_selector::behaviors::DummyCondition>(
    "DummyCondition", ros_node);
  // ---------- Register test_recovery_selector behaviors ---------- //

  // Register nrg_utility_behaviors
  nrg_utility_behaviors::Config config;
  config.ros_node = ros_node; // Share ROS2 node with utility tree nodes
  nrg_utility_behaviors::registerBehaviors(factory, config);

  // Register test_recovery_selector FailureCase type
  factory.registerScriptingEnums<FailureCase>();

  // Create behavior tree
  std::string share_path = ament_index_cpp::get_package_share_directory("test_recovery_selector");
  BT::Tree tree = factory.createTreeFromFile(
    share_path + "/behavior_trees/test_recovery_selector.xml");

  // Initialize blackboard variables that are not intialized by the tree
  tree.rootBlackboard()->set<bool>("task_complete", false);
  
  // Connect to Groot2Publisher
  BT::Groot2Publisher publisher(tree);
  
  try
  {
    // Tick tree every 100ms and ensure SIGINT kills the tree
    BT::NodeStatus status = BT::NodeStatus::RUNNING;

    while (rclcpp::ok() && status == BT::NodeStatus::RUNNING)
    {
      status = tree.tickOnce();
      tree.sleep(std::chrono::milliseconds(100));
    }
  }
  catch (const std::exception& e)
  {
    // Output error through node logger
    RCLCPP_ERROR(ros_node->get_logger(), "%s", e.what());

    rclcpp::shutdown();
    
    return 1;
  }

  return 0;
}
