#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

/////////// antirebote /////////////
volatile int contador = 0;   // Somos de lo mas obedientes
int n = contador ;
long T0 = 0 ;  // Variable global para tiempo

const int Boton_EEPROM=0;
const int LED=2;
 
volatile int tiempoLed=80000000;
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

String ssid_leido;
String pass_leido;
String Topic1_leido;
String Topic2_leido;
String scanWifi;
int ssid_tamano = 0;
int pass_tamano = 0;
int Topic1_tamano = 0;
int Topic2_tamano = 0;

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
              "<fieldset align='center' style='border-style:solid; border-color:#336666; width:200px; height:300px; padding:10px; margin: 5px;'>"
              "<legend><strong>Configurar WI-FI</strong></legend>"
              "SSID: <br> <input name='ssid' type='text' size='15'/> <br><br>"
              "PASSWORD: <br> <input name='pass' type='password' size='15'/> <br><br>"
              "TOPIC1: <br> <input name='topic1' type='text' size='15'/> <br><br>"
              "TOPIC2: <br> <input name='topic2' type='text' size='15'/> <br><br>"
              "<input type='submit' value='Configurar Equipo' />"
              "</fieldset>"
              "</form>"
              "<iframe id='pantalla' name='pantalla' src='' width=900px height=400px frameborder='0' scrolling='no'></iframe>"
              "</body>"
              "</html>";


//*********  INTENTO DE CONEXION   *********************
void intento_conexion() {
  if (lee(70).equals("configurado")) {
    ssid_leido = lee(1);      //leemos ssid y password
    pass_leido = lee(30);
    Topic1_leido=lee(100);
    Topic2_leido=lee(130);

    Serial.print("SSID: ");  //Para depuracion
    Serial.println(ssid_leido);  //Para depuracion
     Serial.print("PASS: ");  //Para depuracion
    Serial.println(pass_leido);
     Serial.print("TOPIC 1: ");  //Para depuracion
    Serial.println(Topic1_leido);  //Para depuracion
     Serial.print("TOPIC 2: ");  //Para depuracion
    Serial.println(Topic1_leido);

    ssid_tamano = ssid_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
    pass_tamano = pass_leido.length() + 1;

    ssid_leido.toCharArray(ssid, ssid_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
    pass_leido.toCharArray(pass, pass_tamano);

    int cuenta = 0;
    WiFi.begin(ssid, pass);      //Intentamos conectar
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
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

  getssid = arregla_simbolos(getssid); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  getpass = arregla_simbolos(getpass);

  getTopic1 = arregla_simbolos(getTopic1); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
  getTopic2 = arregla_simbolos(getTopic2);

  ssid_tamano = getssid.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  pass_tamano = getpass.length() + 1;
  
  Topic1_tamano = getTopic1.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  Topic2_tamano = getTopic2.length() + 1;

  getssid.toCharArray(ssid, ssid_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  getpass.toCharArray(pass, pass_tamano);

  getTopic1.toCharArray(Topic1, Topic1_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
  getTopic2.toCharArray(Topic2, Topic2_tamano);

  Serial.print("ssid: ");
  Serial.println(ssid);     //para depuracion
  Serial.print("pass: ");
  Serial.println(pass);
  Serial.print("Topic1: ");
  Serial.println(Topic1);     //para depuracion
  Serial.print("Topic2: ");
  Serial.println(Topic2);

  WiFi.begin(ssid, pass); 
      //Intentamos conectar
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    cuenta++;
    if (cuenta > 20) {
      graba(70, "noconfigurado");
      server.send(200, "text/html", String("<h2>No se pudo realizar la conexion<br>no se guardaron los datos.</h2>"));
      return;
    }
  }
  Serial.print(WiFi.localIP());
  graba(70, "configurado");
  graba(1, getssid);
  graba(30, getpass);
  graba(100, getTopic1);
  graba(130, getTopic2);
  server.send(200, "text/html", String("<h2>Conexion exitosa a: "+ getssid + "<br> El pass ingresado es: " + getpass + "<br>Datos correctamente guardados." 
  + "<br> El Topic1 ingresado es: " + getTopic1 + "." 
  + "<br> El Topic2 ingresado es: " + getTopic2 + "." 
  + "<br>El equipo se reiniciara Conectandose a la red configurada."));
  delay(250);
  ESP.reset();
}


/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */

WiFiClient wifiClient;

void setup() {
  attachInterrupt( digitalPinToInterrupt(Boton_EEPROM), ServicioBoton, RISING);
  pinMode(Boton_EEPROM, INPUT_PULLUP);
  pinMode(LED,OUTPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  EEPROM.begin(4096);
  value = EEPROM.read(address);
  Serial.print("Leyendo Direccion Registro: ");
  Serial.println(address);
  Serial.print("Dato: ");
  Serial.println(value, DEC);

  if(value){
    tiempoLed=40000000;
    timer0_isr_init();
    timer0_write(ESP.getCycleCount() + tiempoLed); // 80MHz == 1sec
    timer0_attachInterrupt(ISR_Blink);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

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
    Serial.println(lee(70));
    Serial.println(lee(1));
    Serial.println(lee(30));
    
    }
  else{
     noInterrupts();
     tiempoLed=80000000;
     timer0_isr_init();
     timer0_write(ESP.getCycleCount() + tiempoLed); // 80MHz == 1sec
     timer0_attachInterrupt(ISR_Blink);
     interrupts();
     WiFi.mode(WIFI_STA);
     intento_conexion();
   }

  modo=0;
  EEPROM.write(0,modo);
  EEPROM.commit();
 
}

void loop() {
  server.handleClient();

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
}

void ServicioBoton()
   {
       if ( millis() > T0  + 250)
          {   contador++ ;
              T0 = millis();
            
             }
    }

void scanWIFIS(){
 Serial.println("scan start");

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
    delay(50);
    digitalWrite(LED,false);
    delay(50);
    digitalWrite(LED,true);
    delay(50);
    digitalWrite(LED,false);
    delay(50);
    digitalWrite(LED,true);
    delay(50);
    digitalWrite(LED,false);
  }


void ISR_Blink()
   {    digitalWrite(LED,!digitalRead(LED)); 
     timer0_write(ESP.getCycleCount() + tiempoLed); // 80MHz == 1sec
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

 
