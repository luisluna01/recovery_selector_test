#include <filesystem>

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/loggers/groot2_publisher.h>
#include <behaviortree_cpp/loggers/bt_cout_logger.h>

#include <ament_index_cpp/get_package_share_directory.hpp>

// recovery_selector_test FailureCase type
#include "recovery_selector_test/failure_case_type.hpp"
#include "recovery_selector_test/write_tree_nodes_model.hpp"

// RecoverySelector behavior
#include "recovery_selector/recovery_selector.hpp"

// recovery_selector_test behaviors
#include "recovery_selector_test/behaviors/dummy_task.hpp"
#include "recovery_selector_test/behaviors/simple_dummy_task.hpp"
#include "recovery_selector_test/behaviors/dummy_recovery_strategy.hpp"
#include "recovery_selector_test/behaviors/dummy_condition.hpp"
#include "recovery_selector_test/behaviors/simple_request_user_intervention.hpp"
#include "recovery_selector_test/behaviors/request_user_intervention.hpp"

// nrg_behaviors
#include "nrg_behaviors/nrg_behaviors.hpp"

using recovery_selector_test::FailureCase;

// Directory the TreeNodesModel XMLs are written to for Groot2 (<package>/config)
static const std::string CONFIG_DIR =
  (std::filesystem::path(__FILE__).parent_path().parent_path() / "config").string();

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::Node::SharedPtr ros_node = std::make_shared<rclcpp::Node>("example_recovery_selector_bt");

  BT::BehaviorTreeFactory factory; // Initialize BT factory which registers the tree and tree nodes
  
  // ---------- Register recovery_selector_test behaviors ---------- //
  // Register RecoverySelector to read 3 failure cases from the FailureCase type
  factory.registerNodeType<recovery_selector::RecoverySelector<FailureCase, 3>>("RecoverySelector");

  // Register recovery_selector_test::behaviors condition nodes
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "InRoom", ros_node);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "DoorOpen", ros_node);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "NearDoor", ros_node);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "HandleTurned", ros_node);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "HandleGrasped", ros_node);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "GripperStowed", ros_node);
  
  // Register recovery_selector_test::behaviors::SimpleDummyTask nodes
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "WalkInRoom", ros_node, "/failure_source", 10.0, true);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyRecoveryStrategy>(
    "PlanObstacleFreePath", ros_node, FailureCase::OBSTACLE_IN_PATH, "/failure_source", 10.0);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "WalkToDoor", ros_node, "/failure_source", 10.0, true);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyRecoveryStrategy>(
    "UnlockDoor", ros_node, FailureCase::DOOR_LOCKED, "/failure_source", 10.0);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "PushDoorOpen", ros_node, "/failure_source", 10.0, true);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "TurnHandle", ros_node, "/failure_source", 10.0, true);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "GraspHandle", ros_node, "/failure_source", 10.0, true);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "ReleaseAndStowGripper", ros_node, "/failure_source", 5.0, true);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyRecoveryStrategy>(
    "SlowlyRegraspHandle", ros_node, FailureCase::FAILED_GRASP, "/failure_source", 10.0);
  
  // Register recovery_selector_test::behaviors::SimpleRequestUserIntervention
  factory.registerNodeType<recovery_selector_test::behaviors::SimpleRequestUserIntervention>(
    "SimpleRequestUserIntervention", ros_node);

  // FailureCases unregistered by input ports in RecoverySelector
  std::vector<FailureCase> unregistered_cases = {
    FailureCase::UNDEFINED_FAILURE,
    FailureCase::LOW_BATTERY,
    FailureCase::MOTOR_FAILURE,
    FailureCase::COLLISION
  };
  // Register recovery_selector_test::behaviors::RequestUserIntervention
  factory.registerNodeType<recovery_selector_test::behaviors::RequestUserIntervention>(
    "RequestUserIntervention", ros_node, unregistered_cases);
  // ---------- Register recovery_selector_test behaviors ---------- //
  
  // Register nrg_utility_behaviors
  nrg_utility_behaviors::Config config;
  config.ros_node = ros_node; // Share ROS2 node with utility tree nodes
  nrg_utility_behaviors::registerBehaviors(factory, config);

  // Register recovery_selector_test FailureCase type
  factory.registerScriptingEnums<FailureCase>();
  
  // Create behavior tree
  std::string share_path = ament_index_cpp::get_package_share_directory("recovery_selector_test");
  const std::string bt_xml_name = "example_recovery_selector_bt.xml";
  BT::Tree tree = factory.createTreeFromFile(share_path + "/behavior_trees/" + bt_xml_name);

  // Write tree and TreeNodesModel XML to config/ for Groot2
  recovery_selector_test::writeTreeNodesModel(
    tree, bt_xml_name, CONFIG_DIR, ros_node->get_logger());
  
  BT::Groot2Publisher publisher(tree); // Connect to Groot2Publisher

  // BT::StdCoutLogger logger(tree); // Log status of tree during each tick

  // Initialize blackboard keys read before any node has written them
  tree.rootBlackboard()->set<std::vector<FailureCase>>("failure_state", std::vector<FailureCase>());
  tree.rootBlackboard()->set<bool>("in_room", false);
  tree.rootBlackboard()->set<bool>("door_open", false);
  tree.rootBlackboard()->set<bool>("near_door", false);
  tree.rootBlackboard()->set<bool>("handle_grasped", false);
  tree.rootBlackboard()->set<bool>("handle_turned", false);
  tree.rootBlackboard()->set<bool>("gripper_stowed", true);

  try
  {
    // Tick tree every 100ms and ensure SIGINT kills the tree
    BT::NodeStatus status = BT::NodeStatus::RUNNING;

    while (rclcpp::ok() && status == BT::NodeStatus::RUNNING)
    {
      // Print failure cases in failure state blackboard key
      auto failure_state = tree.rootBlackboard()->get<std::vector<FailureCase>>("failure_state");
      RCLCPP_INFO(ros_node->get_logger(), "failure_state: ");
      for (const FailureCase& failure_case : failure_state)
      {
        RCLCPP_INFO(
          ros_node->get_logger(),
          "  - %s",
          recovery_selector_test::failureCaseToString(failure_case).c_str());
      }

      RCLCPP_INFO(ros_node->get_logger(), "ticking----");

      status = tree.tickOnce();
      tree.sleep(std::chrono::milliseconds(1000));

      RCLCPP_INFO(ros_node->get_logger(), "finished tick----\n");
    }

      RCLCPP_INFO(ros_node->get_logger(), "Returned Status: %s\n", BT::toStr(status).c_str());
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
