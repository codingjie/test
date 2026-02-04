package com.example.labsystem.dao;

import com.example.labsystem.pojo.User;
import org.apache.ibatis.annotations.Mapper;

import java.util.List;
@Mapper
public interface UserMapper {
    void addUserList(User user);

    public List<User> getUserList(User user);

    void updateUserList(User user);

    void deleteUserList(Integer user_id);

}
