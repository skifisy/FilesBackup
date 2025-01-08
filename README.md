# FilesBackup

## 通过docker运行

docker构建
```shell
git clone https://github.com/skyfishine/FilesBackup

docker build . --no-cache -t files_backup:1.0

docker run -dit -e DISPLAY=host.docker.internal:0 -e LANG=zh_CN.UTF-8 -e LC_ALL=zh_CN.UTF-8 --name files_backup files_backup:1.0 /bin/bash
```

编译运行：
```shell
cd build
cmake .. && make -j && make install
./backup
```

## 说明

本项目在docker中指定了环境变量`DISPLAY=host.docker.internal:0`，所以可以在宿主机中运行Xming服务器，例如XLaunch，从而显示docker的图形界面