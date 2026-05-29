# EchoServer 离线部署文档

## 1. 上传文件
将 echo_server 上传到服务器

## 2. 授权
chmod +x echo_server

## 3. 后台运行
nohup ./echo_server 0.0.0.0 8080 > server.log 2>&1 &

## 4. 查看日志
tail -f server.log

## 5. 停止服务
pkill echo_server

## 6. 开机自启（systemd）
参考文档内部配置即可。