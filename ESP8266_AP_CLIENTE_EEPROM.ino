#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

int address = 0;
byte value;
byte modo=0;
/* Set these to your desired credentials. */
const char* ssid_AP = "ESP8266";
const char* password_AP = "PASSWORD";


const char* ssid     = "Consola";
const char* password = "tyrrenal";


int channel = 11;
ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
/////////// antirebote /////////////
volatile int contador = 0;   // Somos de lo mas obedientes
int n = contador ;
long T0 = 0 ;  // Variable global para tiempo

const int Boton_EEPROM=12;
const int LED=2;
 
void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup() {
  

  pinMode(INPUT,Boton_EEPROM);
  pinMode(OUTPUT,LED);
  
 
  Serial.begin(115200);
   delay(10);
  
  digitalWrite(LED,LOW);
   Serial.println("off");
    delay(500);
  digitalWrite(LED,HIGH);
   Serial.println("on");
    delay(500);
  digitalWrite(LED,LOW);
   Serial.println("off");
  delay(500);
  digitalWrite(LED,HIGH);
   Serial.println("on");
    delay(500);
  
   Serial.println();
   
  EEPROM.begin(512);
  EEPROM.write(address,modo);
  EEPROM.commit();
  
  value = EEPROM.read(address);
  
  Serial.println("Leyendo Direccion Registro: ");
  Serial.print(address);
  Serial.println("Dato: ");
  Serial.print(value, DEC);
  Serial.println();

  if(value){
  
    Serial.println();
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_AP, password_AP,11,1);// (*char SSID,*char PASS,int CHANNEL,int HIDDEN=1 NO_HIDDEN=0)

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");

    }
    else{
    
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);
         WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
  
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }

        Serial.println("");
        Serial.println("WiFi connected");  
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
         digitalWrite(LED,HIGH);
        
    }


  attachInterrupt( Boton_EEPROM, ServicioBoton,CHANGE);
   delay(250);
}

void loop() {
  server.handleClient();

  if (n != contador)
           {   //Serial.println(contador);
               n = contador ;
            
               
           }
}



void ServicioBoton()
   {
       if ( millis() > T0  + 250)
          {   contador++ ;
          Serial.println("entro");
              T0 = millis();
                 digitalWrite(LED,!digitalRead(LED));
          }
    }


