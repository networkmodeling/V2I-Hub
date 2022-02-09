#!/bin/bash
chmod +x /home/yingtong/V2I-Hub/container/wait-for-it.sh
/home/yingtong/V2I-Hub/container/wait-for-it.sh 127.0.0.1:3306
chmod 777 /var/log/tmx
tmxctl --plugin CommandPlugin --enable
/usr/local/bin/tmxcore
