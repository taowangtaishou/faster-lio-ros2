# faster-lio-ros2

实现了 Faster-LIO 的 ROS2 版本，包含 Livox 雷达驱动与 EKF 高频里程计。

适用环境：Ubuntu 24.04、ROS2 Jazzy。

## 1. 安装 Livox SDK2

雷达驱动依赖 Livox SDK2，已安装可跳过。

```bash
git clone https://github.com/Livox-SDK/Livox-SDK2.git
cmake -S Livox-SDK2 -B Livox-SDK2/build -DCMAKE_BUILD_TYPE=Release
cmake --build Livox-SDK2/build -j2
sudo cmake --install Livox-SDK2/build
sudo ldconfig
```

## 2. 下载并编译

需先安装 ROS2 Jazzy，并完成 rosdep 初始化。私有仓库需要 GitHub 访问权限。

```bash
git clone https://github.com/taowangtaishou/faster-lio-ros2.git
cd faster-lio-ros2
source /opt/ros/jazzy/setup.bash

sudo apt install python3-colcon-common-extensions python3-rosdep
rosdep update
rosdep install --from-paths src --ignore-src -r -y

MAKEFLAGS=-j2 colcon build --symlink-install --executor sequential \
  --cmake-args -DROS_EDITION=ROS2 -DDISTRO_ROS=jazzy \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
```

## 3. 启动

```bash
source install/setup.bash
ros2 launch faster_lio localization.launch.py
```

同时启动 Livox 驱动、Faster-LIO 和 EKF。运行时需连接雷达，并提供配置对应的 IMU 数据。

参考：https://github.com/DifferentialRobotics/faster-lio
