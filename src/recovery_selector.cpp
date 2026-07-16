#include "recovery_selector.hpp"

template <size_t NUM_FAILURE_CASES>
RecoverySelector<NUM_FAILURE_CASES>::RecoverySelector(
  const std::string& name, const BT::NodeConfig& config
):
  BT::ControlNode(name, config)
{
  setRegistrationID("RecoverySelector")
  for(int i = 1; i <= NUM_FAILURE_CASES; i++)
  {

  }
}
