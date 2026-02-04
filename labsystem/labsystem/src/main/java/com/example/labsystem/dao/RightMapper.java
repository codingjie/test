package com.example.labsystem.dao;

import com.example.labsystem.pojo.Right;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Select;

import java.util.List;
@Mapper
public interface RightMapper {
//    @Select("select * from rights")
    public List<Right> getRightList();

    void updateRightList(Right right);

    void deleteRightList(Integer id);
}
