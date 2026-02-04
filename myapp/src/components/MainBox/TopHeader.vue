<template>
    <el-header>
        <div style="color: aliceblue;">实验室管理系统</div>
        <div>
            <span style="line-height: 50px;color: aliceblue;">欢迎{{ user.username }}回来</span>
            <el-dropdown>
                <el-avatar shape="square" :size="50" :src="squareUrl" />
                <template #dropdown>
                    <el-dropdown-menu>
                        <el-dropdown-item>{{ user.role.roleName }}</el-dropdown-item>
                        <el-dropdown-item v-if="user.username === 'admin'"
                            @click="handleUpdate">更新学校平面图</el-dropdown-item>
                        <el-dropdown-item @click="handleExit">退出</el-dropdown-item>

                    </el-dropdown-menu>
                </template>
            </el-dropdown>
        </div>
    </el-header>

    <el-dialog v-model="dialogVisible" title="更新学校平面图" width="500" :before-close="handleClose">
        <el-upload class="upload-demo" drag action="/adminapi/upload" :data="data" :on-success="handleSuccess">
            <el-icon class="el-icon--upload"><upload-filled /></el-icon>
            <div class="el-upload__text">
                将文件拖到此处或 <em>点击上传</em>
            </div>
            <template #tip>
                <div class="el-upload__tip">
                    jpg/png 文件要小于500kb
                </div>
            </template>
        </el-upload>
        <template #footer>
            <div class="dialog-footer">
                <el-button @click="dialogVisible = false">取消</el-button>
                <el-button type="primary" @click="dialogVisible = false">
                    确定
                </el-button>
            </div>
        </template>
    </el-dialog>
</template>

<script setup>
import { useUserStore } from '../../store/useUserStore'
import { useRouterStore } from '../../store/useRouterStore'
import { useRouter } from 'vue-router'
import { ref } from 'vue'
import { UploadFilled } from '@element-plus/icons-vue'
const { changeUser, user } = useUserStore()
const { changeRouter } = useRouterStore()
const router = useRouter()

const squareUrl = "/avatar.png"
const handleExit = () => {
    changeUser({})

    changeRouter(false)
    //跳转到登录
    router.push("/login")
}

//更新平面图
const dialogVisible = ref(false)
const handleUpdate = () => {
    dialogVisible.value = true
}

const data={
    "token":localStorage.getItem("token")
}

const handleSuccess=()=>{
    dialogVisible.value=false
    window.location.reload()
}
</script>

<style lang="scss" scoped>
.el-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    background-color: rgb(14, 123, 218);

}
</style>