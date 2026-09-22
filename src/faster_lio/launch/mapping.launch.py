from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('config', default_value=PathJoinSubstitution([FindPackageShare('faster_lio'), 'config', 'mid360.yaml'])),
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument('rviz', default_value='false'),
        DeclareLaunchArgument('output_directory', default_value='./faster_lio_output'),
        Node(package='faster_lio', executable='run_mapping_online', name='laserMapping', output='screen',
             parameters=[LaunchConfiguration('config'), {'use_sim_time': ParameterValue(LaunchConfiguration('use_sim_time'), value_type=bool),
                         'output_directory': LaunchConfiguration('output_directory')}]),
        Node(package='rviz2', executable='rviz2', condition=IfCondition(LaunchConfiguration('rviz')),
             arguments=['-d', PathJoinSubstitution([FindPackageShare('faster_lio'), 'config', 'faster_lio.rviz'])],
             parameters=[{'use_sim_time': ParameterValue(LaunchConfiguration('use_sim_time'), value_type=bool)}]),
    ])
