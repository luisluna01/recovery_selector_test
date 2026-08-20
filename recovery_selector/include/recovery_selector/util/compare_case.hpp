#pragma once

#include <string>


namespace recovery_selector::util
{

// Helper function for RecoverySelector to compare a failure state with a potential failure case
bool CompareCase(const std::string& failure_state, const std::string& failure_case);

} // namespace recovery_selector::util
