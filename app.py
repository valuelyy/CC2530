from flask import Flask, jsonify
import sqlite3
import os

app = Flask(__name__)
DATABASE = '/app/blog.db'

def init_db():
    """初始化数据库"""
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()
    
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS posts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            content TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    
    cursor.execute("INSERT OR IGNORE INTO posts (title, content) VALUES (?, ?)", 
                  ('欢迎来到我的博客', '这是我的第一篇博客文章！'))
    cursor.execute("INSERT OR IGNORE INTO posts (title, content) VALUES (?, ?)", 
                  ('Docker学习笔记', '今天学习了Docker容器化技术'))
    
    conn.commit()
    conn.close()
    print("数据库初始化完成")

@app.route('/')
def index():
    return '''
    <h1>我的个人博客</h1>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }
        .post { border: 1px solid #ddd; padding: 15px; margin: 10px 0; border-radius: 5px; }
        a { color: #007bff; text-decoration: none; }
        a:hover { text-decoration: underline; }
    </style>
    <p><a href="/posts">📝 查看所有文章</a></p>
    <p><a href="/health">🔍 健康检查</a></p>
    <p><a href="/add/测试标题/测试内容">➕ 添加测试文章</a></p>
    '''

@app.route('/posts')
def posts():
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        cursor.execute('SELECT * FROM posts ORDER BY created_at DESC')
        posts = cursor.fetchall()
        conn.close()
        
        html = '<h1>📖 博客文章</h1><a href="/">← 返回首页</a><hr>'
        for post in posts:
            html += f'''
            <div class="post">
                <h3>{post[1]}</h3>
                <p>{post[2]}</p>
                <small>发布时间: {post[3]}</small>
            </div>
            '''
        return html
    except Exception as e:
        return f"错误: {e}"

@app.route('/add/<title>/<content>')
def add_post(title, content):
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        cursor.execute("INSERT INTO posts (title, content) VALUES (?, ?)", (title, content))
        conn.commit()
        conn.close()
        return f"✅ 文章添加成功: {title}"
    except Exception as e:
        return f"❌ 添加失败: {e}"

@app.route('/health')
def health():
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        cursor.execute('SELECT COUNT(*) FROM posts')
        count = cursor.fetchone()[0]
        conn.close()
        return jsonify({
            'status': 'healthy', 
            'database': 'connected',
            'post_count': count,
            'message': '博客应用运行正常'
        })
    except Exception as e:
        return jsonify({'status': 'error', 'database': str(e)}), 500

if __name__ == '__main__':
    init_db()
    print("🚀 博客应用启动成功！")
    app.run(host='0.0.0.0', port=5000, debug=False)
