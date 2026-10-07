# recovery_selector_test

A ROS 2 package for manually testing the
[`RecoverySelector`](../../recovery_selector/recovery_selector/README.md) BehaviorTree.CPP control
node. It runs a mock behavior tree where the main task is interrupted by failures determined from
the keyboard, to verify the execution of the `RecoverySelector`.

## Potential Failure cases

| Key | Enum |
|-----|------|
| 0 | `LOW_BATTERY` |
| 1 | `MOTOR_FAILURE` |
| 2 | `FAILED_GRASP` |
| 3 | `NAVIGATION_COLLISION` |
| 4 | `MANIPULATION_COLLISION` |
| 5 | `NAVIGATION_OBSTACLE_AVOIDANCE_BLOCK` |
| 6 | `MANIPULATION_OBSTACLE_AVOIDANCE_BLOCK` |
| 7 | `UNDEFINED_FAILURE` |

## Requirements

- `C++20`
- `ROS2 Humble`

## Dependencies

- `behaviortree_cpp`
- `ament_index_cpp`
- `magic_enum`
- `recovery_selector`
- `nrg_behaviors`

## Install and Build

```bash
cd <ros workspace>/src
git clone git@github.com:UTNuclearRobotics/satellite_interception.git

colcon build --packages-up-to recovery_selector_test
source install/setup.bash
```

## Run

`dummy_failure_publisher` reads raw keystrokes from stdin, so it must run in its own terminal with
`ros2 run`.

Terminal 1, the behavior tree:

```bash
ros2 launch recovery_selector_test recovery_selector_test.launch.py
```

Terminal 2, the failure injector:

```bash
ros2 run recovery_selector_test dummy_failure_publisher
```

Press a digit key (`0`-`7`) in terminal 2 to publish the corresponding failure. Keys outside that 
range are ignored. Tree state can be viewed live in [Groot2](https://www.behaviortree.dev/groot).
