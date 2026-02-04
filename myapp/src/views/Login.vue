<template>
    <vue-particles id="tsparticles" @particles-loaded="particlesLoaded" url="http://foo.bar/particles.json" />

    <vue-particles id="tsparticles" @particles-loaded="particlesLoaded" :options="config" />
    <div class="formContainer">
        <h2>智能实验室综合管理系统</h2>
        <el-form class="ruleForm" ref="ruleFormRef" style="max-width: 
        600px" :model="ruleForm" :rules="rules" label-width="auto" status-icon>
            <el-form-item label="用户名" prop="username">
                <el-input v-model="ruleForm.username" />
            </el-form-item>
            <el-form-item label="密码" prop="password">
                <el-input v-model="ruleForm.password" type="password" />
            </el-form-item>
            <el-button type="primary" @click="submitForm(ruleFormRef)">
                登录
            </el-button>
            <el-button type="primary" @click="handleAddConfirm()">
                注册
            </el-button>
        </el-form>
    </div>
</template>

<script setup>
import { useUserStore } from '../store/useUserStore'
import { useRouter } from 'vue-router'
import { ref, reactive } from 'vue'
import { config } from '../util/config'
import axios from 'axios'
import { loadFull } from 'tsparticles'
import { ElMessage } from 'element-plus'
//ref获取表单对象



const particlesLoaded = async container => {
    await loadFull(container);
}
const ruleFormRef = ref()
const ruleForm = reactive({
    username: "",
    password: ""
})

const rules = reactive({
    username: [
        { required: true, message: '请输入用户名', trigger: 'blur' }
    ],
    password: [
        { required: true, message: '请输入密码', trigger: 'blur' }
    ]
})
const { changeUser } = useUserStore()
const router = useRouter()

//登录回调
const submitForm = async (formEl) => {
    if (!formEl) return
    await formEl.validate(async (valid, fields) => {
        if (valid) {
            const res = await axios.post(`/adminapi/users/login`, ruleForm)
            let { code, data } = res.data
            if (code === 0) {
                changeUser(data)
                router.push("/")
            } else {
                ElMessage.error('用户名密码错误')
            }
        } else {
            console.log('error submit!', fields)
        }
    })
    console.log(ruleForm)
}



//注册
const getList = async () => {
    var res = await axios.get("/adminapi/users")
    //console.log(res.data)
    ruleForm.value = res.data
}
const handleAddConfirm = () => {
    ruleFormRef.value.validate(async (valid) => {
        var res = await axios.get("/adminapi/users")
        const isUserExists = res.data.some(user => user.username === ruleForm.username)
        if (valid && !isUserExists) {
            await axios.post(`/adminapi/users`, ruleForm)
            await getList()
            console.log(res.data)
            ElMessage({
                message: '注册成功',
                type: 'success',
            })
        } else {
            ElMessage({
                message: '注册失败,用户名已注册',
                type: 'error',
            })
        }
    })
}
</script>
<style lang="scss" scoped>
.formContainer {
    width: 500px;
    height: 300px;
    position: fixed;
    left: 50%;
    top: 50%;
    transform: translate(-50%, -50%);
    color: antiquewhite;
    text-shadow: 2px 2px 5px black;
    z-index: 100;
}

.ruleForm {
    margin-top: 50px;
}

h2 {
    font-size: 40px;

}

:deep(.el-form-item__label) {
    color: white;
}
</style>