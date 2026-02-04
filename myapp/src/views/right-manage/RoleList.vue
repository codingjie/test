<template>
    <div>
        <el-table :data="tableData" style="width: 100%">
            <!-- <el-table-column prop="roleName" label="角色名称" width="180" /> -->
            <!-- <el-table-column prop="icon" label="图标" width="180" /> -->
            <el-table-column label="角色名称">
                <template #default="scope">
                    <el-popover placement="right" title="权限详情" :width="200" trigger="hover"
                        @before-enter="handleHover(scope.row)">
                        <template #reference>
                            <el-button class="m-2">{{ scope.row.roleName }}</el-button>
                        </template>
                        <template #default>
                            <el-tree :data="rightlist" :props="defaultProps" :default-expand-all="true"
                                :render-content="renderContent" />
                        </template>
                    </el-popover>
                </template>
            </el-table-column>

            <el-table-column label="操作">
                <template #default="scope">
                    <el-button round type="warning" @click="handleUpdate(scope.row)">更新</el-button>
                    <el-popconfirm title="你确定要删除吗?" @confirm="handleDelete(scope.row)" confirm-button-text="确定"
                        cancel-button-text="取消">
                        <template #reference>
                            <el-button type="danger" round>删除</el-button>
                        </template>
                    </el-popconfirm>
                </template>
            </el-table-column>
        </el-table>


        <el-dialog v-model="dialogVisible" title="更新角色">

            <el-form ref="updateFormRef" :model="updateForm" :rules="rules" label-width="80px" class="ruleForm" status-icon>
                <el-form-item label="角色名称" prop="roleName">
                    <el-input v-model="updateForm.roleName" />
                </el-form-item>
                <el-form-item label="角色权限" prop="rights">
                    <el-tree :data="rightlist" :props="defaultProps" show-checkbox node-key="path" ref="treeRef" :check-strictly="true"/>
                </el-form-item>
            </el-form>

            <template #footer>
                <span class="dialog-footer">
                    <el-button @click="dialogVisible = false">取消</el-button>
                    <el-button type="primary" @click="handleConfirm()">
                        更新
                    </el-button>
                </span>
            </template>
        </el-dialog>
    </div>
</template>

<script setup>
import axios from 'axios';
import { ref, onMounted, reactive,nextTick } from 'vue'
const tableData = ref([])
const rightlist = ref([])

const defaultProps = {
    children: 'children',
    label: 'title',
}
onMounted(() => {
    getList()
    getRightList()
})

const getList = async () => {
    var res = await axios.get("/adminapi/roles")
    console.log(res.data)
    tableData.value = res.data
}
const getRightList = async () => {
    var res = await axios.get("/adminapi/rights")
    rightlist.value = res.data
}


const currentRights = ref([])
const handleHover = ({ rights }) => {
    // console.log(rights)
    currentRights.value = rights
}
//自定义tree节点回调函数
const renderContent = (
    h,
    {
        node,
        data,
        store,
    }
) => {

    return h(
        'span',
        {
            class: `custom-tree-node ${currentRights.value.includes(data.path) ? 'active' : ''}`,
        },
        h('span', null, node.label),

    )
}

//更新回调

const handleUpdate = (item) => {
    // console.log(item)
    dialogVisible.value = true
    updateForm.roleName = item.roleName
    currentItem.value = item
    //设置rights给tree
    nextTick(()=>{
        // console.log(item.rights)
        treeRef.value.setCheckedKeys(JSON.parse(item.rights))
    })
    
    
}

//更新对话框
const dialogVisible = ref(false)
const updateFormRef = ref()
const currentItem = ref({})
const updateForm = reactive({
    roleName: ""
})

const rules = reactive({
    roleName: [
        { required: true, message: '请输入角色名称', trigger: 'blur' },
    ],

})


//更新tree
const treeRef  =ref()

//弹出框确定方法

const handleConfirm =async ()=>{
    // console.log(updateForm.roleName,treeRef.value.getCheckedKeys())

    await axios.put(`/adminapi/roles/${currentItem.value.id}`,{
        roleName:updateForm.roleName,
        rights:JSON.stringify(treeRef.value.getCheckedKeys())
    })
    await getList()

    dialogVisible.value = false
}


//删除
const handleDelete =async ({id})=>{
    await axios.delete(`/adminapi/roles/${id}`)
    await getList()
}
</script>


<style lang="scss" scoped>
:deep(.active) {
    color: #f56c6c;
}
</style>