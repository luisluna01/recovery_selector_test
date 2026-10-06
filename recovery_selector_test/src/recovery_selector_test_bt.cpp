#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/loggers/groot2_publisher.h>
#include <behaviortree_cpp/loggers/bt_cout_logger.h>

#include <ament_index_cpp/get_package_share_directory.hpp>

// recovery_selector_test FailureCase type
#include "recovery_selector_test/failure_case_type.hpp"

// RecoverySelector behavior
#include "recovery_selector/recovery_selector.hpp"

// recovery_selector_test behaviors
#include "recovery_selector_test/behaviors/create_dummy_failure.hpp"
#include "recovery_selector_test/behaviors/dummy_task.hpp"
#include "recovery_selector_test/behaviors/dummy_recovery_strategy.hpp"
#include "recovery_selector_test/behaviors/dummy_condition.hpp"
#include "recovery_selector_test/behaviors/dummy_graceful_shutdown.hpp"
#include "recovery_selector_test/behaviors/no_failures_present.hpp"

// nrg_behaviors
#include "nrg_behaviors/nrg_behaviors.hpp"

using recovery_selector_test::FailureCase;

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::Node::SharedPtr ros_node = std::make_shared<rclcpp::Node>("recovery_selector_test_bt");

  BT::BehaviorTreeFactory factory; // Initialize BT factory which registers the tree and tree nodes

  // Register RecoverySelector to read 3 failure cases from the FailureCase type
  factory.registerNodeType<recovery_selector::RecoverySelector<FailureCase, 3>>("RecoverySelector");
  
  // ---------- Register recovery_selector_test behaviors ---------- //
  factory.registerNodeType<recovery_selector_test::behaviors::CreateDummyFailure>(
    "CreateDummyFailure", ros_node);
  
  // Create different DummyRecoveryStrategy behaviors
  factory.registerNodeType<recovery_selector_test::behaviors::DummyRecoveryStrategy>(
    "RecoverFromLowBattery", ros_node, FailureCase::LOW_BATTERY);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyRecoveryStrategy>(
    "RecoverFromMotorFailure", ros_node, FailureCase::MOTOR_FAILURE);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyRecoveryStrategy>(
    "RecoverFromFailedGrasp", ros_node, FailureCase::FAILED_GRASP);

  // Create different DummyCondition behaviors
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "ReconstructionComplete", ros_node);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "GraspsDetected", ros_node);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyCondition>(
    "PipeGrasped", ros_node);

  // Create different DummyTask behaviors
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "ReconstructObject", ros_node, "/failure_source", 10.0, true);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "GeneratePredictedGrasps", ros_node, "/failure_source", 10.0, true);
  factory.registerNodeType<recovery_selector_test::behaviors::DummyTask>(
    "GraspPipe", ros_node, "/failure_source", 10.0, true);

  // Create DummyGracefulShutdown behavior
  factory.registerNodeType<recovery_selector_test::behaviors::DummyGracefulShutdown>(
    "GracefulShutdown", ros_node);

  factory.registerNodeType<recovery_selector_test::behaviors::NoFailuresPresent>(
    "NoFailuresPresent", ros_node);
  // ---------- Register recovery_selector_test behaviors ---------- //

  // Register nrg_utility_behaviors
  nrg_utility_behaviors::Config config;
  config.ros_node = ros_node; // Share ROS2 node with utility tree nodes
  nrg_utility_behaviors::registerBehaviors(factory, config);

  // Register recovery_selector_test FailureCase type
  factory.registerScriptingEnums<FailureCase>();

  // Create behavior tree
  std::string share_path = ament_index_cpp::get_package_share_directory("recovery_selector_test");
  BT::Tree tree = factory.createTreeFromFile(
    share_path + "/behavior_trees/recovery_selector_test.xml");

  // Initialize blackboard variables that are not intialized by the tree
  tree.rootBlackboard()->set<std::vector<FailureCase>>("failure_state", std::vector<FailureCase>());
  tree.rootBlackboard()->set<bool>("reconstruction_complete", false);
  tree.rootBlackboard()->set<bool>("grasps_detected", false);
  tree.rootBlackboard()->set<bool>("pipe_grasped", false);

  BT::Groot2Publisher publisher(tree); // Connect to Groot2Publisher

  // BT::StdCoutLogger logger(tree); // Log status of tree during each tick
  
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
