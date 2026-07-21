/* Copyright (C) 2020-2025 Davide Faconti -  All Rights Reserved
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

#pragma once

#include "behaviortree_cpp/control_node.h"


template <size_t NUM_FAILURE_CASES>
class RecoverySelector : public BT::ControlNode
{
public:
  RecoverySelector(const std::string& name, const BT::NodeConfig& config);

  virtual ~RecoverySelector() override = default;

  // Make RecoverySelector non-copyable
  RecoverySelector(const RecoverySelector&) = delete;
  RecoverySelector& operator=(const RecoverySelector&) = delete;
  RecoverySelector(RecoverySelector&&) = delete;
  RecoverySelector& operator=(RecoverySelector&&) = delete;

  virtual void halt() override;

  static BT::PortsList providedPorts();


private:
  int running_child_ = -1;
  std::vector<std::string> case_keys_; // Strings indicating cases for potential failure states
};
