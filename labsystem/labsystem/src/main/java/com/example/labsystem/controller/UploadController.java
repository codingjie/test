package com.example.labsystem.controller;

import com.example.labsystem.service.UploadService;
import com.example.labsystem.utils.ResultOBJ;
import com.example.labsystem.utils.SYSConstant;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;

@RequestMapping("/adminapi/upload")
@RestController
public class UploadController {
    @Autowired
    private UploadService uploadService;

    @PostMapping
    public ResultOBJ upload(@RequestParam("file") MultipartFile file){
        try {
            uploadService.upload(file);
            return new ResultOBJ(SYSConstant.CODE_SUCCESS,"上传成功");
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
