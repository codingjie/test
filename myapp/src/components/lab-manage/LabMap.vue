<template>
    <div style="min-height: 500px; justify-content: 
        center;position: relative" id="map" />
</template>

<script setup>
import { onMounted,nextTick} from 'vue';
import { ImageLayer, PointLayer, Scene } from '@antv/l7';
import { Map } from '@antv/l7-maps';
import { url } from '../../util/urlconfig';
const props=defineProps(["x","y","t"])
let scene
onMounted(() => {
    nextTick(()=>{
        scene = new Scene({
        id: 'map',
        map: new Map({
            center: [props.x, props.y],
            zoom: 5,
            version: 'SIMPLE',
            mapSize: 1000,
            maxZoom: 5,
            minZoom: 2,
            pitchEnabled: false,
            rotateEnabled: false,
        }),
    });
    scene.setBgColor('rgb(94, 182, 140)');

    const imagelayer = new ImageLayer({}).source(
        url,
        {
            parser: {
                type: 'image',
                extent: [360, 400, 640, 600],
            },
        },
    );

    imagelayer.on('click', e => {
        console.log(e)
        let { lng, lat } = e.lngLat
        popup = new Popup({
            offsets: [0, 0],
            closeButton: false
        })
            .setLnglat(e.lngLat)
            .setHTML(`<button class="el-button el-button--primary"
            onclick="add_popup(${lng},${lat})">选择位置</button>`);
        scene.addPopup(popup);
    });

    scene.on('loaded', () => {
        addTextLayer([
           {
            x:props.x,
            y:props.y,
            t:props.t
           }
        ])
        scene.addLayer(imagelayer);
    });
    })
})


const addTextLayer = (data) => {
    const textlayer = new PointLayer({ zIndex: 2 })
        .source(
            data,
        {
            parser: {
                type: 'json',
                x: 'x',
                y: 'y',
            },
        })
        .shape('t', 'text')
        .size(20)
        .active({
            color: '#00f',
            mix: 0.9,
        })
        .color('rgb(86, 156, 214)')
        .style({
            textAnchor: 'center', // 文本相对锚点的位置 center|left|right|top|bottom|top-left
            spacing: 2, // 字符间距
            fontWeight: '800',
            padding: [1, 1], // 文本包围盒 padding [水平，垂直]，影响碰撞检测结果，避免相邻文本靠的太近
            stroke: '#ffffff', // 描边颜色
            strokeWidth: 2, // 描边宽度
            textAllowOverlap: true,
        });
        scene.addLayer(textlayer);
}
</script>