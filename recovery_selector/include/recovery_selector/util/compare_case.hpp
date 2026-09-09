#pragma once

#include <string>


namespace recovery_selector::util
{

// Helper function for RecoverySelector to compare a failure state with a potential failure case
template <typename EnumType>
bool CompareCase(const EnumType& failure_state, const EnumType& failure_case)
{
  return failure_state == failure_case;
}

} // namespace recovery_selector::util
