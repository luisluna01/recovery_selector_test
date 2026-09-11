# Constants for failure cases available in test_recovery_selector::FailureCase
uint8 CASE_NO_FAILURE=0
uint8 CASE_LOW_BATTERY=1
uint8 CASE_MOTOR_FAILURE=2
uint8 CASE_FAILED_GRASP=3
uint8 CASE_NAVIGATION_COLLISION=4
uint8 CASE_MANIPULATION_COLLISION=5
uint8 CASE_NAVIGATION_OBSTACLE_AVOIDANCE_BLOCK=6
uint8 CASE_MANIPULATION_OBSTACLE_AVOIDANCE_BLOCK=7

# This message is an integer that should map to a test_recovery_selector::FailureCase
uint8 case_id
