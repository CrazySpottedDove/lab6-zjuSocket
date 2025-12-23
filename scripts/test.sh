#!/bin/bash
TEST_DIR=./build/test_client
CLIENT_NUM=10
for ((i=1; i<=CLIENT_NUM; i++)); do
    echo "启动客户端实例 #$i"
    ${TEST_DIR} &
done

wait
echo "所有客户端实例已启动完成"