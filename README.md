### 固件在main文件夹里面，使用arduino烧录，记得flash里烧录上文件系统的html
### 固件源代码中wifi名车和密码设置成自己的，保证电脑和esp8266在同一局域网下
### 双击start.bat运行python上位机（环境已经配好）
</br>

### 基于python的keyboard库获取全局按键事件，并通过requests库向esp8266发送http请求，实现对relay继电器的开关
### 设了了热键ctrl+alt+shift快速切换继电器的响声(本质是控制继电器关闭、触发的时间)

</br>

### http连接的延迟还挺大的，之后考虑蓝牙或者无线红外之类的方法吧