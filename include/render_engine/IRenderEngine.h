#pragma once

// 前向声明，避免在接口头文件中引入过多渲染实现细节
class Camera;
struct Scene;
struct RenderParams;
struct GUIContext;

/**
 * @class IRenderEngine
 * @brief 渲染引擎纯虚接口，负责窗口创建/销毁、帧驱动、输入与时间查询
 */
class IRenderEngine {
public:
    virtual ~IRenderEngine() = default;

    /// 初始化窗口与图形上下文，创建渲染所需资源。成功返回 true
    virtual bool Initialize() = 0;

    /// 是否应关闭窗口
    virtual bool ShouldClose() const = 0;

    /// 开始一帧：处理输入、清屏等
    virtual void BeginFrame() = 0;

    /// 结束一帧：交换缓冲区、轮询事件等
    virtual void EndFrame() = 0;

    /// 接收 Application 传入的场景、相机与渲染参数，渲染一帧
    virtual void RenderFrame(Scene& scene, Camera& camera, const RenderParams& params) = 0;

    /// 渲染 GUI
    virtual void RenderGui(const GUIContext& ctx) = 0;

    /// 本帧与上一帧的时间差
    virtual float GetDeltaTime() const = 0;

    /// 当前窗口宽度
    virtual int GetWidth() const = 0;

    /// 当前窗口高度
    virtual int GetHeight() const = 0;

    /// 查询按键是否按下
    virtual bool IsKeyPressed(int key) const = 0;

    /// 供输入系统使用：设置用于鼠标输入的相机指针
    virtual void SetCameraForInput(Camera* camera) = 0;

    /// 获取当前聚光灯开关状态
    virtual bool IsSpotLightOn() const = 0;
};
