"""Complete upstream localization chain: Livox driver -> Faster-LIO -> EKF."""
from pathlib import Path
import yaml
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, OpaqueFunction
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def launch_nodes(context):
    config = LaunchConfiguration('config').perform(context)
    parameters = yaml.safe_load(Path(config).read_text())['/**']['ros__parameters']
    imu_topic = LaunchConfiguration('imu_topic').perform(context)
    if not imu_topic:
        imu_topic = parameters.get('common.imu_topic', parameters.get('common', {}).get('imu_topic', '/livox/imu'))
    sim_time = ParameterValue(LaunchConfiguration('use_sim_time'), value_type=bool)
    return [
        Node(package='livox_ros_driver2', executable='livox_ros_driver2_node', name='livox_lidar_publisher2',
             output='screen', condition=IfCondition(LaunchConfiguration('enable_driver')),
             parameters=[{'xfer_format': 1, 'multi_topic': 0, 'data_src': 0, 'publish_freq': 10.0,
                          'output_data_type': 0, 'frame_id': 'livox_frame',
                          'user_config_path': LaunchConfiguration('driver_config'),
                          'use_sim_time': sim_time}]),
        Node(package='faster_lio', executable='run_mapping_online', name='laserMapping', output='screen',
             parameters=[config, {'common.imu_topic': imu_topic, 'use_sim_time': sim_time,
                                  'output_directory': LaunchConfiguration('output_directory')}]),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(PathJoinSubstitution([FindPackageShare('ekf'), 'launch', 'ekf_lidar.launch.py'])),
            launch_arguments={'config': LaunchConfiguration('ekf_config'), 'imu_topic': imu_topic,
                              'odom_topic': '/laserMapping/odometry',
                              'use_sim_time': LaunchConfiguration('use_sim_time')}.items()),
        Node(package='rviz2', executable='rviz2', condition=IfCondition(LaunchConfiguration('rviz')),
             arguments=['-d', PathJoinSubstitution([FindPackageShare('faster_lio'), 'config', 'faster_lio.rviz'])],
             parameters=[{'use_sim_time': sim_time}]),
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('config', default_value=PathJoinSubstitution([FindPackageShare('faster_lio'), 'config', 'mid360.yaml'])),
        DeclareLaunchArgument('ekf_config', default_value=PathJoinSubstitution([FindPackageShare('ekf'), 'config', 'PX4_vio_drone.yaml'])),
        DeclareLaunchArgument('driver_config', default_value=PathJoinSubstitution([FindPackageShare('livox_ros_driver2'), 'config', 'MID360_config.json'])),
        DeclareLaunchArgument('enable_driver', default_value='true'),
        DeclareLaunchArgument('imu_topic', default_value='', description='Empty: read common.imu_topic from mapping config'),
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument('rviz', default_value='false'),
        DeclareLaunchArgument('output_directory', default_value='./faster_lio_output'),
        OpaqueFunction(function=launch_nodes),
    ])
