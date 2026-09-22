#include "laser_mapping.h"
#include "utils.h"
#include <rosbag2_cpp/reader.hpp>
#include <rclcpp/serialization.hpp>

template<class Message>
typename Message::SharedPtr Decode(const std::shared_ptr<rosbag2_storage::SerializedBagMessage> &record) {
    rclcpp::SerializedMessage serialized(*record->serialized_data);
    auto msg = std::make_shared<Message>();
    rclcpp::Serialization<Message>().deserialize_message(&serialized, msg.get());
    return msg;
}

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;
    rclcpp::init(argc, argv);
    int result = 0;
    try {
        auto node = std::make_shared<rclcpp::Node>("laserMapping");
        auto mapping = std::make_shared<faster_lio::LaserMapping>();
        if (!mapping->InitROS(node)) throw std::runtime_error("Mapping initialization failed");
        mapping->SetOffline(true);
        const auto bag = node->declare_parameter<std::string>("bag_file", "");
        if (bag.empty()) throw std::runtime_error("Set the bag_file ROS2 parameter to a rosbag2 directory or file");
        const auto lidar = node->get_parameter("common.lid_topic").as_string();
        const auto imu = node->get_parameter("common.imu_topic").as_string();
        const auto livox = node->get_parameter("preprocess.lidar_type").as_int() == 1;
        rosbag2_cpp::Reader reader;
        reader.open(bag);
        bool found_lidar = false, found_imu = false;
        for (const auto &topic : reader.get_all_topics_and_types()) {
            if (topic.name == lidar) found_lidar = topic.type == (livox ? "livox_ros_driver2/msg/CustomMsg" : "sensor_msgs/msg/PointCloud2");
            if (topic.name == imu) found_imu = topic.type == "sensor_msgs/msg/Imu";
        }
        if (!found_lidar || !found_imu) throw std::runtime_error("Bag topics/types do not match common.lid_topic, common.imu_topic and preprocess.lidar_type");
        while (rclcpp::ok() && reader.has_next()) {
            auto record = reader.read_next();
            if (record->topic_name == lidar) {
                if (livox) mapping->LivoxPCLCallBack(Decode<livox_ros_driver2::msg::CustomMsg>(record));
                else mapping->StandardPCLCallBack(Decode<sensor_msgs::msg::PointCloud2>(record));
            } else if (record->topic_name == imu) {
                mapping->IMUCallBack(Decode<sensor_msgs::msg::Imu>(record));
            } else continue;
            mapping->Run();
        }
        mapping->Finish();
        auto trajectory = node->declare_parameter<std::string>("traj_log_file", "");
        if (trajectory.empty()) trajectory = node->get_parameter("output_directory").as_string() + "/traj.txt";
        mapping->Savetrajectory(trajectory);
        faster_lio::Timer::PrintAll();
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what(); result = 1;
    }
    rclcpp::shutdown();
    google::ShutdownGoogleLogging();
    return result;
}
