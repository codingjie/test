package com.example.labsystem.controller;

import com.example.labsystem.config.JwtConfig;
import com.example.labsystem.pojo.User;
import com.example.labsystem.service.UserService;
import com.example.labsystem.utils.ResultOBJ;
import com.example.labsystem.utils.SYSConstant;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/adminapi/users")
public class UserController {
    @Autowired
    private UserService userService;
    @Autowired
    private JwtConfig jwtConfig;
    @GetMapping
    public List<User> getUserList(User user){
        return userService.getUserList(user);
    }
    @PostMapping("/login")
    public ResultOBJ login(@RequestBody User user){
        try {
            List<User> list=userService.getUserList(user);
            if(list.isEmpty()){
                throw new RuntimeException();
            }
            User currentUser=list.get(0);
            String token=jwtConfig.createToken(currentUser.getUsername());
            currentUser.setToken(token);
            return new ResultOBJ(SYSConstant.CODE_SUCCESS,SYSConstant.LOGIN_SUCCESS,list.get(0));
        } catch (Exception e) {
            return new ResultOBJ(SYSConstant.CODE_ERROR,SYSConstant.LOGIN_ERROR);
        }
    }

    @PostMapping
    public ResultOBJ addUserList(@RequestBody User user){
        try {
            userService.addUserList(user);
            return ResultOBJ.ADD_SUCCESS;
        } catch (Exception e) {
            return ResultOBJ.ADD_ERROR;
        }
    }

    @PutMapping(value = "/{id}")
    public ResultOBJ updateUserList(@PathVariable Integer id,@RequestBody User user){
        try {
            user.setId(id);
            userService.updateUserList(user);
            return ResultOBJ.UPDATE_SUCCESS;
        } catch (Exception e) {
            return ResultOBJ.UPDATE_ERROR;
        }
    }
    @DeleteMapping(value = "/{id}")
    public ResultOBJ deleteUserList(@PathVariable Integer id){
        try {
            userService.deleteUserList(id);
            return ResultOBJ.DELETE_SUCCESS;
        } catch (Exception e) {
            return ResultOBJ.DELETE_ERROR;
        }
    }

}
