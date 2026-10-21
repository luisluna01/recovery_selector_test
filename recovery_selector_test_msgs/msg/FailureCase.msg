# Constants for failure cases available in recovery_selector_test::FailureCase
# Note: Values must match the enumerator indices of recovery_selector_test::FailureCase
uint8 CASE_LOW_BATTERY=0
uint8 CASE_MOTOR_FAILURE=1
uint8 CASE_FAILED_GRASP=2
uint8 COLLISION=3
uint8 OBSTACLE_PRESENT=4
uint8 DOOR_LOCKED=5
uint8 CASE_UNDEFINED_FAILURE=6

# This message is an integer that should map to a recovery_selector_test::FailureCase
uint8 case_id
