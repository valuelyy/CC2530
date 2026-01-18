# Docker博客应用 - 使用指南

## 🚀 快速开始
```bash
# 部署应用
./deploy.sh

# 查看状态
./manage.sh status

# 查看访问地址
./manage.sh url
```

## 🌐 访问地址
- 首页: http://localhost:
- 健康检查: http://localhost:/health  
- 文章列表: http://localhost:/posts
- 添加文章: http://localhost:/add/标题/内容

## 🛠 管理命令
```bash
./manage.sh start    # 启动
./manage.sh stop     # 停止  
./manage.sh restart  # 重启
./manage.sh logs     # 日志
./manage.sh clean    # 清理
```

## 📊 当前状态
- 应用端口: 
- 容器状态: 未运行
