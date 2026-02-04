const LabType = [
    {
        label: "机房",
        value: 1,
    },
    {
        label: "大教室",
        value: 2,
    },
    {
        label: "小教室",
        value: 3,
    },
]
const CollegeTpye = [
    {
        label: "公共",
        value: 1,
    },
    {
        label: "人工智能学院",
        value: 2,
    },
    {
        label: "艺术学院",
        value: 3,
    },
    {
        label: "软件学院",
        value: 4,
    },
]
 const  LabColorType = {
    1:"#FFB6C1",
    2:"#e6e657ff",
    3:"#5a796bff",
    4:"#800080"
}

 const  CollegesColorType = {
    1:"#95a9bdff",
    2:"#14c145",
    3:"#e4c15aff",
    4:"#4682B4"
}

const ClassType=[
    {
        label:"1-2节(9:00-10:20)",
        value:1
    },
    {
        label:"3-4节(10:40-12:00)",
        value:2
    },
    {
        label:"5-6节(12:30-13:50)",
        value:3
    },
    {
        label:"7-8节(14:00-15:20)",
        value:4
    },
    {
        label:"9-10节(15:30-16:50)",
        value:5
    },
    {
        label:"11-12节(17:00-18:20)",
        value:6
    },
    {
        label:"13-14节(19:00-20:20)",
        value:7
    },
    {
        label:"15-16节(20:30-21:50)",
        value:8
    }
]

const Book_state_text=["审核中","已批准","被驳回"]
const Book_state_type=["warning","success","danger"]

const ADMIN=1
const TEACHER=2

const AUDIT=0;
const APPROVE=1;
const REJECT=2;
export{
    LabType,
    CollegeTpye,
    LabColorType,
    CollegesColorType,
    ClassType,
    Book_state_text,
    Book_state_type,
    ADMIN,
    TEACHER,
    AUDIT,
    APPROVE,
    REJECT
}