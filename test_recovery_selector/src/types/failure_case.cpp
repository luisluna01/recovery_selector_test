#include "test_recovery_selector/types/failure_case.hpp"


namespace test_recovery_selector
{

std::string failureCaseToString(FailureCase failure_case)
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
  }
  
  throw std::invalid_argument("Input must be an enumerator from FailureCase");
}

} // namespace teset_recovery_selector