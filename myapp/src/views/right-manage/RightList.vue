<template>
  <div>
    <el-table :data="tableData" style="width: 100%" row-key="path">
      <el-table-column prop="title" label="权限名称" width="180" />
      <el-table-column label="图标" width="180">
        <template #default="scope">
          <div style="display: flex; align-items: center">
            <el-icon size="25px">
              <component :is="mapIcons[scope.row.icon]"></component>
            </el-icon>
          </div>
        </template>
      </el-table-column>
      <el-table-column label="操作">
        <template #default="scope">
          <el-button size="small" @click="handleUpdate(scope.row)">更新</el-button>
          <el-popconfirm title="你确定要删除吗？"
          @confirm="handleDelete(scope.row)"
          confirm-button-text="确定" cancl-button-text="取消">
            <template #reference>
              <el-button size="small" type="danger">删除</el-button>
            </template>
          </el-popconfirm>

        </template>
      </el-table-column>
    </el-table>

    <el-dialog v-model="dialogVisible" title="更新权限">

      <el-form class="ruleForm" ref="updateFormRef" style="max-width: 
        600px" :model="updateForm" :rules="rules" status-icon>
        <el-form-item label="权限名称" prop="title">
          <el-input v-model="updateForm.title" />
        </el-form-item>

      </el-form>

      <template #footer>
        <div class="dialog-footer">
          <el-button @click="dialogVisible = false">取消</el-button>
          <el-button type="primary" @click="handleConfirm()">
            更新
          </el-button>
        </div>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import axios from 'axios'
import {
  HomeFilled,
  User,
  Key,
  OfficeBuilding,
  UploadFilled,
  List,
} from '@element-plus/icons-vue'

//图标映射
const mapIcons = {
  User: User,
  HomeFilled,
  Key,
  OfficeBuilding,
  UploadFilled,
  List
}
import { onMounted, reactive, ref } from 'vue'
const tableData = ref([])

onMounted(() => {
  getList()
})

const getList = async () => {
  var res = await axios.get("/adminapi/rights")
  //console.log(res.data)
  tableData.value = res.data
}

//更新对话框
const dialogVisible = ref(false)
const updateFormRef = ref()
const currentItem = ref({})
const updateForm = reactive({
  title: ""
})

const rules = reactive({
  title: [
    { required: true, message: '请输入权限名称', trigger: 'blur' }
  ],
})

const handleUpdate = (item) => {
  //console.log(item)
  currentItem.value = item
  updateForm.title = item.title
  dialogVisible.value = true
}

const handleConfirm = () => {
  updateFormRef.value.validate(async (valid, fields) => {
    if (valid) {
      await axios.put(`/adminapi/rights/${currentItem.value.id}`,
        updateForm)
      dialogVisible.value = false
      await getList()
    } else {
      console.log('error submit!', fields)
    }
  })
}

//删除
const handleDelete=async({id})=>{
  await axios.delete(`/adminapi/rights/${id}`)
  await getList()
}
</script>