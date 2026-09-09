// This file hosts an enum representing possible failure cases as a type


namespace test_recovery_selector
{

enum class FailureCase
{
  LOW_BATTERY,
  MOTOR_FAILURE,
  FAILED_GRASP,
  NAVIGATION_COLLISION,
  MANIPULATION_COLLISION,
  NAVIGATION_OBSTACLE_AVOIDANCE_BLOCK,
  MANIPULATION_OBSTACLE_AVOIDANCE_BLOCK
};

}; // namespace test_recovery_selector
