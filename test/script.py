import socket
import json
import struct
import threading
import time
from statistics import mean, stdev

class RpcClient:
    def __init__(self, host, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))
        self.msg_id = 0

    def call(self, method, params, service_name="CalculatorService"):
        # 构造请求体，包含 service_name
        request_body = json.dumps({
            "method_name": method,
            "params": params,
            "service_name": service_name  # 添加 service_name 字段
        }).encode('utf-8')

        header = struct.pack("<III", 0x12345678,  len(request_body), self.msg_id)
        self.msg_id += 1
        #print(f"Request: method={method}, params={params}, service_name={service_name}")
        #print(f"Request body: {request_body.decode('utf-8')}")

        self.sock.sendall(header + request_body)

        # 接收响应头部
        start_time = time.time()
        try:
            resp_header = self.sock.recv(12)
            magic, body_len, msg_id = struct.unpack("<III", resp_header)

            # 接收响应体
            resp_body = self.sock.recv(body_len)
            response = json.loads(resp_body.decode('utf-8'))

            # 打印响应结果
            #print(f"Response: code={response['code']}, result={response['result']}")

            end_time = time.time()
            latency = end_time - start_time

            return response, latency
        except Exception as e:
            print(f"Error: {e}")
            return None, None

def stress_test(latencies, errors):
    client = RpcClient("127.0.0.1", 12345)
    for _ in range(1000):
        result, latency = client.call("add", [2, 3])
        if latency is not None:
            latencies.append(latency * 1000)  # 转换为毫秒
        else:
            errors += 1
    return errors

# 启动10个线程并发测试
threads = []
latencies = []
errors = 0

start_time = time.time()

for _ in range(10):
    t = threading.Thread(target=stress_test, args=(latencies, errors))
    threads.append(t)
    t.start()

for t in threads:
    t.join()

end_time = time.time()

total_time = end_time - start_time
total_requests = 10 * 1000  # 10个线程，每个线程发送1000个请求

# 计算性能指标
if latencies:
    avg_latency = mean(latencies)
    stdev_latency = stdev(latencies)
    max_latency = max(latencies)
else:
    avg_latency = 0
    stdev_latency = 0
    max_latency = 0

requests_per_sec = total_requests / total_time
transfer_per_sec = 0  # 需要根据实际传输数据量计算

print(f"Running {int(total_time)}s test @ http://127.0.0.1:12345")
print(f"  10 threads and 10 connections")
print(f"  Thread Stats   Avg      Stdev     Max   +/- Stdev")
print(f"    Latency     {avg_latency:.2f}ms    {stdev_latency:.2f}ms   {max_latency:.2f}ms    -nan%")
print(f"    Req/Sec     {requests_per_sec:.2f}      {0.00}     {requests_per_sec:.2f}      -nan%")
print(f"  {total_requests} requests in {total_time:.2f}s, {transfer_per_sec:.2f}B read")
print(f"  Socket errors: connect 0, read {errors}, write 0, timeout 0")
print(f"Requests/sec:      {requests_per_sec:.2f}")
print(f"Transfer/sec:       {transfer_per_sec:.2f}B")