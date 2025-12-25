# ReportCore

### 通用数据包结构

| 字段名   | 类型         | 长度（字节） | 说明                   |
|----------|--------------|--------------|------------------------|
| type     | MessageType  | 1            | 消息类型，见下表       |
| data     | char[]       | 1023         | 消息内容，含义见下表   |

---

### 消息类型与 data 字段内容

| 类别   | type 枚举值                  | data 字段内容说明                                 |
|--------|------------------------------|---------------------------------------------------|
| 请求包 | DISCONNECT                   | 空                                                |
|        | GET_TIME                     | 空                                                |
|        | GET_NAME                     | 空                                                |
|        | GET_CLIENT_LIST              | 空                                                |
|        | SEND_MESSAGE                 | 目标客户端和消息内容，格式如 `ip:port:message`    |
| 响应包 | ANSWER_GET_TIME              | 时间字符串 `"YYYY-MM-DD HH:MM:SS"`                |
|        | ANSWER_GET_NAME              | 服务器名称字符串                                  |
|        | ANSWER_GET_CLIENT_LIST       | 客户端列表字符串，每行一个 `ip:port`              |
|        | ANSWER_SEND_MESSAGE          | "Ok"、"Not Found" 或 "Error"                      |
| 指示包 | REPOST                       | 被转发的消息内容                                  |

---

### 可视化结构图

```
+-----------+-----------------------------+
| type (1B) | data (1023B)                |
+-----------+-----------------------------+
```

---

所有包格式均为上述结构，type 字段区分包类型，data 字段内容随 type 不同而不同，详见上表。

<div style="page-break-after: always;"></div>

### 客户端（Client）结构图

```
+---------------------------------------------------+
|                GenericClient<ClientType>          |
+---------------------------------------------------+
| + Run()                                           |  <-- 唯一对外暴露的主接口
+---------------------------------------------------+
|                  (private)                        |
| + IsConnected()                                   |
| + handlePrintHelp()                               |
| + handleConnectServer()                           |
| + handleDisconnectServer()                        |
| + handleGetServerTime()                           |
| + handleGetServerName()                           |
| + handleGetClientList()                           |
| + handleSendMessage()                             |
| + handleExit()                                    |
+---------------------------------------------------+
         |（所有私有方法均通过模板转发调用）|
         v
+---------------------------------------------------+
|                ClientType（如Client）              |
+---------------------------------------------------+
| - server_socket_handle_                           |  <-- 私有数据成员
| - connected_                                      |
| - receiver_thread_                                |
| ...                                               |
| + IsConnected()                                   |  <-- 实现接口
| + handlePrintHelp()                               |
| + handleConnectServer()                           |
| + handleDisconnectServer()                        |
| + handleGetServerTime()                           |
| + handleGetServerName()                           |
| + handleGetClientList()                           |
| + handleSendMessage()                             |
| + handleExit()                                    |
| ...                                               |
+---------------------------------------------------+
```

---

<div style="page-break-after: always;"></div>

### 服务端（Server）结构图

```
+-------------------------------+
|           Server              |
+-------------------------------+
| + Run()                       |  <-- 唯一对外暴露的主接口
+-------------------------------+
| - server_socket_handle_       |  <-- 私有数据成员
| - server_addr_                |
| - clients_ (vector)           |
| - clients_mutex_              |
| ...                           |
| + clientHandlerThread()       |  <-- 私有线程处理
| + send(socket, message)       |
|                               |
+-------------------------------+
|         SocketInfo            |  <-- 私有结构体
|         ThreadArg             |
+-------------------------------+
```

---