/* DESCRIPCION:
  Proyecto: Hidrosmart
  Hardware:
    ESP32, marca Lyligo, modelo T-Display con pantalla TFT color, 135x240 
   uso de Blynk para
        - Mostrar datos de sensores de humedad
        - Configurar horario de riego
        - Configurar threshold de % humedad
        - Mostrar probabilidad de lluvia desde Weather API
 */

#include <SPI.h> 
#include <TFT_eSPI.h> //Biblioteca de la pantalla
#include <blynk_data.h> 
#include <BlynkSimpleEsp32.h> //Biblioteca de Blynk
#include <HTTPClient.h> //Libreria
#include <ArduinoJson.h> //Libreria para utilizar archivos json
#include <WidgetRTC.h> //Lib para sincronizar la hora 
#include <hidrosmart.h>

//Configuracion de Blynk
char auth[] = BLYNK_AUTH_TOKEN; //Token
char ssid[] = ""; //Nombre de la red wifi
char pass[] = ""; //Contrasena de la red

const int  RENDER_DISPLAY_TIEMPO = 1000L; //periodo del timer del diplay en ms
const int  WEATHER_API_QUERY_TIEMPO = 60000L; //periodo del timer en ms para solicitar los datos al API
const int  ACT_ZONAS_TIEMPO = 500L; //periodo en ms para actualizar zonas de riego

BlynkTimer hidrosmartTimer;  //Timer para tareas periodicas

TFT_eSPI tft = TFT_eSPI(135, 240); 
WidgetRTC rtc;

int timerRenderDisplayId;
int timerConsultarClimaId;
int timerActualizarZonasId;

float probabilidadLluvia = 0;
bool blynkConnected = false; //flag que indica el estado de conexion con Blynk

Clima climaCartago(API_KEY,CITY);
ZonaRiego* zonas[3]; //Crea un array donde se almacenaran las 3 zonas

void setup()
{
  Serial.begin(115200);
  delay(4000);
  Serial.println("< RIEGO INTELIGENTE >");
  //Configuracion de los timers
  hidrosmartTimer.setTimeout(3600000L, [] () {} ); // dummy/sacrificial Function
  timerRenderDisplayId = hidrosmartTimer.setInterval(RENDER_DISPLAY_TIEMPO, renderDisplay); //Actualizacion del diplay
  timerConsultarClimaId = hidrosmartTimer.setInterval(WEATHER_API_QUERY_TIEMPO,actualizarProbabiliadLLuvia); //Timer para consultar al API
  hidrosmartTimer.disable(timerConsultarClimaId);
  timerActualizarZonasId = hidrosmartTimer.setInterval(ACT_ZONAS_TIEMPO,actualizarZonas); //Actualizar las zona
  hidrosmartTimer.disable(timerConsultarClimaId);

  //Configuracion de la pantalla
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2); 
  tft.setCursor(0, 0);
  tft.drawString("HIDROSMART", tft.width() / 2, tft.height() / 2 - 18);
  tft.drawString("__________", tft.width() / 2, tft.height() / 2 );
  tft.setTextColor(TFT_BLUE);
  tft.drawString("ING. COMPUTADORES", tft.width() / 2, tft.height() / 2 + 18);
  tft.drawString("TEC", tft.width() / 2, tft.height() / 2 + 34 );
  tft.drawString("Para ver si corre", tft.width() / 2, tft.height() / 2 + 50); 



  
}


void loop()
{
  
}



