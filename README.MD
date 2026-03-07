# Simple-OpenGL-FPS
![alt text](https://img.shields.io/badge/C++-17-blue.svg)
![alt text](https://img.shields.io/badge/OpenGL-3.3+-green.svg)
![alt text](https://img.shields.io/badge/License-MIT-yellow.svg)

***Simple-OpenGL-FPS*** 是一个基于 **C++** 和 **OpenGL 3.3** 的从零编写的第一人称射击游戏原型。

本项目实现了基于接口抽象的一套渲染引擎架构，支持 PBR 渲染管线、骨骼动画以及实体管理系统，使用 Assimp 模型加载库，目前支持 glTF 和 glb 格式的模型/纹理/动画。

本项目由我的学习仓库 [*MyLearnOpenGL*](https://github.com/snailxxd/MyLearnOpenGL) 重构并丰富而来，重点从“实现图形特性”转向了“游戏引擎架构设计”，从面向过程的教学实现重构为模块化的面向对象设计。

## 📸 项目演示
![](docs/images/reload.gif)
![](docs/images/lookaround.gif)

## 🏗 构建说明 (Build)

#### 工具链 (Toolchain)
- CMake: 版本建议 ≥ 3.13
- 编译器: Windows: MinGW (GCC 7+)

#### 外部依赖库 (External Libraries)
本项目依赖以下第三方库，请确保这些库已安装并在系统路径中，或放置于项目的 /lib 和 /include 目录下：
|库名称|用途|获取方式|
|-|-|-|
|GLFW3|窗口管理与输入|已位于 \lib 中|
|Assimp|3D 模型与动画加载|已位于 \lib 中|
|GLAD|OpenGL 核心加载器|已位于 \include 中|
|stb_image|图片加载库|已位于 \include 中|
|Dear ImGui|实时调试 GUI 面板|已位于 \include 中|
|zlib|数据压缩 (Assimp 依赖)|随 Assimp 安装或系统自带|
|OpenGL|图形渲染底层 API|显卡驱动自带|

#### 构建并运行
克隆仓库：
```bash
git clone git@github.com:snailxxd/Simple-OpenGL-FPS.git
```

执行构建：
```bash
cd Simple-OpenGL-FPS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++
cmake --build build --config Release -j 8
```

运行 demo：
```bash
.\build\demo.exe 
```

## 🚀 核心特性
#### 1. 渲染引擎
- PBR 渲染管线：实现了基于物理的渲染（Physically Based Rendering），采用 Cook-Torrance BRDF 模型。
- 阴影系统：实现了阴影映射，并支持 PCF 软阴影。
- 天空盒与 HDR：将 HDR 全景图转换为立方体贴图（Cubemap），渲染为天空盒，提供真实的天空环境。
- 后期处理：支持 Gamma 校正与颜色空间管理。
#### 2. 动画系统
- 骨骼动画：基于 Assimp 实现了复杂的骨骼动画加载与播放。
- 状态机控制：通过角色/武器状态机控制其动作。
#### 3. 引擎架构
- 引擎与应用层解耦：通过 IRenderEngine 抽象接口隔离了底层 OpenGL 实现与上层 FPS 游戏逻辑，具备良好的可扩展性。
- 实体组件思想：实现了 Entity-Scene 管理系统，支持场景实体的自动化更新与渲染。
- 输入系统：封装了基于 GLFW 的原始鼠标/键盘输入，实现了平滑的相机控制与武器摆动。
#### 4. 调试系统
- 集成 ImGui：实时监控帧率、调整渲染参数（光照、阴影、模型位置、动画速度等）。

## 🛠 逻辑层次

- Application: 玩家控制、相机逻辑、武器状态机
- Render Engine: 负责窗口管理、OpenGL 状态维护、着色器资源加载。
- World & Entity: 管理游戏场景中的所有对象及其变换坐标，模型、材质与动画的加载。

## 📅 开发路线图
- [x] 核心渲染管线与 PBR 支持
- [x] 骨骼动画系统
- [x] FPS 风格视角与控制
- [ ] 物理引擎与碰撞检测
- [ ] 性能优化

## 📚 参考与引用
- LearnOpenGL 教程
- 我的基础学习仓库 & 渲染引擎原型：[MyLearnOpenGL](https://github.com/snailxxd/MyLearnOpenGL)
- demo 中使用的武器模型 & 动画：["FPS AK-74m animations"](https://skfb.ly/oBq6Q) by Cransh
- demo 中使用的场景模型：["Prison(Game environment concept)"](https://skfb.ly/oo966) by P736728