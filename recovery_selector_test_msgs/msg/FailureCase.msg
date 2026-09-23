# Constants for failure cases available in recovery_selector_test::FailureCase
# Note: Values must match the enumerator indices of recovery_selector_test::FailureCase
uint8 CASE_LOW_BATTERY=0
uint8 CASE_MOTOR_FAILURE=1
uint8 CASE_FAILED_GRASP=2
uint8 CASE_NAVIGATION_COLLISION=3
uint8 CASE_MANIPULATION_COLLISION=4
uint8 CASE_NAVIGATION_OBSTACLE_AVOIDANCE_BLOCK=5
uint8 CASE_MANIPULATION_OBSTACLE_AVOIDANCE_BLOCK=6
uint8 CASE_UNDEFINED_FAILURE=7

# This message is an integer that should map to a recovery_selector_test::FailureCase
uint8 case_id
