package com.example.labsystem.service;

import com.example.labsystem.pojo.Right;
import com.example.labsystem.pojo.Role;

import java.util.List;

public interface RoleService {
    public List<Role> getRoleList();

    void updateRolelist(Role role);

    void deleteRoleList(Integer id);
}
