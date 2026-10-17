from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PythonExpression

from launch_ros.actions import Node


def generate_launch_description():
    # Choose which behavior tree executer node to launch
    node_arg = DeclareLaunchArgument(
        'bt_executer',
        default_value='recovery_selector_test_bt',
        choices=['recovery_selector_test_bt', 'example_traditional_bt'],
        description='Behavior tree executer node to launch'
    )

    node = LaunchConfiguration('bt_executer')

    return LaunchDescription([
        node_arg,

        # Launch recovery_selector_test behavior tree executer node
        Node(
            package='recovery_selector_test',
            executable='recovery_selector_test_bt',
            output='screen',
            emulate_tty=True,
            condition=IfCondition(
                PythonExpression(["'", node, "' == 'recovery_selector_test_bt'"]))
        ),

        # Launch example_traditional_bt behavior tree executer node
        Node(
            package='recovery_selector_test',
            executable='example_traditional_bt',
            output='screen',
            emulate_tty=True,
            condition=IfCondition(
                PythonExpression(["'", node, "' == 'example_traditional_bt'"]))
        )
    ])
