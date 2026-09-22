from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, EnvironmentVariable
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('config', default_value=PathJoinSubstitution([FindPackageShare('ekf'), 'config', 'PX4_vio_drone.yaml'])),
        DeclareLaunchArgument('imu_topic', default_value='/mavros/imu/data'),
        DeclareLaunchArgument('odom_topic', default_value='/laserMapping/odometry'),
        DeclareLaunchArgument('output_topic', default_value='/ekf/ekf_odom'),
        DeclareLaunchArgument('use_sim_time', default_value='false'),

        Node(package='ekf', executable='ekf', name='ekf', output='screen',
             parameters=[LaunchConfiguration('config'), {
                 'use_sim_time': ParameterValue(LaunchConfiguration('use_sim_time'), value_type=bool),

             }],
             remappings=[('~/imu', LaunchConfiguration('imu_topic')),
                         ('~/bodyodometry', LaunchConfiguration('odom_topic')),
                         ('~/ekf_odom', LaunchConfiguration('output_topic'))]),
    ])
