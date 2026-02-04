package com.example.labsystem.pojo;

import org.springframework.stereotype.Component;

@Component
public class TempHolder {
    // 初始值设为 0.0，volatile 保证多线程可见性
    private volatile double currentTemp = 0.0;

    public double getCurrentTemp() {
        return currentTemp;
    }

    public void setCurrentTemp(double currentTemp) {
        this.currentTemp = currentTemp;
    }
}