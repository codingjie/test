"""
豆瓣小组帖子内容解析器
从HTML中提取帖子标题、楼主信息、评论等关键信息，格式化为JSON
"""
import json
import re
from pathlib import Path
from bs4 import BeautifulSoup
from urllib.parse import urlparse


class DoubanPostParser:
    def __init__(self, html_content: str):
        """
        初始化解析器
        
        Args:
            html_content: HTML内容字符串或HTML文件路径
        """
        # 如果传入的是文件路径，读取文件内容；否则直接使用HTML字符串
        if isinstance(html_content, str) and Path(html_content).exists():
            with open(html_content, 'r', encoding='utf-8') as f:
                html_content = f.read()
        
        self.soup = BeautifulSoup(html_content, 'html.parser')
    
    def extract_user_id_from_url(self, url: str) -> str:
        """从用户URL中提取用户ID"""
        if not url:
            return ""
        match = re.search(r'/people/(\d+)/', url)
        return match.group(1) if match else ""
    
    def extract_post_info(self) -> dict:
        """提取帖子基本信息"""
        post_info = {
            "title": "",
            "post_id": "",
            "author": {
                "id": "",
                "name": "",
                "url": ""
            },
            "content": "",
            "create_time": "",
            "location": "",
            "like_count": 0,
            "comment_count": 0,
            "url": ""
        }
        
        # 提取标题
        title_elem = self.soup.find('h1')
        if title_elem:
            post_info["title"] = title_elem.get_text(strip=True)
        
        # 从script标签中提取帖子ID
        script_tags = self.soup.find_all('script')
        for script in script_tags:
            script_text = script.string
            if script_text and 'window._CONFIG.topic' in script_text:
                # 提取topic的id字段
                match = re.search(r'window\._CONFIG\.topic\s*=\s*\{[^}]*"id":\s*"(\d+)"', script_text)
                if match:
                    post_info["post_id"] = match.group(1)
                    break
        
        # 如果还没找到，从URL中提取
        if not post_info["post_id"]:
            url_elem = self.soup.find('meta', {'name': 'mobile-agent'})
            if url_elem:
                mobile_url = url_elem.get('content', '')
                if mobile_url:
                    match = re.search(r'/topic/(\d+)/', mobile_url)
                    if match:
                        post_info["post_id"] = match.group(1)
        
        # 提取楼主信息
        author_link = self.soup.find('div', class_='topic-doc')
        if author_link:
            author_elem = author_link.find('span', class_='from')
            if author_elem:
                author_a = author_elem.find('a')
                if author_a:
                    post_info["author"]["name"] = author_a.get_text(strip=True)
                    author_url = author_a.get('href', '')
                    post_info["author"]["url"] = author_url
                    post_info["author"]["id"] = self.extract_user_id_from_url(author_url)
        
        # 提取发布时间
        create_time_elem = self.soup.find('span', class_='create-time')
        if create_time_elem:
            post_info["create_time"] = create_time_elem.get_text(strip=True)
        
        # 提取位置
        location_elem = self.soup.find('span', class_='ip-location')
        if location_elem:
            post_info["location"] = location_elem.get_text(strip=True)
        
        # 提取帖子内容
        content_div = self.soup.find('div', id='link-report')
        if content_div:
            topic_content = content_div.find('div', class_='topic-content')
            if topic_content:
                # 提取所有文本内容
                paragraphs = topic_content.find_all('p')
                content_texts = [p.get_text(strip=True) for p in paragraphs if p.get_text(strip=True)]
                post_info["content"] = "\n".join(content_texts)
        
        # 提取点赞数
        like_elem = self.soup.find('span', class_='react-num')
        if like_elem:
            like_text = like_elem.get_text(strip=True)
            try:
                post_info["like_count"] = int(like_text)
            except:
                pass
        
        # 从script标签中提取评论数
        for script in script_tags:
            script_text = script.string
            if script_text and 'commentCount' in script_text:
                match = re.search(r'"commentCount":\s*"(\d+)"', script_text)
                if match:
                    try:
                        post_info["comment_count"] = int(match.group(1))
                    except:
                        pass
                    break
        
        # 提取帖子URL（优先使用桌面版URL）
        script_tags = self.soup.find_all('script')
        for script in script_tags:
            script_text = script.string
            if script_text and 'application/ld+json' in str(script.get('type', '')):
                match = re.search(r'"url":\s*"(https://[^"]+)"', script_text)
                if match:
                    post_info["url"] = match.group(1)
                    break
        
        # 如果没找到，使用mobile URL
        if not post_info["url"]:
            url_elem = self.soup.find('meta', {'name': 'mobile-agent'})
            if url_elem:
                mobile_url = url_elem.get('content', '')
                if mobile_url:
                    match = re.search(r'url=(https://[^\s]+)', mobile_url)
                    if match:
                        post_info["url"] = match.group(1)
        
        return post_info
    
    def extract_comments(self, max_comments: int = None) -> list:
        """提取评论信息"""
        comments = []
        
        # 查找所有评论项（包括最赞回复和普通回复）
        comment_items = self.soup.find_all('li', class_='comment-item')
        
        for idx, item in enumerate(comment_items):
            if max_comments and idx >= max_comments:
                break
            
            comment = {
                "comment_id": "",
                "author": {
                    "id": "",
                    "name": "",
                    "url": ""
                },
                "content": "",
                "publish_time": "",
                "location": "",
                "is_author": False,
                "like_count": 0,
                "reply_to": None
            }
            
            # 提取评论ID
            comment["comment_id"] = item.get('id', '')
            if not comment["comment_id"]:
                comment["comment_id"] = item.get('data-cid', '')
            
            # 提取作者ID
            author_id = item.get('data-author-id', '')
            comment["author"]["id"] = author_id
            
            # 提取作者名称和URL
            author_elem = item.find('h4')
            if author_elem:
                author_link = author_elem.find('a')
                if author_link:
                    comment["author"]["name"] = author_link.get_text(strip=True)
                    author_url = author_link.get('href', '')
                    comment["author"]["url"] = author_url
                    if not comment["author"]["id"]:
                        comment["author"]["id"] = self.extract_user_id_from_url(author_url)
            
            # 检查是否是楼主
            author_icon = item.find('span', class_='topic-author-icon')
            if author_icon and author_icon.get_text(strip=True) == '楼主':
                comment["is_author"] = True
            
            # 提取发布时间和位置
            pubtime_elem = item.find('span', class_='pubtime')
            if pubtime_elem:
                pubtime_text = pubtime_elem.get_text(strip=True)
                # 解析时间和位置（格式：2026-01-01 07:15:48 黑龙江）
                parts = pubtime_text.split()
                if len(parts) >= 2:
                    comment["publish_time"] = " ".join(parts[:2])  # 日期和时间
                    if len(parts) > 2:
                        comment["location"] = parts[2]  # 位置
            
            # 提取评论内容
            reply_content = item.find('div', class_='reply-content')
            if reply_content:
                markdown_div = reply_content.find('div', class_='markdown')
                if markdown_div:
                    paragraphs = markdown_div.find_all('p')
                    content_texts = [p.get_text(strip=True) for p in paragraphs if p.get_text(strip=True)]
                    comment["content"] = "\n".join(content_texts)
                else:
                    # 如果没有markdown div，直接提取文本
                    comment["content"] = reply_content.get_text(strip=True)
            
            # 提取点赞数
            like_link = item.find('a', class_='comment-vote')
            if like_link:
                like_text = like_link.get_text(strip=True)
                match = re.search(r'赞\s*\(?(\d+)\)?', like_text)
                if match:
                    try:
                        comment["like_count"] = int(match.group(1))
                    except:
                        pass
            
            # 检查是否有回复引用
            reply_quote = item.find('div', class_='reply-quote')
            if reply_quote:
                quote_content = reply_quote.find('div', class_='reply-quote-content')
                if quote_content:
                    ref_author_id = quote_content.get('data-author-id', '')
                    ref_cid = quote_content.get('data-ref-cid', '')
                    ref_content_elem = quote_content.find('span', class_='all')
                    if not ref_content_elem:
                        ref_content_elem = quote_content.find('span', class_='short')
                    
                    ref_content = ref_content_elem.get_text(strip=True) if ref_content_elem else ""
                    
                    ref_author_elem = quote_content.find('span', class_='pubdate')
                    ref_author_name = ""
                    if ref_author_elem:
                        ref_author_link = ref_author_elem.find('a')
                        if ref_author_link:
                            ref_author_name = ref_author_link.get_text(strip=True)
                    
                    comment["reply_to"] = {
                        "comment_id": ref_cid,
                        "author_id": ref_author_id,
                        "author_name": ref_author_name,
                        "content": ref_content
                    }
            
            # 只添加有内容的评论
            if comment["content"] or comment["comment_id"]:
                comments.append(comment)
        
        return comments
    
    def parse(self, max_comments: int = None) -> dict:
        """
        解析整个帖子，返回结构化数据
        
        Args:
            max_comments: 最大评论数量限制，None表示提取所有评论
        
        Returns:
            包含帖子信息和评论的字典
        """
        result = {
            "post": self.extract_post_info(),
            "comments": self.extract_comments(max_comments)
        }
        
        return result
    
    def save_json(self, output_file: str = None, max_comments: int = None, indent: int = 2):
        """
        解析并保存为JSON文件
        
        Args:
            output_file: 输出文件路径，默认为原HTML文件名.json
            max_comments: 最大评论数量限制
            indent: JSON缩进空格数
        """
        data = self.parse(max_comments)
        
        if output_file is None:
            output_file = self.html_file.with_suffix('.json')
        else:
            output_file = Path(output_file)
        
        output_file.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=indent)
        
        print(f"数据已保存到: {output_file}")
        print(f"帖子标题: {data['post']['title']}")
        print(f"楼主: {data['post']['author']['name']}")
        print(f"评论数: {len(data['comments'])}")
        
        return output_file


def main():
    """主函数"""
    import sys
    
    # 默认使用response.html
    html_file = "response.html"
    if len(sys.argv) > 1:
        html_file = sys.argv[1]
    
    if not Path(html_file).exists():
        print(f"错误: 文件 {html_file} 不存在")
        return
    
    # 创建解析器并解析
    parser = DoubanPostParser(html_file)
    
    # 保存为JSON
    output_file = Path(html_file).with_suffix('.json')
    parser.save_json(str(output_file))
    
    # 也可以打印到控制台
    data = parser.parse()
    print("\n解析结果预览:")
    print(json.dumps(data, ensure_ascii=False, indent=2)[:500] + "...")


if __name__ == "__main__":
    main()
