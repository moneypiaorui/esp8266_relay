import keyboard
import requests

# 定义服务器的URL(写自己的esp8266/32的ip，要在同一个局域网中)
SERVER_URL = 'http://192.168.137.227/type'

flag = {}
relayTime = 1

def mode_switch():
    global relayTime
    # 更改间隔时间（继电器响声）
    relayTime = (relayTime+1) % 4
    print('时间间隔改为'+str(relayTime+1))

# 监听键盘释放的回调函数
def on_key(event):
    if(event.event_type == 'down'):
        if not event.name in flag or flag[event.name] == 'up':
            flag[event.name] = 'down'
            # 构建请求体
            payload = {'key': event.name, 'time': relayTime+1 }
            # 发送POST请求给服务器
            requests.post(SERVER_URL, data=payload)
            print(event.name + '已按下')
    else:
        print(event.name + '松开')
        flag[event.name] = 'up'

keyboard.hook(on_key)
keyboard.add_hotkey('ctrl+alt+shift', mode_switch)

# 保持程序运行
keyboard.wait()