# ⚛️ Atomic Sandbox

**从基本粒子开始的较为真实的物理沙盒**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![OpenGL](https://img.shields.io/badge/OpenGL-4.6-green.svg)](https://www.opengl.org/)

## ✨ 特性

- 🔬 **从基本粒子开始**：模拟质子、中子、电子构建原子
- 🎯 **较为真实的物理**：基于原子定律计算
- ⏰ **时间操控**：支持从 1/1,000,000 倍慢放到 1000 倍快进
- ⏸️ **逐帧分析**：暂停 + 帧步进，逐帧解剖物理过程
- 🎨 **非像素3D渲染**：球体实例化 + 电子云光晕效果
- 💰 **完全免费**：无内购，无广告，开源

## 🖥️ 系统要求

| 配置 | 最低要求 | 推荐配置 |
|------|---------|---------|
| CPU | 1核 2.5GHz | 4核 |
| 内存 | 2GB | 8GB |
| 显卡 | OpenGL 3.3 兼容 | GTX 960 / GTX 1060 |
| 存储 | 100MB | 500MB |
| 系统 | Windows 7 / Windows 10/11 |

## 🚀 快速开始

### 方式一：下载预编译版本（推荐）

从 [Releases](https://github.com/LittleApple12CD/AtomicSandbox/releases) 页面下载对应系统的安装包，解压后运行 `AtomicSandbox.exe`。

### 方式二：从源码编译

#### 依赖安装

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libglm-dev libgl1-mesa-dev
