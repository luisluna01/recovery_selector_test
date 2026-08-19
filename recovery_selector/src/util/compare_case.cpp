#include "util/compare_case.hpp"


namespace RS
{

bool CompareCase(const std::string& failure_state, const std::string& failure_case)
{
  return failure_state == failure_case;
}

} // namespace RS