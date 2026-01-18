#!/bin/bash

# 读取应用端口
if [ -f .app_port ]; then
    APP_PORT=$(cat .app_port)
else
    echo "❌ 找不到应用端口信息，请先运行 ./deploy.sh"
    exit 1
fi

echo "=== 博客应用功能测试 ==="
echo "应用端口: $APP_PORT"

echo -e "\n1. 测试健康检查..."
curl -s http://localhost:$APP_PORT/health | python -m json.tool

echo -e "\n2. 测试首页..."
curl -s http://localhost:$APP_PORT/ | grep -o '<title>.*</title>' || echo "首页访问正常"

echo -e "\n3. 测试文章列表..."
curl -s http://localhost:$APP_PORT/posts | head -5

echo -e "\n4. 测试添加文章..."
curl -s "http://localhost:$APP_PORT/add/自动化测试/这是通过脚本测试添加的文章"

echo -e "\n5. 查看容器状态..."
docker ps --filter "name=blog-app" --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"

echo -e "\n✅ 测试完成！"
echo "🌐 完整访问地址: http://localhost:$APP_PORT"
