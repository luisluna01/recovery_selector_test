#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>

#include <ament_index_cpp/get_package_share_directory.hpp>

// RecoverySelector behavior
#include "recovery_selector/recovery_selector.hpp"

// test_recovery_selector behaviors
#include "test_recovery_selector/behaviors/create_dummy_failure.hpp"
#include "test_recovery_selector/behaviors/dummy_task.hpp"
#include "test_recovery_selector/behaviors/dummy_recovery_strategy.hpp"

// nrg_behaviors
#include "nrg_behaviors/nrg_behaviors.hpp"


namespace test_rs = test_recovery_selector::behaviors;

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::Node::SharedPtr ros_node = std::make_shared<rclcpp::Node>("test_recovery_selector_bt");

  BT::BehaviorTreeFactory factory; // Initialize BT factory which registers the tree and tree nodes

  // Register RecoverySelector with 3 failure_cases
  factory.registerNodeType<recovery_selector::RecoverySelector<3>>("RecoverySelector");

  // Register test_recovery_selector behaviors
  factory.registerNodeType<test_rs::CreateDummyFailure>("CreateDummyFailure", ros_node);
  factory.registerNodeType<test_rs::DummyTask>("DummyTask", ros_node);
  factory.registerNodeType<test_rs::DummyRecoveryStrategy>("DummyRecoveryStrategyA", ros_node, "a");
  factory.registerNodeType<test_rs::DummyRecoveryStrategy>("DummyRecoveryStrategyB", ros_node, "b");
  factory.registerNodeType<test_rs::DummyRecoveryStrategy>("DummyRecoveryStrategyC", ros_node, "c");

  // Register nrg_utility_behaviors
  nrg_utility_behaviors::Config config;
  config.ros_node = ros_node; // Share ROS2 node with utility tree nodes
  nrg_utility_behaviors::registerBehaviors(factory, config);

  // Create tree
  std::string share_path = ament_index_cpp::get_package_share_directory("test_recovery_selector");
  BT::Tree tree = factory.createTreeFromFile(
    share_path + "/behavior_trees/test_recovery_selector.xml");
  
  tree.tickWhileRunning(std::chrono::milliseconds(100));

  rclcpp::shutdown();

  return 0;
}
