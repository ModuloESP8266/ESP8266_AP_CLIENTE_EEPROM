#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>


//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "192.168.0.106"
#define MQTT_SERVER_WAN "idirect.dlinkddns.com"

/////////// antirebote /////////////
volatile int contador = 0;   // Somos de lo mas obedientes
int n = contador ;
long T0 = 0 ;  // Variable global para tiempo

volatile int contador2 = 0;   // Somos de lo mas obedientes
int n2 = contador2 ;
long T02 = 0 ;  // Variable global para tiempo

const int Boton_EEPROM=0;
const int LED=2;
const int sw=14;
const int relay=12;

volatile int tiempoLed=800000000;
int address = 0;
byte value;
byte modo=0;
/* Set these to your desired credentials. */
const char* ssid_AP = "ESP8266";
const char* password_AP = "PASSWORD";

int channel = 11;

ESP8266WebServer server(80);


char ssid[20];
char pass[20];
char Topic1[20];
char Topic2[20];
char ServerWan[40]={'i','d','i','r','e','c','t','.','d','l','i','n','k','.','c','o','m'};
char ServerLan[20];

String ssid_leido;
String pass_leido;
String Topic1_leido;
String Topic2_leido;
String ServerWan_leido;
String ServerLan_leido;

String scanWifi;

int ssid_tamano = 0;
int pass_tamano = 0;
int Topic1_tamano = 0;
int Topic2_tamano = 0;
int ServerWan_tamano = 0;
int ServerLan_tamano = 0;

////// ADDRESS EEPROM

int dir_conf = 70;
int dir_ssid = 1;
int dir_pass = 30;
int dir_topic1 = 100;
int dir_topic2 = 130;
int dir_serverwan = 150;
int dir_serverlan = 190;



String arregla_simbolos(String a) {
  a.replace("%C3%A1", "á");
  a.replace("%C3%A9", "é");
  a.replace("%C3%A", "i");
  a.replace("%C3%B3", "ó");
  a.replace("%C3%BA", "ú");
  a.replace("%21", "!");
  a.replace("%23", "#");
  a.replace("%24", "$");
  a.replace("%25", "%");
  a.replace("%26", "&");
  a.replace("%27", "/");
  a.replace("%28", "(");
  a.replace("%29", ")");
  a.replace("%3D", "=");
  a.replace("%3F", "?");
  a.replace("%27", "'");
  a.replace("%C2%BF", "¿");
  a.replace("%C2%A1", "¡");
  a.replace("%C3%B1", "ñ");
  a.replace("%C3%91", "Ñ");
  a.replace("+", " ");
  a.replace("%2B", "+");
  a.replace("%22", "\"");
  return a;
}

String pral = "<html>"
              "<meta http-equiv='Content-Type' content='text/html  ; charset=utf-8'/>"
              "<title>CONFIG ESP8266</title> <style type='text/css'> body,td,th { color: #036; } body { background-color: #999; } </style> </head>"
              "<body> "
              "<h1>CONFIGURACION</h1><br>"
              "<form action='config' method='get' target='pantalla'>"
              "<fieldset align='center' style='border-style:solid; border-color:#336666; width:300px; height:500px; padding:10px; margin: 5px;'>"
              "<legend><strong>Configurar WI-FI</strong></legend>"
              "SSID: <br> <input name='ssid' type='text' size='15'/> <br><br>"
              "PASSWORD: <br> <input name='pass' type='password' size='15'/> <br><br>"
              "TOPIC1: <br> <input name='topic1' type='text' size='15'/> <br><br>"
              "TOPIC2: <br> <input name='topic2' type='text' size='15'/> <br><br>"
              "SERVER WAN: <br> <input name='serverwan' type='text' size='15'/> <br><br>"
              "SERVER LAN: <br> <input name='serverlan' type='text' size='15'/> <br><br>"
              "<input type='submit' value='Configurar Equipo' />"
              "</fieldset>"
              "</form>"
              "<iframe id='pantalla' name='pantalla' src='' width=900px height=400px frameborder='0' scrolling='no'></iframe>"
              "</body>"
              "</html>";


void setup() {

  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(1024);
  
  value = EEPROM.read(address);
  graba(150,"idirect.dlinkddns.com");
   delay(10);
  ReadDataEprom();
 
  Serial.print("Configuracion: ");
  Serial.println(lee(dir_conf));
  if(lee(dir_conf)=="noconfigurado"){
    value=1;
    }
  
  attachInterrupt( digitalPinToInterrupt(Boton_EEPROM), ServicioBoton, RISING);
  attachInterrupt( digitalPinToInterrupt(sw), ServicioBoton2, RISING);
  
  pinMode(Boton_EEPROM, INPUT);
  pinMode(LED,OUTPUT);
  
  
  if(value ){
    Serial.println("**********MODO CONFIGURACION************");
    scanWIFIS();
    Serial.print("Configuring access point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_AP, password_AP,11,0);// (*char SSID,*char PASS,int CHANNEL,int HIDDEN=1 NO_HIDDEN=0)
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", []() {server.send(200, "text/html", pral);});
    server.on("/config", wifi_conf);
    server.begin();
    Serial.println("Webserver iniciado...");
    Serial.println("Conectese a la ssid: ESP8266 con password: PASSWORD.");
   
    }
  else{
     Serial.println("**********MODO NORMAL************");
     
     pinMode(sw, INPUT_PULLUP);
     pinMode(relay,OUTPUT);
     digitalWrite(relay,true);
     WiFi.mode(WIFI_STA);
     intento_conexion();

     noInterrupts();
     tiempoLed=80000000;
     timer0_isr_init();
     timer0_write(ESP.getCycleCount() + 800000000L); // 80MHz == 10sec
     timer0_attachInterrupt(ISR_Blink);
     interrupts();

     
   }

  modo=0;
  EEPROM.write(0,modo);
  EEPROM.commit();
 
}

WiFiClient wifiClient;

PubSubClient client(ServerWan, 1883, callback, wifiClient);

//*********  INTENTO DE CONEXION   *********************
void intento_conexion() {
  
  if (lee(dir_conf).equals("configurado")) {
  
    
    Serial.print("SSID: ");  //Para depuracion
    Serial.println(ssid_leido);  //Para depuracion
    Serial.print("PASS: ");  //Para depuracion
    Serial.println(pass_leido);
    Serial.print("TOPIC 1: ");  //Para depuracion
    Serial.println(Topic1_leido);  //Para depuracion
    Serial.print("TOPIC 2: ");  //Para depuracion
    Serial.println(Topic1_leido);
    Serial.print("Server Lan MQTT: ");  //Para depuracion
    Serial.println(ServerWan_leido);  //Para depuracion
    Serial.print("Server Lan MQTT: ");  //Para depuracion
    Serial.println(ServerLan_leido);

    ssid_tamano = ssid_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
    pass_tamano = pass_leido.length() + 1;
    Topic1_tamano = Topic1_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
    Topic2_tamano = Topic2_leido.length() + 1;

    ssid_leido.toCharArray(ssid, ssid_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
    pass_leido.toCharArray(pass, pass_tamano);
    Topic1_leido.toCharArray(Topic1, Topic1_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
    Topic2_leido.toCharArray(Topic2, Topic2_tamano);

    int cuenta = 0;
    WiFi.begin(ssid, pass);      //Intentamos conectar
    while (WiFi.status() != WL_CONNECTED) {
      delay(400);
      blink50();
      cuenta++;
      
      if (cuenta > 20) {
        Serial.println("Fallo al conectar");
        return;
      }
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conexion exitosa a: ");
    Serial.println(ssid);
    Serial.println(WiFi.localIP());
    
     // mosquitto
    
        while (!client.connected()) {
      Serial.println("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
       Serial.println(clientName);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str(),"diego","24305314")){
        Serial.println("MTQQ Connected");
        client.subscribe(Topic1);
        //client.subscribe(Topic2);
      }

      //otherwise print failed for debugging
      else{Serial.println("Failed."); abort();}
    }
      
    
    blink50();
    digitalWrite(LED,true);
   
  }
 
 timer0_detachInterrupt();
  
}

//**** CONFIGURACION WIFI  *******
void wifi_conf() {
  int cuenta = 0;

  String getssid = server.arg("ssid"); //Recibimos los valores que envia por GET el formulario web
  String getpass = server.arg("pass");
  String getTopic1 = server.arg("topic1"); //Recibimos los valores que envia por GET el formulario web
  String getTopic2 = server.arg("topic2");
  String getServerWan = server.arg("serverwan"); //Recibimos los valores que envia por GET el formulario web
  String getServerLan = server.arg("serverlan");

  getssid = arregla_simbolos(getssid); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  getpass = arregla_simbolos(getpass);
  getTopic1 = arregla_simbolos(getTopic1); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  getTopic2 = arregla_simbolos(getTopic2);
  getServerWan = arregla_simbolos(getServerWan); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  getServerLan = arregla_simbolos(getServerLan);

  ssid_tamano = getssid.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  pass_tamano = getpass.length() + 1;
  Topic1_tamano = getTopic1.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  Topic2_tamano = getTopic2.length() + 1;
  ServerWan_tamano = getServerWan.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  ServerLan_tamano = getServerLan.length() + 1;

  getssid.toCharArray(ssid, ssid_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  getpass.toCharArray(pass, pass_tamano);
  getTopic1.toCharArray(Topic1, Topic1_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  getTopic2.toCharArray(Topic2, Topic2_tamano);
  getServerWan.toCharArray(ServerWan, ServerWan_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  getServerLan.toCharArray(ServerLan, ServerLan_tamano);
  
  Serial.print("ssid: ");
  Serial.println(ssid);     //para depuracion
  Serial.print("pass: ");
  Serial.println(pass);
  Serial.print("Topic1: ");
  Serial.println(Topic1);     //para depuracion
  Serial.print("Topic2: ");
  Serial.println(Topic2);
  Serial.print("Server Wan MQTT: ");
  Serial.println(ServerWan);
  Serial.print("Server Lan MQTT: ");
  Serial.println(ServerLan);
 
  WiFi.begin(ssid, pass); 
  //Intentamos conectar
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(400);
    blink50();
    Serial.print(".");
    cuenta++;
    if (cuenta > 20) {
      graba(dir_conf, "noconfigurado");
      server.send(200, "text/html", String("<h2>No se pudo realizar la conexion<br>no se guardaron los datos.</h2>"));
      return;
    }
  }
  
  Serial.print(WiFi.localIP());
  graba(dir_conf, "configurado");
  graba(dir_ssid, getssid);
  graba(dir_pass, getpass);
  graba(dir_topic1, getTopic1);
  graba(dir_topic2, getTopic2);
  graba(dir_serverwan, getServerWan);
  graba(dir_serverlan, getServerLan);
  server.send(200, "text/html", String("<h2>Conexion exitosa a: "+ getssid + "<br> Pass: '" + getpass + "' .<br>" 
  + "<br> Topic1: " + getTopic1 + ".<br>" 
  + "<br> Topic2: " + getTopic2 + ".<br>" 
  + "<br> Server Wan MQTT: " + getTopic1 + ".<br>" 
  + "<br> Server Lan MQTT: " + getTopic2 + ".<br>" 
  + "<br>El equipo se reiniciara Conectandose a la red configurada."));
  delay(50);
  ESP.reset();
}

void loop() {
  server.handleClient();

   //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {intento_conexion();}

  //maintain MQTT connection
  client.loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 


  if (n != contador)
  
           {  blink50();
              timer0_detachInterrupt();
              n = contador ;
              modo=1;
              EEPROM.write(0,modo);
              EEPROM.commit();
              delay(10);
              ESP.reset();
              
         }
   if (n2 != contador2)
  
           {  n2 = contador2 ;
              Serial.println("Relay!!");
              digitalWrite(relay,!digitalRead(relay));
              if(digitalRead(relay)){client.publish("prueba/light1/confirm", "Light1 On");}
              else{client.publish("prueba/light1/confirm", "Light1 Off");}
           
         }
}

void ServicioBoton(){
       if ( millis() > T0  + 250)
          {   contador++ ;
              T0 = millis();
            
             }
    }

void ServicioBoton2(){ if ( millis() > T02  + 250){   contador2++ ;
              T02 = millis();}
    }

void scanWIFIS(){
  Serial.println("scan start");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
     
    }
  }

}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}

void blink50(){
    digitalWrite(LED,true);
    delay(30);
    digitalWrite(LED,false);
    delay(30);
    digitalWrite(LED,true);
    delay(30);
    digitalWrite(LED,false);
    delay(30);
   
  }

void ISR_Blink(){ 
  
     //digitalWrite(LED,!digitalRead(LED)); 
     if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Se cayo la conexion...");
      ESP.reset();
     }
     timer0_write(ESP.getCycleCount() + 800000000); // 80MHz == 1sec
   }

//*******  G R A B A R  EN LA  E E P R O M  ***********
void graba(int addr, String a) {
  int tamano = (a.length() + 1);
  Serial.print(tamano);
  char inchar[30];    //'30' Tamaño maximo del string
  a.toCharArray(inchar, tamano);
  EEPROM.write(addr, tamano);
  for (int i = 0; i < tamano; i++) {
    addr++;
    EEPROM.write(addr, inchar[i]);
  }
  EEPROM.commit();
}

//*******  L E E R   EN LA  E E P R O M    **************

String lee(int addr) {
  String nuevoString;
  int valor;
  int tamano = EEPROM.read(addr);
  for (int i = 0; i < tamano; i++) {
    addr++;
    valor = EEPROM.read(addr);
    nuevoString += (char)valor;
  }
  return nuevoString;
}
 
/////////////   MQTT //////////////////// 
void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

  
   if(topicStr == Topic1){

      //turn the light on if the payload is '1' and publish to the MQTT server a confirmation message
        if(payload[0] == '1'){
          digitalWrite(relay, HIGH);
          client.publish(Topic1+'/confirm', "Light1 On");
          }
      //turn the light off if the payload is '0' and publish to the MQTT server a confirmation message
        else if (payload[0] == '0'){
          digitalWrite(relay, LOW);
          client.publish(Topic1+'/confirm', "Light1 Off");
        }
   }
  
}
/////////////// FIN MQTT //////////////////////


void ReadDataEprom(){
  
  
  ssid_leido = lee(dir_ssid);      //leemos ssid y password
  pass_leido = lee(dir_pass);
  Topic1_leido=lee(dir_topic1);
  Topic2_leido=lee(dir_topic2);
  ServerWan_leido=lee(dir_serverwan);
  ServerLan_leido=lee(dir_serverlan);
  
  ServerWan_leido = arregla_simbolos(ServerWan_leido); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  ServerLan_leido = arregla_simbolos(ServerLan_leido);
  ssid_leido = arregla_simbolos(ssid_leido); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  pass_leido = arregla_simbolos(pass_leido);
  Topic1_leido = arregla_simbolos(Topic1_leido); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  Topic2_leido = arregla_simbolos(Topic2_leido);
  
  ServerWan_tamano = ServerWan_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  ServerLan_tamano = ServerLan_leido.length() + 1;
  ssid_tamano = ssid_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  pass_tamano = pass_leido.length() + 1;
  Topic1_tamano = Topic1_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  Topic2_tamano = Topic2_leido.length() + 1;

  ServerWan_leido.toCharArray(ServerWan, ServerWan_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  ServerLan_leido.toCharArray(ServerLan, ServerLan_tamano);
  ssid_leido.toCharArray(ssid, ssid_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
  pass_leido.toCharArray(pass, pass_tamano);
  Topic1_leido.toCharArray(Topic1, Topic1_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
  Topic2_leido.toCharArray(Topic2, Topic2_tamano);

   
  Serial.print("ssid: ");
  Serial.println(ssid);     //para depuracion
  Serial.print("pass: ");
  Serial.println(pass);
  Serial.print("Topic1: ");
  Serial.println(Topic1);     //para depuracion
  Serial.print("Topic2: ");
  Serial.println(Topic2);
  Serial.print("Server Wan MQTT: ");
  Serial.println(ServerWan);
  Serial.print("Server Lan MQTT: ");
  Serial.println(ServerLan);
 
  
  
  }
