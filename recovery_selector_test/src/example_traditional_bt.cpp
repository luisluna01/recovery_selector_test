#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/loggers/groot2_publisher.h>

#include <ament_index_cpp/get_package_share_directory.hpp>


// recovery_selector_test FailureCase type
#include "recovery_selector_test/failure_case_type.hpp"

// RecoverySelector behavior
#include "recovery_selector/recovery_selector.hpp"

// recovery_selector_test behaviors
#include "recovery_selector_test/behaviors/dummy_task.hpp"
#include "recovery_selector_test/behaviors/dummy_condition.hpp"

using recovery_selector_test::FailureCase;

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::Node::SharedPtr ros_node = std::make_shared<rclcpp::Node>("example_traditional_bt");

  BT::BehaviorTreeFactory factory; // Initialize BT factory which registers the tree and tree nodes
  
  // ---------- Register recovery_selector_test behaviors ---------- //
  // Register RecoverySelector to read 3 failure cases from the FailureCase type
  factory.registerNodeType<recovery_selector::RecoverySelector<FailureCase, 3>>("RecoverySelector");

  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "InRoom", ros_node);

  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "WalkInRoom", ros_node, "/walk_in_room", 10.0, true);
  // ---------- Register recovery_selector_test behaviors ---------- //
  
  // Register recovery_selector_test FailureCase type
  factory.registerScriptingEnums<FailureCase>();
  
  // Create behavior tree
  std::string share_path = ament_index_cpp::get_package_share_directory("recovery_selector_test");
  BT::Tree tree = factory.createTreeFromFile(
    share_path + "/behavior_trees/example_traditional_bt.xml");

  // Initialize blackboard keys read before any node has written them
  tree.rootBlackboard()->set<std::vector<FailureCase>>("failure_state", std::vector<FailureCase>());
  tree.rootBlackboard()->set<bool>("in_room", false);

  BT::Groot2Publisher publisher(tree); // Connect to Groot2Publisher

  try
  {
    // Tick tree every 100ms and ensure SIGINT kills the tree
    BT::NodeStatus status = BT::NodeStatus::RUNNING;

    while (rclcpp::ok() && status == BT::NodeStatus::RUNNING)
    {
      // // Print failure cases in failure state blackboard key
      // auto failure_state = tree.rootBlackboard()->get<std::vector<FailureCase>>("failure_state");
      // RCLCPP_INFO(ros_node->get_logger(), "failure_state: ");
      // for (const FailureCase& failure_case : failure_state)
      // {
      //   RCLCPP_INFO(
      //     ros_node->get_logger(),
      //     "  - %s",
      //     recovery_selector_test::failureCaseToString(failure_case).c_str());
      // }
      
      RCLCPP_INFO(ros_node->get_logger(), "ticking----");

      status = tree.tickOnce();
      tree.sleep(std::chrono::milliseconds(1000));

      RCLCPP_INFO(ros_node->get_logger(), "finished tick----\n");
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
