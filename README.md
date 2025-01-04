# FilesBackup

```shell
git clone https://github.com/skyfishine/FilesBackup
docker build . --no-cache -t files_backup:1.0
docker run -dit -e DISPLAY=host.docker.internal:0 --name files_backup files_backup:1.0 /bin/bash
```