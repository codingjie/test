package com.example.labsystem.service;

import com.example.labsystem.pojo.Right;

import java.util.List;

public interface RightService {
    public List<Right> getRightList();

    void updateRightlist(Right right);

    void deleteRightList(Integer id);
}
