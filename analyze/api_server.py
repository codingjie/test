"""
Flask API服务器
提供情感分析数据的API接口
"""
from flask import Flask, jsonify, send_from_directory
from flask_cors import CORS
from flask_compress import Compress
from pathlib import Path
import json
import sys
import os
from datetime import datetime

# 添加当前目录到路径
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sentiment_analyzer import SentimentAnalyzer

app = Flask(__name__, static_folder='../frontend', static_url_path='')
CORS(app)
Compress(app)  # 启用gzip压缩

analyzer = SentimentAnalyzer()

# 缓存分析结果
_analysis_cache = None
_cache_file = Path(__file__).parent / "analysis_result.json"


def load_cache_from_file():
    """从文件加载缓存"""
    global _analysis_cache
    if _cache_file.exists():
        try:
            print(f"从缓存文件加载数据: {_cache_file}")
            with open(_cache_file, 'r', encoding='utf-8') as f:
                _analysis_cache = json.load(f)
            print("缓存加载成功！")
            return True
        except Exception as e:
            print(f"加载缓存失败: {e}")
    return False


def save_cache_to_file(data):
    """保存缓存到文件"""
    try:
        print(f"保存分析结果到缓存文件: {_cache_file}")
        with open(_cache_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=2)
        print("缓存保存成功！")
    except Exception as e:
        print(f"保存缓存失败: {e}")


def get_analysis_result(force_refresh=False):
    """获取分析结果（带缓存）"""
    global _analysis_cache
    
    # 如果强制刷新，清除缓存
    if force_refresh:
        _analysis_cache = None
        if _cache_file.exists():
            _cache_file.unlink()
    
    # 如果内存中没有缓存，尝试从文件加载
    if _analysis_cache is None:
        if not load_cache_from_file():
            # 文件缓存不存在，重新分析
            print("正在分析数据（使用多进程并行处理加速）...")
            # CPU密集型任务（情感分析、关键词提取）使用进程池
            _analysis_cache = analyzer.analyze_batch("data", use_threading=False)
            print("分析完成！")
            # 保存到文件缓存
            save_cache_to_file(_analysis_cache)
    
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
    result = get_analysis_result(force_refresh=True)
    return jsonify({"message": "Analysis refreshed", "statistics": result["overall_statistics"]})


@app.route('/api/cache-info')
def get_cache_info():
    """获取缓存信息"""
    cache_exists = _cache_file.exists()
    cache_size = _cache_file.stat().st_size if cache_exists else 0
    cache_time = datetime.fromtimestamp(_cache_file.stat().st_mtime).isoformat() if cache_exists else None
    
    return jsonify({
        "cache_exists": cache_exists,
        "cache_size": cache_size,
        "cache_size_mb": round(cache_size / 1024 / 1024, 2),
        "cache_time": cache_time
    })


if __name__ == '__main__':
    print("启动API服务器...")
    print("访问 http://localhost:5000 查看可视化页面")
    app.run(debug=True, host='0.0.0.0', port=5000)

