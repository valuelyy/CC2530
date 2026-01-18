#!/bin/bash

# 自动查找可用端口
find_available_port() {
    for port in 8080 8081 8082 8083 8084 8085; do
        if ! ss -tuln | grep ":$port " > /dev/null; then
            echo $port
            return
        fi
    done
    echo "8086"  # 如果都占用，使用8086
}

APP_PORT=$(find_available_port)

echo "🚀 开始部署博客应用..."
echo "📡 使用端口: $APP_PORT"

# 停止并删除现有容器
docker stop blog-app 2>/dev/null || true
docker rm blog-app 2>/dev/null || true

# 从私有仓库拉取最新镜像
echo "📥 拉取最新镜像..."
docker pull localhost:5000/blog-app:latest

# 启动应用容器
echo "🐳 启动容器..."
docker run -d \
  --name blog-app \
  -p $APP_PORT:5000 \
  -v blog_data:/app \
  localhost:5000/blog-app:latest

echo "⏳ 等待应用启动..."
sleep 10

# 健康检查
echo "🔍 进行健康检查..."
if curl -s http://localhost:$APP_PORT/health > /dev/null; then
    echo "✅ 部署完成！"
    echo "🌐 访问地址: http://localhost:$APP_PORT"
    echo "🔍 健康检查: http://localhost:$APP_PORT/health"
    
    # 保存端口信息
    echo $APP_PORT > .app_port
else
    echo "❌ 部署失败，请检查日志: docker logs blog-app"
    exit 1
fi
