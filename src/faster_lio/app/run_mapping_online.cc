#include "laser_mapping.h"
#include "utils.h"
#include <filesystem>

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;
    rclcpp::init(argc, argv);
    int result = 0;
    try {
        auto node = std::make_shared<rclcpp::Node>("laserMapping");
        auto mapping = std::make_shared<faster_lio::LaserMapping>();
        if (!mapping->InitROS(node)) throw std::runtime_error("Mapping initialization failed");
        auto trajectory = node->declare_parameter<std::string>("traj_log_file", "");
        if (trajectory.empty()) trajectory = node->get_parameter("output_directory").as_string() + "/traj.txt";
        // One executor serializes input and estimator state, including shutdown.
        auto timer = node->create_wall_timer(std::chrono::milliseconds(1), [mapping]() { mapping->Run(); });
        rclcpp::spin(node);
        mapping->Finish();
        mapping->Savetrajectory(trajectory);
        faster_lio::Timer::PrintAll();
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what();
        result = 1;
    }
    rclcpp::shutdown();
    google::ShutdownGoogleLogging();
    return result;
}
