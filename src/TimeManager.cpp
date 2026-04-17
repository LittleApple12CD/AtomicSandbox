#include "TimeManager.hpp"
#include <algorithm>
#include <iostream>

TimeManager::TimeManager()
    : timeScale(1.0f)   // 保持1x，但 dt 本身是 0.016 秒
    , paused(false)
    , singleStepRequest(false)
    , simulationTime(0.0f)
    , fixedStep(1.0f / 60.0f) {}  // 60 FPS

void TimeManager::setTimeScale(float scale) {
    timeScale = std::clamp(scale, 1e-6f, 1000.0f);
    std::cout << "Time scale set to " << timeScale << "x" << std::endl;
}

void TimeManager::pause() {
    paused = true;
    std::cout << "Simulation paused" << std::endl;
}

void TimeManager::resume() {
    paused = false;
    lastFrameTime = std::chrono::steady_clock::now();
    std::cout << "Simulation resumed" << std::endl;
}

void TimeManager::stepFrame() {
    if (paused) {
        singleStepRequest = true;
        std::cout << "Frame step requested" << std::endl;
    } else {
        paused = true;
        singleStepRequest = true;
    }
}

float TimeManager::getDeltaTime() {
    if (paused && !singleStepRequest) {
        return 0.0f;
    }
    
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - lastFrameTime).count();
    
    // 限制最大帧间隔
    dt = std::min(dt, 0.1f);
    
    if (singleStepRequest) {
        singleStepRequest = false;
        if (paused) {
            // 步进后保持暂停
            lastFrameTime = now;
            simulationTime += fixedStep;
            return fixedStep;
        }
    }
    
    lastFrameTime = now;
    float scaledDt = dt * timeScale;
    simulationTime += scaledDt;
    
    return scaledDt;
}

void TimeManager::resetFrameTime() {
    lastFrameTime = std::chrono::steady_clock::now();
}