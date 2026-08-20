#include "recovery_selector/util/compare_case.hpp"


namespace recovery_selector::util
{

bool CompareCase(const std::string& failure_state, const std::string& failure_case)
{
  return failure_state == failure_case;
}

} // namespace recovery_selector::util
