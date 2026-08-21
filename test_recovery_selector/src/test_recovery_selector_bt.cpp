#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>

#include <ament_index_cpp/get_package_share_directory.hpp>

#include "nrg_behaviors/nrg_behaviors.hpp"
#include "test_recovery_selector/behaviors/create_dummy_failure.hpp"


int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::Node::SharedPtr ros_node = std::make_shared<rclcpp::Node>("test_recovery_selector_bt");

  BT::BehaviorTreeFactory factory; // Initialize BT factory which registers the tree and tree nodes

  // Register nrg_utility_behaviors
  nrg_utility_behaviors::Config config;
  config.ros_node = ros_node; // Share ROS2 node with utility tree nodes
  nrg_utility_behaviors::registerBehaviors(factory, config);

  // Create tree
  std::string share_path = ament_index_cpp::get_package_share_directory("test_recovery_selector");
  BT::Tree tree = factory.createTreeFromFile(
    share_path + "/behavior_trees/test_recovery_selector.xml");
  
  tree.tickWhileRunning();

  rclcpp::shutdown();

  return 0;
}
