# 情感分析模块使用指南

## 功能概述

情感分析模块提供了对豆瓣帖子进行多维度情感分析的功能，包括：

1. **整体情感分析**：分析帖子内容和所有评论的情感倾向
2. **关键词提取**：提取帖子和评论中的关键词
3. **时间维度分析**：分析情感随时间的变化趋势
4. **作者维度分析**：分析不同评论者的情感倾向
5. **可视化展示**：通过Web界面展示分析结果

## 安装依赖

```bash
pip install -r requirements.txt
```

## 使用方法

### 1. 运行情感分析

#### 分析单个帖子

```python
from analyze.sentiment_analyzer import SentimentAnalyzer
import json

analyzer = SentimentAnalyzer()

# 读取帖子数据
with open('data/16_355670360.json', 'r', encoding='utf-8') as f:
    post_data = json.load(f)

# 分析
result = analyzer.analyze_post(post_data)
print(json.dumps(result, ensure_ascii=False, indent=2))
```

#### 批量分析

```python
from analyze.sentiment_analyzer import SentimentAnalyzer

analyzer = SentimentAnalyzer()
result = analyzer.analyze_batch("data")

# 保存结果
import json
with open('analyze/analysis_result.json', 'w', encoding='utf-8') as f:
    json.dump(result, f, ensure_ascii=False, indent=2)
```

### 2. 启动Web可视化服务

```bash
cd analyze
python api_server.py
```

然后在浏览器中访问：http://localhost:5000

## 分析维度说明

### 1. 情感得分 (Sentiment Score)

- **范围**：0-1之间
- **含义**：
  - 0.6以上：正面情感
  - 0.4-0.6：中性情感
  - 0.4以下：负面情感

### 2. 情感强度 (Intensity)

- **范围**：0-1之间
- **含义**：情感表达的强烈程度，越接近1越强烈

### 3. 关键词权重

- 使用TF-IDF算法提取关键词
- 权重越高，关键词越重要

### 4. 时间维度

- 分析评论发布时间的分布
- 观察情感随时间的变化趋势

## API接口说明

### GET /api/overall

获取总体统计信息

**响应示例**：
```json
{
  "total_posts": 35,
  "total_comments": 892,
  "avg_sentiment_score": 0.5234,
  "sentiment_distribution": {
    "positive": 450,
    "negative": 320,
    "neutral": 122
  },
  "posts_by_sentiment": {
    "positive": ["355098228", "347611190"],
    "negative": ["355670360"],
    "neutral": ["353141636"]
  }
}
```

### GET /api/posts

获取所有帖子的分析结果

### GET /api/keywords

获取关键词列表

### GET /api/post/<post_id>

获取单个帖子的详细分析

### GET /api/refresh

刷新分析结果（重新分析所有数据）

## 可视化图表说明

1. **整体情感分布饼图**：显示正面、负面、中性评论的分布比例
2. **情感得分分布直方图**：显示帖子情感得分的分布情况
3. **关键词云图**：展示高频关键词，字体大小表示权重
4. **时间维度情感变化**：显示情感随时间的变化趋势
5. **帖子情感分类统计**：统计正面、负面、中性帖子的数量

## 注意事项

1. 首次运行需要下载snownlp的模型文件，可能需要一些时间
2. 分析大量数据时可能需要较长时间，建议使用缓存机制
3. 关键词提取基于jieba分词，对于特定领域可能需要自定义词典
4. 情感分析结果仅供参考，不能完全代表真实情感

