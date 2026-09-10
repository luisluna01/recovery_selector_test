#pragma once

#include <string>


namespace recovery_selector::util
{

// Helper function for RecoverySelector to compare a failure state with a potential failure case
template <typename EnumType>
bool compareCase(EnumType failure_state, EnumType failure_case)
{
  return failure_state == failure_case;
}

} // namespace recovery_selector::util
