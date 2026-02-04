<template>
    <el-aside width="200px">
        <el-scrollbar>
            <el-menu :default-active="route.fullPath" class="el-menu-vertical-demo" style="height: 100vh;":router="true">
                <template v-for="data in datalist" :key="data.path" >
                    <el-sub-menu :index="data.path" v-if="data.children.length && checkAuth(data.path)">
                        <template #title>
                            <el-icon>
                                <component :is="mapIcons[data.icon]"></component>
                            </el-icon>
                            <span>{{ data.title }}</span>
                        </template>
                        <template v-for="item in data.children" :key="item.path">
                            <el-menu-item :index="item.path" v-if="checkAuth(item.path)">
                            <el-icon>
                                <component :is="mapIcons[item.icon]"></component>
                            </el-icon>
                            <span>{{ item.title }}</span>
                        </el-menu-item>
                        </template>
                    </el-sub-menu>
                    <el-menu-item :index="data.path" v-else-if="checkAuth(data.path)">
                        <el-icon>
                            <component :is="mapIcons[data.icon]"></component>
                        </el-icon>
                        <span>{{ data.title }}</span>
                    </el-menu-item>
                </template>
            </el-menu>
        </el-scrollbar>
    </el-aside>
</template>

<script setup>
import {
    HomeFilled,
    User,
    Key,
    OfficeBuilding,
    UploadFilled,
    List,
} from '@element-plus/icons-vue'

import { onMounted, ref } from 'vue'
import axios from 'axios'
import { useRoute } from 'vue-router'
import {useUserStore} from '../../store/useUserStore'

const route =useRoute()
console.log(route)
onMounted(() => {
    getList()
})
const datalist = ref([])
const getList = async () => {
    var res = await axios.get("/adminapi/rights")
    //console.log(res.data)
    datalist.value = res.data
}

//图标映射
const mapIcons = {
    User: User,
    HomeFilled,
    Key,
    OfficeBuilding,
    UploadFilled,
    List
}
const {user}=useUserStore()
const checkAuth=(path)=>{
    return user.role.rights.includes(path)
}
</script>