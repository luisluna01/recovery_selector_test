/* Copyright (C) 2019-2025 Davide Faconti, Eurecat -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "recovery_selector/recovery_selector.hpp"


namespace recovery_selector
{

template <size_t NUM_CASES>
RecoverySelector<NUM_CASES>::RecoverySelector(
  const std::string& name, const BT::NodeConfig& config
):
  BT::ControlNode(name, config)
{
  setRegistrationID("RecoverySelector")
  
  for(size_t i = 1; i <= NUM_CASES; i++)
  {
    // Create keys for cases of potential failure states
    case_keys_.push_back(std::string("case_") + std::to_string(i));
  }
}


template <size_t NUM_CASES>
BT::PortsList RecoverySelector<NUM_CASES>::providedPorts()
{
  BT::PortsList ports;

  // Create port failure state to recover from
  ports.insert(BT::InputPort<std::string>("failure_state"));

  // Create port for cases of potential failure states
  for(unsigned i = 1; i <= NUM_CASES; i++)
  {
    std::string case_key = std::string("case_") + std::to_string(i);
    ports.insert(BT::InputPort<std::string>(case_key));
  }

  return ports;
}


template <size_t NUM_CASES>
BT::NodeStatus RecoverySelector<NUM_CASES>::tick()
{
  // Ensure node has appropriate number of children
  if(childrenCount() != NUM_CASES + 1)
  {
    throw LogicError(
      "Wrong number of children in RecoverySelector: must be (num_cases + default)");
  }

  str::string failure_state; // Current failure state to resolve from
  std::string case_value;
  int child_index = int(NUM_CASES);

  // If failure state is present create index to identify child that should be ticked
  // - If no failure state is choose default child
  if(getInput("failure_state", failure_state))
  {
    // Check each case until the first match
    for(int index = 0; index < int(NUM_CASES); ++index)
    {
      const std::string& case_key = case_keys_[index];

      if(getInput(case_key, case_value))
      {
        if(RS::CompareCase(failure_state, case_value))
        {
          child_index = index;
          
          break;
        }
      }

    }
  }

  // Unless default child, halt currently running child if different from appropriate case
  if(running_child_ != -1 && running_child_ != child_index)
  {
    haltChild(running_child_);
  }

  // Store the selected child
  auto& selected_child = BT::children_nodes_[child_index];

  // Emit a tick signal to the selected child and store the return status
  BT::NodeStatus selected_child_status = selected_child->executeTick();

  if(selected_child_status == BT::NodeStatus::SKIPPED)
  {
    // Clear index so default child ticked next
    running_child_ = -1;

    return BT::NodeStatus::SKIPPED;
  }
  else if (selected_child_status == BT::NodeStatus::RUNNING)
  {
    running_child_ = child_index;
  }
  else // If RecoverySelector returns SUCCESS or FAILURE
  {
    // Set status of all children to IDLE and send halt() signal to all RUNNING children
    resetChildren();

    // Clear index so default child ticked next
    running_child_ = -1;
  }

  return selected_child_status;
}


template <size_t NUM_CASES>
void RecoverySelector<NUM_CASES>::halt()
{
  running_child_ = -1;

  // Force all children's status back to IDLE and this node's status back to IDLE
  BT::ControlNode::halt();
}

} // namespace recovery_selector
