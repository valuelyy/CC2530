# 构建镜像
echo "=== 构建Docker镜像 ==="
docker build -t blog-app:v1 .

# 检查可用端口并测试运行
echo "=== 查找可用端口进行测试 ==="
for port in 5001 5002 5003 5004; do
    if ! ss -tuln | grep ":$port " > /dev/null; then
        TEST_PORT=$port
        break
    fi
done

echo "使用端口 $TEST_PORT 进行测试..."

# 测试运行（使用找到的可用端口）
docker run -d --name test-app -p $TEST_PORT:5000 blog-app:v1
sleep 5

echo "=== 测试应用功能 ==="
curl -s http://localhost:$TEST_PORT/health || echo "应用还在启动中..."
sleep 3
curl -s http://localhost:$TEST_PORT/health

echo "=== 清理测试容器 ==="
docker stop test-app && docker rm test-app

# 推送到私有仓库
echo "=== 推送镜像到私有仓库 ==="
docker tag blog-app:v1 localhost:5000/blog-app:latest
docker push localhost:5000/blog-app:latest

echo "=== 验证推送 ==="
curl -s http://localhost:5000/v2/_catalog 