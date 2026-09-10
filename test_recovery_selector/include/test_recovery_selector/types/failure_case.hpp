#pragma once

#include <string>
#include <stdexcept>

namespace test_recovery_selector
{

// This enum creates a type to represent possible failure cases written and used by [BT::TreeNode]s
enum class FailureCase
{
  NO_FAILURE,
  LOW_BATTERY,
  MOTOR_FAILURE,
  FAILED_GRASP,
  NAVIGATION_COLLISION,
  MANIPULATION_COLLISION,
  NAVIGATION_OBSTACLE_AVOIDANCE_BLOCK,
  MANIPULATION_OBSTACLE_AVOIDANCE_BLOCK
};

// Convert Failure Case to std::string
std::string failureCaseToString(FailureCase failure_case);

} // namespace test_recovery_selector
