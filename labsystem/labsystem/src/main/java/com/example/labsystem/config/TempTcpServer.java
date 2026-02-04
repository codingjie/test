package com.example.labsystem.config;
import com.example.labsystem.pojo.TempHolder;
import jakarta.annotation.PostConstruct;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;

@RestController
public class TempTcpServer {
    private static final int TCP_PORT = 8088;
    // 注入全局温度存储类
    @Autowired
    private TempHolder tempHolder;

    // 项目启动时自动启动 TCP 服务
    @PostConstruct
    public void startTcpServer() {
        new Thread(() -> {
            try (ServerSocket serverSocket = new ServerSocket(TCP_PORT)) {
                System.out.println("TCP 服务器启动，监听端口：" + TCP_PORT);
                // 等待 ESP8266 连接
                Socket clientSocket = serverSocket.accept();
                System.out.println("ESP8266 已连接：" + clientSocket.getInetAddress());
                // 读取数据
                BufferedReader br = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                String data;
                while ((data = br.readLine()) != null) {
                    System.out.println("收到数据：" + data);
                    // 解析数据（假设格式 temp:25.5）
                    if (data.startsWith("temp:")) {
                        double temp = Double.parseDouble(data.split(":")[1]);
                        // 更新全局温度
                        tempHolder.setCurrentTemp(temp);
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }).start();
    }

    // 给前端提供的 HTTP 接口，获取实时温度
    @GetMapping("/api/temp")
    public double getRealTimeTemp() {
        return tempHolder.getCurrentTemp();
    }
}
