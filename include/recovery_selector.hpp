#pragma once

#include "behaviortree_cpp/control_node.h"

template <size_t NUM_FAILURE_CASES>
class RecoverySelector : public BT::ControlNode
{
public:
  RecoverySelector(const std::string& name, const BT::NodeConfig& config);

  ~RecoverySelector() override = default;

  // Make RecoverySelector non-copyable
  RecoverySelector(const RecoverySelector&) = delete;
  RecoverySelector& operator=(const RecoverySelector&) = delete;
  RecoverySelector(RecoverySelector&&) = delete;
  RecoverySelector& operator=(RecoverySelector&&) = delete;

  void halt() override;

  static BT::PortsList providedPorts();
};
