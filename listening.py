import keyboard
import requests
import websocket

# WebSocket服务器地址
websocket_url = "ws://192.168.137.18:81"

# 建立WebSocket连接
ws = websocket.WebSocket()
ws.connect(websocket_url)

# 定义服务器的URL(写自己的esp8266/32的ip，要在同一个局域网中)
SERVER_URL = 'http://192.168.137.18/change'

flag = {}
relayTime = 1

requests.post(SERVER_URL, data={'time': relayTime + 1})

def mode_switch():
    global relayTime
    # 更改间隔时间（继电器响声）
    relayTime = (relayTime+1) % 4
    requests.post(SERVER_URL, data={'time': relayTime + 1})
    print(relayTime+1)

# 监听键盘释放的回调函数
def on_key(event):
    if(event.event_type == 'down'):
        if not event.name in flag or flag[event.name] == 'up':
            flag[event.name] = 'down'
            ws.send(event.name)
            print(event.name + '已按下')
    else:
        print(event.name + '松开')
        flag[event.name] = 'up'

keyboard.hook(on_key)
keyboard.add_hotkey('ctrl+alt+space', mode_switch)
# 保持程序运行
keyboard.wait()

# 保持WebSocket连接
while True:
    try:
        ws.run_forever()
    except websocket.WebSocketConnectionClosedException:
        print("WebSocket connection closed")
        ws.connect(websocket_url)