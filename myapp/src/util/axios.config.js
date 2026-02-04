import axios from 'axios'
import { ElLoading } from 'element-plus'


// Add a request interceptor
axios.interceptors.request.use(function (config) {
    // Do something before the request is sent
    // console.log("请求之前")
    const token=localStorage.getItem("token")
    config.headers.token=token


    return config;
  }, function (error) {
    // Do something with the request error
    return Promise.reject(error);
  });

// Add a response interceptor
axios.interceptors.response.use(function (response) {
    // Any status code that lies within the range of 2xx causes this function to trigger
    // Do something with response data
    // console.log("then响应处理之前")
    const res=response.data
    res?.data?.token && localStorage.setItem("token",res?.data?.token)
    return response;
  }, function (error) {
    // Any status codes that fall outside the range of 2xx cause this function to trigger
    // Do something with response error
    // console.log("catch响应处理之前",error.response)

    const {status}=error.response
    if(status===401){
      localStorage.removeItem("token")
      //重定向
      window.localStorage.href="#/login"
    }
    return Promise.reject(error);
  });