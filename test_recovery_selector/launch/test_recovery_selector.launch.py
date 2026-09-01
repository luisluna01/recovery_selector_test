from launch import LaunchDescription

from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        # Launch behavior tree executer node
        Node(
            package='test_recovery_selector',
            executable='test_recovery_selector_bt'
        )
    ])
