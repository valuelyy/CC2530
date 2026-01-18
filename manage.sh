#!/bin/bash

get_app_port() {
    if [ -f .app_port ]; then
        cat .app_port
    else
        echo "未知"
    fi
}

APP_PORT=$(get_app_port)

case "$1" in
    "start")
        echo "启动博客应用..."
        docker start blog-app
        echo "应用地址: http://localhost:$APP_PORT"
        ;;
    "stop")
        echo "停止博客应用..."
        docker stop blog-app
        ;;
    "restart")
        echo "重启博客应用..."
        docker restart blog-app
        sleep 3
        echo "应用地址: http://localhost:$APP_PORT"
        ;;
    "status")
        echo "=== 容器状态 ==="
        docker ps -a --filter "name=blog-app" --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"
        echo -e "\n=== 应用信息 ==="
        echo "访问地址: http://localhost:$APP_PORT"
        echo "健康检查: http://localhost:$APP_PORT/health"
        ;;
    "logs")
        echo "=== 应用日志 ==="
        docker logs -f blog-app
        ;;
    "clean")
        echo "清理环境..."
        docker stop blog-app 2>/dev/null || true
        docker rm blog-app 2>/dev/null || true
        docker volume rm blog_data 2>/dev/null || true
        rm -f .app_port 2>/dev/null || true
        echo "✅ 清理完成"
        ;;
    "url")
        echo "🌐 应用访问地址:"
        echo "首页: http://localhost:$APP_PORT"
        echo "健康检查: http://localhost:$APP_PORT/health"
        echo "文章列表: http://localhost:$APP_PORT/posts"
        ;;
    *)
        echo "📖 博客应用管理脚本"
        echo "当前端口: $APP_PORT"
        echo ""
        echo "用法: $0 {start|stop|restart|status|logs|clean|url}"
        echo ""
        echo "快速访问:"
        echo "  $0 url    # 显示所有访问地址"
        echo "  $0 status # 查看应用状态"
        echo "  $0 logs   # 查看实时日志"
        ;;
esac
