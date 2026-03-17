# TaoRenderer
1. 项目概览
类型: 纯 C++ 编写的软光栅渲染器 (Software Rasterizer)。
环境: Visual Studio 工程 (Win32 API)。
核心逻辑: 不依赖 OpenGL/DirectX 等图形 API，手动实现了从顶点处理到像素填充的完整管线。
2. 核心模块与文件结构
所有源码均位于 TaoRenderer-master 目录下：

平台与入口
main.cpp: 程序的入口点。
初始化 Window, TaoRenderer, Scene。
包含主循环 (Main Loop)，处理输入消息（如按键切换渲染模式 SetRenderState 或移动相机）。
Window.h/cpp: 封装了 Windows API。
负责创建窗口、处理系统消息 (WinProc)。
负责将渲染好的 Framebuffer 显示到屏幕上。
渲染核心 (Core)
TaoRenderer.h/cpp: 渲染管线的核心类。
提供了 DrawLine (画线), SetPixel (画点) 等基础绘图函数。
实现了光栅化逻辑，包括三角形遍历、重心坐标计算等。
调度着色器 (Shader) 进行顶点和片元处理。
DataBuffer.h/cpp: 数据管理。
管理 Framebuffer (颜色缓冲区) 和 DepthBuffer (深度缓冲区)。
存储全局渲染状态（如 Uniforms, MVP 矩阵）。
Shader.h/cpp: 可编程着色器的抽象。
定义了 VertexShader (顶点着色) 和 FragmentShader (片元着色) 的接口。
支持多态，允许实现不同的着色效果（如 Phong, Blinn-Phong 等）。
场景管理 (Scene)
Scene.h/cpp: 场景图管理，包含多个模型和光源。
Model.h/cpp: 3D 模型类，负责加载网格数据 (Mesh) 和材质。
Camera.h/cpp: 摄像机类，负责计算观察矩阵 (View Matrix) 和处理视角漫游。
Texture.h/cpp: 纹理加载与采样逻辑。
📐 数学基础 (Math)
Math.h, Matrix.h, Vector.h: 自定义的数学库。
实现了向量 (Vec3, Vec4) 和 矩阵 (Mat4x4) 的运算。
提供了透视投影、视图变换等图形学算法所需的数学支持。
![Uploading 11.2_converted.gif…]()
![11 3_converted](https://github.com/user-attachments/assets/fe5ef5d1-6645-437b-8215-973c1b98ab74)
![9 3_converted](https://github.com/user-attachments/assets/d9c75b8a-d45e-456f-8a93-5ab52ad1f712)
