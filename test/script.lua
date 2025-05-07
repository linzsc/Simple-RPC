-- script.lua

-- 定义请求方法
function request()
    -- 构造请求体
    local body = '{"method_name": "add", "params": {"a": 5, "b": 3}}'
    local body_len = string.len(body)

    -- 构造 RpcHeader（二进制格式）
    local header = string.pack("<III", 0x12345678, 123, body_len)

    -- 返回请求（二进制数据）
    return wrk.format("POST", "/", nil, header .. body)
end