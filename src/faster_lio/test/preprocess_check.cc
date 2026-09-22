#include "pointcloud_preprocess.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

void Check(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
int main() {
    try {
        faster_lio::PointCloudPreprocess processor;
        PointCloudType::Ptr output(new PointCloudType);
        auto custom = std::make_shared<livox_ros_driver2::msg::CustomMsg>();
        processor.Process(custom, output);
        Check(output->empty(), "Empty Livox cloud must be safe");
        custom->point_num = 100;
        processor.Process(custom, output);
        Check(output->empty(), "Inconsistent Livox count must not access missing points");
        custom->points.resize(3);
        for (size_t i = 0; i < custom->points.size(); ++i) {
            custom->points[i].x = 1.f + i;
            custom->points[i].offset_time = i * 1000000;
        }
        processor.Process(custom, output);
        Check(output->size() == 2, "Valid Livox points must survive preprocessing");
        Check(std::abs(output->back().curvature - 2.f) < 1e-6, "Livox offsets must convert nanoseconds to milliseconds");
        auto cloud = std::make_shared<sensor_msgs::msg::PointCloud2>();
        pcl::PointCloud<velodyne_ros::Point> velodyne;
        pcl::toROSMsg(velodyne, *cloud);
        processor.SetLidarType(faster_lio::LidarType::VELO32);
        processor.Process(cloud, output);
        Check(output->empty(), "Empty Velodyne cloud must be safe");
        velodyne.resize(1);
        velodyne[0].x = 2.f; velodyne[0].y = 0.f; velodyne[0].z = 0.f;
        velodyne[0].ring = 65535; velodyne[0].time = 0.f;
        pcl::toROSMsg(velodyne, *cloud);
        processor.Process(cloud, output);
        Check(output->empty(), "Invalid ring must be rejected without indexing past scan arrays");
        pcl::PointCloud<hesai_ros::Point> hesai;
        pcl::toROSMsg(hesai, *cloud);
        processor.SetLidarType(faster_lio::LidarType::HESAIxt32);
        processor.Process(cloud, output);
        Check(output->empty(), "Empty Hesai cloud must be safe");
        std::cout << "Preprocessing regression checks passed\n";
    } catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
    return 0;
}
