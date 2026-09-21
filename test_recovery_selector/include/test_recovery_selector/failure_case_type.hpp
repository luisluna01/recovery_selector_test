#pragma once

#include <string>
#include <stdexcept>

namespace test_recovery_selector
{

// This enum creates a type to represent possible failure cases written and used by [BT::TreeNode]s
enum class FailureCase
{
  NO_FAILURE, // Keep first
  LOW_BATTERY,
  MOTOR_FAILURE,
  FAILED_GRASP,
  NAVIGATION_COLLISION,
  MANIPULATION_COLLISION,
  NAVIGATION_OBSTACLE_AVOIDANCE_BLOCK,
  MANIPULATION_OBSTACLE_AVOIDANCE_BLOCK,
  UNDEFINED_FAILURE // Keep last
};

// Convert Failure Case to std::string
inline std::string failureCaseToString(FailureCase failure_case)
{
  switch (failure_case)
  {
    case FailureCase::NO_FAILURE:
      return "NO_FAILURE";
    case FailureCase::LOW_BATTERY:
      return "LOW_BATTERY";
    case FailureCase::MOTOR_FAILURE:
      return "MOTOR_FAILURE";
    case FailureCase::FAILED_GRASP:
      return "FAILED_GRASP";
    case FailureCase::NAVIGATION_COLLISION:
      return "NAVIGATION_COLLISION";
    case FailureCase::MANIPULATION_COLLISION:
      return "MANIPULATION_COLLISION";
    case FailureCase::NAVIGATION_OBSTACLE_AVOIDANCE_BLOCK:
      return "NAVIGATION_OBSTACLE_AVOIDANCE_BLOCK";
    case FailureCase::MANIPULATION_OBSTACLE_AVOIDANCE_BLOCK:
      return "MANIPULATION_OBSTACLE_AVOIDANCE_BLOCK";
    case FailureCase::UNDEFINED_FAILURE:
      return "UNDEFINED_FAILURE";
  }
  
  throw std::invalid_argument("Input must be an enumerator from FailureCase");
}

} // namespace test_recovery_selector
