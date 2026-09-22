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
        DeclareLaunchArgument('offset_px', default_value=EnvironmentVariable('INIT_X', default_value='0.0')),
        DeclareLaunchArgument('offset_py', default_value=EnvironmentVariable('INIT_Y', default_value='0.0')),
        DeclareLaunchArgument('offset_pz', default_value=EnvironmentVariable('INIT_Z', default_value='0.0')),
        Node(package='ekf', executable='ekf', name='ekf', output='screen',
             parameters=[LaunchConfiguration('config'), {
                 'use_sim_time': ParameterValue(LaunchConfiguration('use_sim_time'), value_type=bool),
                 'offset_px': ParameterValue(LaunchConfiguration('offset_px'), value_type=float),
                 'offset_py': ParameterValue(LaunchConfiguration('offset_py'), value_type=float),
                 'offset_pz': ParameterValue(LaunchConfiguration('offset_pz'), value_type=float),
             }],
             remappings=[('~/imu', LaunchConfiguration('imu_topic')),
                         ('~/bodyodometry', LaunchConfiguration('odom_topic')),
                         ('~/ekf_odom', LaunchConfiguration('output_topic'))]),
    ])
