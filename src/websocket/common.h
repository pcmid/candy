// SPDX-License-Identifier: MIT
#ifndef CANDY_WEBSOCKET_COMMON_H
#define CANDY_WEBSOCKET_COMMON_H

#include <any>
#include <string>

namespace Candy {

enum class WebSocketMessageType {
    Message = 0,
    Open = 1,
    Close = 2,
    Error = 3,
};

class WebSocketConn {
public:
    // 重载小于号,用于作为 std::map 的 key
    bool operator<(const WebSocketConn &other) const;

    // 重载等于号,用于判断是否是相同的连接
    bool operator==(const WebSocketConn &other) const;

    // 用 std::any 隐藏具体实现,避免上层感知到表示连接的具体类型
    std::any conn;
};

// 消息会被放到消息队列里,从消息队列里取出来的时候至少要包含消息的类型和来源,
// 对于客户端,消息的来源只能是服务端,所以客户端的消息队列可以不填充 conn 字段
struct WebSocketMessage {
    WebSocketMessageType type;
    std::string buffer;
    WebSocketConn conn;
};

} // namespace Candy

#endif
