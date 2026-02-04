<template>
    <div>
        <div style="text-align: left;">
            <el-button type="primary" @click="handleAdd">添加用户</el-button>
        </div>
        <el-table :data="tableData" style="width: 100%">
            <!-- <el-table-column prop="roleName" label="角色名称" width="180" /> -->
            <!-- <el-table-column prop="icon" label="图标" width="180" /> -->
            <el-table-column label="角色名称">
                <template #default="scope">
                    <div>{{ scope.row.role.roleName }}</div>
                </template>
            </el-table-column>
            <el-table-column prop="username" label="角色名称" width="180" />
            <el-table-column label="操作">
                <template #default="scope">
                    <el-button round type="warning" @click="handleUpdate(scope.row)" :disabled="scope.row.is_default ? true
                        : false">更新</el-button>
                    <el-popconfirm title="你确定要删除吗?" @confirm="handleDelete(scope.row)" confirm-button-text="确定"
                        cancel-button-text="取消">
                        <template #reference>
                            <el-button type="danger" round :disabled="scope.row.is_default ? true
                                : false">删除</el-button>
                        </template>
                    </el-popconfirm>
                </template>
            </el-table-column>
        </el-table>

        <!-- 添加对话框 -->
        <el-dialog v-model="dialogVisible" title="添加用户">

            <el-form class="ruleForm" ref="addFormRef" style="max-width: 
        600px" :model="addForm" :rules="rules" status-icon>
                <el-form-item label="用户名" prop="username">
                    <el-input v-model="addForm.username" />
                </el-form-item>
                <el-form-item label="密码" prop="password">
                    <el-input v-model="addForm.password" />
                </el-form-item>
                <el-form-item label="角色" prop="roleId">
                    <el-select v-model="addForm.roleId" placeholder="请选择角色" style="width: 240px">
                        <el-option v-for="item in roleList" :key="item.id" :label="item.roleName" :value="item.id" />
                    </el-select>
                </el-form-item>
            </el-form>

            <template #footer>
                <div class="dialog-footer">
                    <el-button @click="dialogVisible = false">取消</el-button>
                    <el-button type="primary" @click="handleAddConfirm()">
                        确定
                    </el-button>
                </div>
            </template>
        </el-dialog>

        <!-- 更新对话框 -->
        <el-dialog v-model="dialogUpdateVisible" title="更新用户">

            <el-form class="ruleForm" ref="updateFormRef" style="max-width: 
        600px" :model="updateForm" :rules="rules" status-icon>
                <el-form-item label="用户名" prop="username">
                    <el-input v-model="updateForm.username" />
                </el-form-item>
                <el-form-item label="密码" prop="password">
                    <el-input v-model="updateForm.password" />
                </el-form-item>
                <el-form-item label="角色" prop="roleId">
                    <el-select v-model="updateForm.roleId" placeholder="请选择角色" style="width: 240px">
                        <el-option v-for="item in roleList" :key="item.id" :label="item.roleName" :value="item.id" />
                    </el-select>
                </el-form-item>
            </el-form>

            <template #footer>
                <div class="dialog-footer">
                    <el-button @click="dialogUpdateVisible = false">取消</el-button>
                    <el-button type="primary" @click="handleupdateConfirm()">
                        确定
                    </el-button>
                </div>
            </template>
        </el-dialog>
    </div>
</template>

<script setup>
import axios from 'axios';
import { dialogContentEmits } from 'element-plus/es/components/dialog/src/dialog-content.mjs';
import { ref, onMounted, reactive } from 'vue'
const tableData = ref([])
const roleList = ref([])
onMounted(() => {
    getList()
    getRoleList()
})

const getList = async () => {
    var res = await axios.get("/adminapi/users")
    console.log(res.data)
    tableData.value = res.data
}
const getRoleList = async () => {
    var res = await axios.get("/adminapi/roles")
    console.log(res.data)
    roleList.value = res.data
}
//显示添加对话框
const handleAdd = () => {
    dialogVisible.value = true
}
const dialogVisible = ref(false)
const addFormRef = ref()
const addForm = reactive({
    username: "",
    password: "",
    roleId: ""

})
const rules = reactive({
    username: [
        { required: true, message: '请输入用户名', trigger: 'blur' }
    ],
    password: [
        { required: true, message: '请输入密码', trigger: 'blur' }
    ],
    roleId: [
        { required: true, message: '请选择角色', trigger: 'blur' }
    ],
})
const handleAddConfirm = () => {
    addFormRef.value.validate(async (valid) => {
        if (valid) {
            dialogVisible.value = false
            await axios.post(`/adminapi/users`, addForm)
            await getList()
            addForm.username = ""
            addForm.password = ""
            addForm.roleId = ""

        } else {
            console.log(valid)
        }
    })
}

//更新
const dialogUpdateVisible = ref(false)
const updateFormRef = ref()
const currentItem = ref({})
const updateForm = reactive({
    username: "",
    password: "",
    roleId: ""

})
const handleUpdate = (item) => {
    //更新对话显示
    dialogUpdateVisible.value = true
    //updateForm 更新
    updateForm.username = item.username
    updateForm.password = item.password
    updateForm.roleId = item.roleId

    currentItem.value = item
}

const handleupdateConfirm=async()=>{
    dialogUpdateVisible.value=false
    await axios.put(`/adminapi/users/${currentItem.value.id}`,updateForm)
    await getList()
}

const handleDelete=async({id})=>{
    await axios.delete(`/adminapi/users/${id}`)
    await getList()
}


</script>