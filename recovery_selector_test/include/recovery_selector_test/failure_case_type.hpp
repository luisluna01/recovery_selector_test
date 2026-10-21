#pragma once

#include <string>
#include <stdexcept>
#include <vector>

#include <behaviortree_cpp/basic_types.h>


namespace recovery_selector_test
{

// This enum creates a type to represent possible failure cases written and used by [BT::TreeNode]s
enum class FailureCase
{
  LOW_BATTERY,
  MOTOR_FAILURE,
  FAILED_GRASP,
  COLLISION,
  OBSTACLE_PRESENT,
  DOOR_LOCKED,
  UNDEFINED_FAILURE // Keep last
};

// Convert Failure Case to std::string
inline std::string failureCaseToString(FailureCase failure_case)
{
  switch (failure_case)
  {
    case FailureCase::LOW_BATTERY:
      return "LOW_BATTERY";
    case FailureCase::MOTOR_FAILURE:
      return "MOTOR_FAILURE";
    case FailureCase::FAILED_GRASP:
      return "FAILED_GRASP";
    case FailureCase::COLLISION:
      return "COLLISION";
    case FailureCase::OBSTACLE_PRESENT:
      return "OBSTACLE_PRESENT";
    case FailureCase::DOOR_LOCKED:
      return "DOOR_LOCKED";
    case FailureCase::UNDEFINED_FAILURE:
      return "UNDEFINED_FAILURE";
  }
  
  throw std::invalid_argument("Input must be an enumerator from FailureCase");
}

} // namespace recovery_selector_test


namespace BT
{

// Single enum used only by the vector parser below.
// Example: getInput<FailureCase> never reaches this; parseString uses the scripting registry first
template <>
inline recovery_selector_test::FailureCase convertFromString(StringView failure_case_string)
{
  using recovery_selector_test::FailureCase;
  using recovery_selector_test::failureCaseToString;

  const auto last_failure_case = static_cast<uint8_t>(FailureCase::UNDEFINED_FAILURE);
  for (int i = 0; i <= last_failure_case; ++i)
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
inline std::vector<recovery_selector_test::FailureCase> convertFromString(StringView str)
{
  const auto parts = splitString(str, ';');

  std::vector<recovery_selector_test::FailureCase> output;
  output.reserve(parts.size());
  for (const auto& part : parts)
  {
    output.push_back(convertFromString<recovery_selector_test::FailureCase>(part));
  }
  return output;
}

}  // namespace BT
