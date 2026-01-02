"""
Flask API服务器
提供情感分析数据的API接口
"""
from flask import Flask, jsonify, send_from_directory
from flask_cors import CORS
from pathlib import Path
import json
import sys
import os

# 添加当前目录到路径
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sentiment_analyzer import SentimentAnalyzer

app = Flask(__name__, static_folder='../frontend', static_url_path='')
CORS(app)

analyzer = SentimentAnalyzer()

# 缓存分析结果
_analysis_cache = None


def get_analysis_result():
    """获取分析结果（带缓存）"""
    global _analysis_cache
    
    if _analysis_cache is None:
        print("正在分析数据...")
        _analysis_cache = analyzer.analyze_batch("data")
        print("分析完成！")
    
    return _analysis_cache


@app.route('/')
def index():
    """返回前端页面"""
    return send_from_directory('../frontend', 'index.html')


@app.route('/api/overall')
def get_overall_statistics():
    """获取总体统计信息"""
    result = get_analysis_result()
    return jsonify(result["overall_statistics"])


@app.route('/api/posts')
def get_posts():
    """获取所有帖子分析结果"""
    result = get_analysis_result()
    return jsonify(result["posts"])


@app.route('/api/keywords')
def get_keywords():
    """获取关键词"""
    result = get_analysis_result()
    return jsonify(result["top_keywords"])


@app.route('/api/post/<post_id>')
def get_post_detail(post_id):
    """获取单个帖子的详细分析"""
    result = get_analysis_result()
    
    for post in result["posts"]:
        if post["post_info"]["post_id"] == post_id:
            return jsonify(post)
    
    return jsonify({"error": "Post not found"}), 404


@app.route('/api/refresh')
def refresh_analysis():
    """刷新分析结果"""
    global _analysis_cache
    _analysis_cache = None
    result = get_analysis_result()
    return jsonify({"message": "Analysis refreshed", "statistics": result["overall_statistics"]})


if __name__ == '__main__':
    print("启动API服务器...")
    print("访问 http://localhost:5000 查看可视化页面")
    app.run(debug=True, host='0.0.0.0', port=5000)

