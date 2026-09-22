#include "ekf.h"

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/logging.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <Eigen/Eigen>
#include <random>

using namespace std;
using namespace Eigen;

rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_noise_pub;
//add imu noise
double imu_noise = 0.05;    
Vector3d noise;
std::default_random_engine generator;
std::normal_distribution<double> distribution_imu(0, imu_noise);
    
void imu_callback(const sensor_msgs::msg::Imu::ConstSharedPtr &msg)
{
    // add imu noise
    noise = Vector3d(distribution_imu(generator), distribution_imu(generator), distribution_imu(generator)); 

    if (::imu_noise == 0) noise.setZero();
    sensor_msgs::msg::Imu imu_noise;
    imu_noise = *msg;

    imu_noise.linear_acceleration.x += noise(0);
    imu_noise.linear_acceleration.y += noise(1);
    imu_noise.linear_acceleration.z += noise(2);

    imu_noise_pub->publish(imu_noise);
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto n = std::make_shared<rclcpp::Node>("imu_add_noise_quat");
    imu_noise = n->declare_parameter<double>("imu_noise", 0.05);
    if (!std::isfinite(imu_noise) || imu_noise < 0) {
        RCLCPP_ERROR(n->get_logger(), "imu_noise must be finite and non-negative");
        rclcpp::shutdown(); return 1;
    }
    distribution_imu = std::normal_distribution<double>(0, imu_noise > 0 ? imu_noise : 1.0);
    auto sub = n->create_subscription<sensor_msgs::msg::Imu>("~/imu", rclcpp::SensorDataQoS().keep_last(1000), imu_callback);
    imu_noise_pub = n->create_publisher<sensor_msgs::msg::Imu>("/djiros/imu_noise", 1000);
    rclcpp::spin(n);
    sub.reset();
    imu_noise_pub.reset();
    n.reset();
    rclcpp::shutdown();
    return 0;
}
