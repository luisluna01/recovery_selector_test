from launch import LaunchDescription

from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        # Launch behavior tree executer node
        Node(
            package='recovery_selector_test',
            executable='recovery_selector_test_bt',
            output='screen',
            emulate_tty=True
        )
    ])
