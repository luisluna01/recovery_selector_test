#pragma once

#include <string>
#include <stdexcept>
#include <vector>

#include <behaviortree_cpp/basic_types.h>


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


namespace BT
{

// Single enum used only by the vector parser below.
// Exmple: getInput<FailureCase> never reaches this; parseString uses the scripting registry first
template <>
inline test_recovery_selector::FailureCase convertFromString(StringView failure_case_string)
{
  using test_recovery_selector::FailureCase;
  using test_recovery_selector::failureCaseToString;

  const auto last_failure_case = static_cast<uint8_t>(FailureCase::UNDEFINED_FAILURE);
  for (auto i = 0; i <= last_failure_case; ++i)
  {
    const auto failure_case = static_cast<FailureCase>(i);
    if (failure_case_string == failureCaseToString(failure_case))
    {
      return failure_case;
    }
  }
  throw RuntimeError("Invalid FailureCase: " + std::string(failure_case_string));
}

// Vector parser for enums
// Example: "LOW_BATTERY;MOTOR_FAILURE" -> {LOW_BATTERY, MOTOR_FAILURE}
template <>
inline std::vector<test_recovery_selector::FailureCase> convertFromString(StringView str)
{
  const auto parts = splitString(str, ';');

  std::vector<test_recovery_selector::FailureCase> output;
  output.reserve(parts.size());
  for (const auto& part : parts)
  {
    output.push_back(convertFromString<test_recovery_selector::FailureCase>(part));
  }
  return output;
}

}  // namespace BT
