#pragma once

#include <chrono>

class TimeManager {
public:
    TimeManager();
    
    void setTimeScale(float scale);
    float getTimeScale() const { return timeScale; }
    
    void pause();
    void resume();
    bool isPaused() const { return paused; }
    
    void stepFrame();
    
    float getDeltaTime();
    void resetFrameTime();
    
    float getSimulationTime() const { return simulationTime; }
    
private:
    float timeScale;
    bool paused;
    bool singleStepRequest;
    float simulationTime;
    std::chrono::steady_clock::time_point lastFrameTime;
    float fixedStep;
};