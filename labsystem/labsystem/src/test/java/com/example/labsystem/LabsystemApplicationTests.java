package com.example.labsystem;

import com.example.labsystem.config.JwtConfig;
import com.example.labsystem.dao.*;
import com.example.labsystem.pojo.Book;
import com.example.labsystem.pojo.Lab;
import com.example.labsystem.pojo.User;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import java.util.Date;

@SpringBootTest
class LabsystemApplicationTests {
    @Autowired
    private JwtConfig jwtConfig;
    @Test
    void contextLoads() throws InterruptedException {
        String token=jwtConfig.createToken("admin");

        System.out.println(token);
        Thread.sleep(5000);
        try {
            System.out.println(jwtConfig.getSubject(token));
        } catch (Exception e) {
            System.out.println("过期");
        }
    }

}
