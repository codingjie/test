import requests
from lxml import etree
from typing import List, Dict, Optional


def get_post_list(group_id: str, page: int = 0, headers: Optional[Dict] = None, cookies: Optional[Dict] = None) -> List[Dict[str, str]]:
    """
    获取豆瓣小组的帖子列表
    
    Args:
        group_id: 小组ID（例如：724338）
        page: 页码，从0开始
        headers: 请求头，如果为None则使用默认请求头
        cookies: Cookie，如果为None则使用默认Cookie
    
    Returns:
        帖子列表，每个帖子包含title和url字段
        [
            {
                "title": "帖子标题",
                "url": "帖子链接"
            },
            ...
        ]
    """
    # 默认请求头
    default_headers = {
        "accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
        "accept-language": "zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6",
        "cache-control": "no-cache",
        "pragma": "no-cache",
        "priority": "u=0, i",
        "referer": f"https://www.douban.com/group/{group_id}/discussion?start={page*25}&type=new",
        "sec-ch-ua": "\"Microsoft Edge\";v=\"143\", \"Chromium\";v=\"143\", \"Not A(Brand\";v=\"24\"",
        "sec-ch-ua-mobile": "?0",
        "sec-ch-ua-platform": "\"Windows\"",
        "sec-fetch-dest": "document",
        "sec-fetch-mode": "navigate",
        "sec-fetch-site": "same-origin",
        "sec-fetch-user": "?1",
        "upgrade-insecure-requests": "1",
        "user-agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36 Edg/143.0.0.0"
    }
    
    # 默认Cookie
    default_cookies = {
        "bid": "Sx-OD3y3uFs",
        "_pk_id.100001.8cb4": "54b1b2cc6d6373b3.1738384552.",
        "__yadk_uid": "deF7TCDjFs6zwrhiCa2iCnwvGXPjJWeE",
        "ll": "\"118284\"",
        "_pk_ref.100001.8cb4": "%5B%22%22%2C%22%22%2C1767332517%2C%22https%3A%2F%2Fcn.bing.com%2F%22%5D",
        "_pk_ses.100001.8cb4": "1",
        "__utma": "30149280.1494767046.1738384552.1751849459.1767332518.3",
        "__utmc": "30149280",
        "__utmz": "30149280.1767332518.3.3.utmcsr=cn.bing.com|utmccn=(referral)|utmcmd=referral|utmcct=/",
        "__utmt": "1",
        "ap_v": "0,6.0",
        "__utmb": "30149280.19.9.1767332959044"
    }
    
    # 使用传入的headers和cookies，如果没有则使用默认值
    if headers is None:
        headers = default_headers
    else:
        headers = {**default_headers, **headers}
    
    if cookies is None:
        cookies = default_cookies
    else:
        cookies = {**default_cookies, **cookies}
    
    # 构建URL和参数
    url = f"https://www.douban.com/group/{group_id}/discussion"
    params = {
        "start": str(page * 25),
        "type": "new"
    }
    
    # 发送请求
    response = requests.get(url, headers=headers, cookies=cookies, params=params)
    response.raise_for_status()
    
    # 解析HTML
    html = etree.HTML(response.text)
    
    # 使用xpath表达式提取帖子列表
    # //td[@class='title']/a[@href]
    post_links = html.xpath("//td[@class='title']/a[@href]")
    
    # 提取帖子信息
    post_list = []
    for link in post_links:
        title = link.text.strip() if link.text else ""
        url = link.get('href', '')
        
        # 补全URL（如果是相对路径）
        if url and not url.startswith('http'):
            url = 'https://www.douban.com' + url
        
        if title and url:
            post_list.append({
                "title": title,
                "url": url
            })
    
    return post_list


if __name__ == "__main__":
    # 示例用法
    import sys
    import io
    
    # 设置输出编码为UTF-8（解决Windows控制台编码问题）
    if sys.platform == 'win32':
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
    
    group_id = "724338"
    page = 0
    
    posts = get_post_list(group_id, page)
    
    print(f"获取到 {len(posts)} 个帖子：\n")
    for idx, post in enumerate(posts, 1):
        print(f"{idx}. {post['title']}")
        print(f"   URL: {post['url']}\n")