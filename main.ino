#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
const int D0 = 16;
const int D1 = 5;//亮度暗
const int D2 = 4;//亮度中
const int D3 = 0;//亮度亮
const int D4 = 2;//电源键
const int D5 = 14;//夜光键

String ssid = "ChainPray";
String password = "qdd20050629";

const char* mqttServer = "47.96.132.249";
const int mqttPort = 1883;
const char* mqttUser = "admin";
const char* mqttPassword = "public";
const char* topic1 = "d1mini/command";
const char* topic2 = "d1mini/state";
char* state = "OFF";

String wifi_config_file = "/wifiConfig.txt";

WiFiClient espClient;
PubSubClient client(espClient);

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);

  //启动闪存服务
  if (SPIFFS.begin()) {

    Serial.println("SPIFFS Started.");
    getWIFIconfig();
  } else {
    Serial.println("SPIFFS Failed to Started.");
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
  }
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  server.on("/WIFIconfig", handleWIFI);
  server.on("/type", TYPE);
  server.onNotFound(handleUserRequet); //处理没有匹配的处理程序的url
  server.begin();
  Serial.println("HTTP server started");
  //设置MATT服务
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  server.handleClient();

}
void callback(char* topic, byte* payload, unsigned int length) {
  // 处理订阅消息的回调函数
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char* cmd = new char[length + 1];
  for (int i = 0; i < length; i++) {
    cmd[i] = (char)payload[i];
  }
  cmd[length] = '\0';
  Serial.println(cmd);
  if (strcmp(topic, topic1) == 0) {

    if (strcmp(cmd, "ON") == 0 && strcmp(state, "OFF") == 0) {
      state = "ON";
      digitalWrite(0, HIGH);
    } else if (strcmp(cmd, "OFF") == 0 && strcmp(state, "ON") == 0) {
      state = "OFF";
      digitalWrite(0, LOW);
    }
    client.publish("esp01/state", state);
  } else if (strcmp(topic, topic2) == 0) {

  }

  delete[] cmd; // 释放动态分配的内存
  Serial.println();
}

void reconnect() {
  // 重连到MQTT服务器
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP01Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.publish("esp01/state", state);//每次重连上传台灯状态
      client.subscribe(topic1);
      client.subscribe(topic2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
// 获取文件类型
String getContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//处理文件请求
bool handleFileRead(String path) {
  // 将访问地址修改为/index.html便于SPIFFS访问
  if (path.endsWith("/")) {
    path = "/index.html";
  }

  String contentType = getContentType(path);  // 获取文件类型

  if (SPIFFS.exists(path)) {
    //    读取闪存文件并返回
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;                                  // 如果文件未找到，则返回false
}
//处理html类请求
void handleUserRequet() {

  // 获取用户请求网址信息
  String webAddress = server.uri();

  // 通过handleFileRead函数处处理用户访问
  bool fileReadOK = handleFileRead(webAddress);

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK) {
    server.send(404, "text/plain", "404 Not Found");
  }
}

//获取闪存中的WIFI配置
void getWIFIconfig() {
  if (SPIFFS.exists(wifi_config_file)) {
    Serial.print(wifi_config_file);
    Serial.println(" FOUND.");
    //建立File对象用于从SPIFFS中读取文件
    File dataFile = SPIFFS.open(wifi_config_file, "r");
    ssid = (dataFile.readStringUntil('\n'));
    password = (dataFile.readStringUntil('\n'));
    ssid.trim();
    password.trim();
    dataFile.close();
    //    Serial.println(ssid);
    //    Serial.println(password);
  } else {
    Serial.print(wifi_config_file);
    Serial.println(" NOT FOUND.");
  }


}

void handleWIFI() {
  //  ssid = server.arg("ssid").c_str();
  //  password = server.arg("password").c_str();

  File configFile = SPIFFS.open(wifi_config_file, "w");
  if (configFile) {
    configFile.println(server.arg("ssid")); // 将你的SSID写入文件
    configFile.println(server.arg("password")); // 将你的密码写入文件
    configFile.close();
    Serial.println("Config file written successfully");
    server.send(200, "text/html", "Config file written successfully");
  } else {
    Serial.println("Failed to open config file for writing");
    server.send(400, "text/html", "Failed to open config file for writing");
  }
}
void TYPE() {
   int delayTime = atoi(server.arg("time").c_str()); // 获取参数值
  digitalWrite(D1, HIGH);
  delay(delayTime);
  digitalWrite(D1, LOW);
  server.send(200, "text/html", "D1 is switched");
  Serial.print("D1 is switched");
  client.publish("D1mini/D1", "switch");

}
