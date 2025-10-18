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
char ssid[] = "chimuelo"; //Nombre de la red wifi
char pass[] = "sueltalo"; //Contrasena de la red

const int  RENDER_DISPLAY_TIEMPO = 1000L; //periodo del timer para actualizar el diplay en ms (cada segundo)
const int  WEATHER_API_QUERY_TIEMPO = 60000L; //periodo del timer en ms para solicitar los datos al API (1 minuto)
const int  ACT_ZONAS_TIEMPO = 500L; //periodo en ms para actualizar zonas de riego (medio segundo)

//BlynkTime es una clase de la libreria de blynk
BlynkTimer hidrosmartTimer;  //Se crea un objeto Timer para tareas periodicas

TFT_eSPI tft = TFT_eSPI(135, 240); //Crea un objeto TFT_eSPI (representa la pantalla del esp32)
WidgetRTC rtc; //Crea un objeto WidgetRTC que permitira sincronizar la hora de Blynk con el esp32

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
  hidrosmartTimer.setTimeout(3600000L, [] () {} ); // Timer se sacrifico para inicializar correctamente los otros timers
  //setInterval(timer,funcion) ejecuta la funcion cada x milisegundos
  timerRenderDisplayId = hidrosmartTimer.setInterval(RENDER_DISPLAY_TIEMPO, renderDisplay); //Actualizacion del diplay (e)
  timerConsultarClimaId = hidrosmartTimer.setInterval(WEATHER_API_QUERY_TIEMPO,actualizarProbabiliadLLuvia); //Timer para consultar al API
  hidrosmartTimer.disable(timerConsultarClimaId); //deshabilita el timer que consulta el clima
  timerActualizarZonasId = hidrosmartTimer.setInterval(ACT_ZONAS_TIEMPO,actualizarZonas); //Actualizar las zona
  hidrosmartTimer.disable(timerConsultarClimaId); //Desactiva el timer para consultar la % de lluvia

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

  //Crear instancias de las zonas
  zonas[0] = new ZonaRiego(1,HUM1_PIN,V1_PIN,2000,3000,1,00,00,1,1,00,70,true);
  zonas[1] = new ZonaRiego(2,HUM2_PIN,V2_PIN,2000,3000,1,00,00,1,1,00,70,true);
  zonas[2] = new ZonaRiego(3,HUM3_PIN,V3_PIN,2000,3000,1,00,00,1,1,00,70,true);    
  for (int i = 0; i < 3; i++) zonas[i]->iniciar(); //inicia cada una de las zonas

  /* Init session to Blynk server */
  Blynk.begin(auth, ssid, pass); //Conecta el ES32 al wifi y luego al servidor de Blynk por medio del token
  Serial.println("Blynk IoT Cloud connected..."); //Print de confirmacion

  while (!blynkConnected); //Cuando blynkConnectec = true
  Serial.println("Iniciando sincronizacion de Blynk...");
  Blynk.syncAll(); //Le envia al dispositivo los valores actuales de los widgets de la app
  Serial.println("Blynk sincronizado...");

  probabilidadLluvia = climaCartago.obtenerProbabilidad(); //Llama al metodo para obtener valores del API
  //probabilidadLluvia = 70;

  //Habilitar los temporizadores
  hidrosmartTimer.enable(timerConsultarClimaId); //Actuvia de vuelta los timer
  hidrosmartTimer.enable(timerActualizarZonasId); 
  delay(3000); 
}

//Muestra datos en la pantalla 
void renderDisplay()
{ 
  //Serial.println("< Render display >");
  tft.setTextSize(2);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLUE);
  //tft.drawString("HUM: " + String(humedad_actual), tft.width() / 2, tft.height() / 2 - 20);
  tft.drawString("P.LLUVIA: " + String(probabilidadLluvia), tft.width() / 2, tft.height() / 2 ); //Probabilidad de lluvia 
  //tft.setTextColor(TFT_RED);
  //tft.drawString("" + String(sample), tft.width() / 2, tft.height() / 2 + 30);

  /*tft.setTextSize(4);
  if (estado_valvula == true){ //El rele en false es que esta activo
    tft.setTextColor(TFT_RED);
    tft.drawString("REGANDO", tft.width() / 2, tft.height() / 2 + 45);
  } else {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("--", tft.width() / 2, tft.height() / 2 + 45);    
  }*/

//El metodo virtualWrite envia datos a la aplicacion por medio de un pin virtual
//(pin virtual, dato a enviar)
//Los pines se definen en el archivo blynk_data.h
  Blynk.virtualWrite(blynk_hum1_actual,zonas[0]->obtenerHumedadActual()); //obtiene la humedad del metodo obtenerHumedadActual del objeto zona
  Blynk.virtualWrite(blynk_hum2_actual,zonas[1]->obtenerHumedadActual());
  Blynk.virtualWrite(blynk_hum3_actual,zonas[2]->obtenerHumedadActual());
  Blynk.virtualWrite(blynk_estado1_riego,zonas[0]->obtenerEstadoRiego()); //obtiene el estado de riego
  Blynk.virtualWrite(blynk_estado2_riego,zonas[1]->obtenerEstadoRiego());
  Blynk.virtualWrite(blynk_estado3_riego,zonas[2]->obtenerEstadoRiego());
  Blynk.virtualWrite(blynk_probabilidad_lluvia,probabilidadLluvia); //Obtiene la probabilidad de lluvia y la envia

}


void actualizarProbabiliadLLuvia()
{
  probabilidadLluvia = climaCartago.obtenerProbabilidad();

  //Descomentar solo para asignar un valor fijo de prueba
  //probabilidadLluvia = 65;
};

void actualizarZonas()
{
  //Serial.printf("Hora actual: %d:%d:%d\n",hour(),minute(),second());
  for (int i = 0; i < 3; i++) //Recorre el array de zonas y ejecuta el metodo actualizar en una de ellas
  {
    zonas[i]->actualizar(hour(),minute(),second(),probabilidadLluvia); //Llama al metodo actualizar de cada una de las zonas
  };
  Serial.println();
}


void loop()
{
  hidrosmartTimer.run();  //Mantiene los timer activos y ejecuta las tareas programadas
  Blynk.run(); //Mantiene blynk corriendo
}
  
BLYNK_CONNECTED()  
{
  // Synchronize time on connection
  rtc.begin(); //Sincroniza el relog con la hora del servidor
  blynkConnected = true; 
}

//Metodo que se ejecuta cuando llega un valor desde blynk
BLYNK_WRITE_DEFAULT()
//Param: objeto con el valor enviado por blynk
// param.asX convierte ese valor al tipo X escogido
{
  //Informacion obtenida de: https://docs.blynk.io/en/blynk-library-firmware-api/virtual-pins

  //param es un objeto que representa el valor que se recibe de la app
  int pin = request.pin; //Numero de pin virtual que cambio el dato - request contiene el pin que genero el evento
  int valueInt = param.asInt(); //Convierte el valor recibido a entero
  float valueFloat = param.asFloat(); //Convierte el valor recibido a float
  double valueDouble = param.asDouble(); //Convierte el valor recibido a double
  String valueString = param.asString();
  TimeInputParam t(param); //Clase de la libreria de Blynk que permite interpretar el tiempo ingresado
  //Se crea un objeto t y interpreta el valor param

  Serial.printf("Blynk downlink detected.  | pin: %d | As int: %d | As float: %f | As double : %f | As String : %s\n",
            pin,valueInt,valueFloat,valueDouble,valueString);

  //Sistema de casos donde se recibe el pin solicitado mediante request.pin
  switch(pin) 
  {
    //HUMEDAD MINIMA DE LAS ZONAS
    //Llama a establecerHumMin de la respectiva zona pasandole el valor enviado desde el pin respectivo
    case blynk_hum1_min:
      Serial.printf("Processing blynk_hum1_min datastream downlink\n");
      zonas[0]->establecerHumMin(valueInt);
      break;
    case blynk_hum2_min:
      Serial.printf("Processing blynk_hum2_min datastream downlink\n");
      zonas[1]->establecerHumMin(valueInt);
      break;
    case blynk_hum3_min:
      Serial.printf("Processing blynk_hum3_min datastream downlink\n"); 
      zonas[2]->establecerHumMin(valueInt);
      break;

    //HUMEDAD MAXIMA DE LAS ZONA  
    //Llama a establecerHumMax de la respectiva zona pasandole el valor enviado desde el pin respectivo
    case blynk_hum1_max:
      Serial.printf("Processing blynk_hum1_max datastream downlink\n");
      zonas[0]->establecerHumMax(valueInt);
      break;      
    case blynk_hum2_max:
      Serial.printf("Processing blynk_hum2_max datastream downlink\n");
      zonas[1]->establecerHumMax(valueInt);
      break;
    case blynk_hum3_max:
      Serial.printf("Processing blynk_hum3_max datastream downlink\n");
      zonas[2]->establecerHumMax(valueInt);
      break;

    //ESTADO DE RIEGO DE LAS ZONAS  
    case blynk_estado1_riego:
      Serial.printf("Processing blynk_estado1_riego datastream downlink\n");
      break;
    case blynk_estado2_riego:
      Serial.printf("Processing blynk_estado2_riego datastream downlink\n"); 
      break;
    case blynk_estado3_riego:
      Serial.printf("Processing blynk_estado3_riego datastream downlink\n");
      break;  

    //HORARIO DE RIEGO DE LAS ZONAS  
    //Llama a establecerHorario de la respectiva zona pasandole el valor enviado desde el pin respectivo
    //Primero verifica que las horas sean validas
    //T.hasStartTime o t.hasStopTime son metodo de la clase param que retorna true o false segun la validez de la hora ingresada
    case blynk_horario1_riego:
      Serial.printf("Processing blynk_horario1_riego datastream downlink\n");

      Serial.println("Detectado DOWNLINK para HORARIO. Valor recibido: ");
      // Process start time
      if (t.hasStartTime()) //Metodo de la clase param que retorna true o false segun la validez de la hora ingresada
      {
        Serial.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute() + ":" + t.getStartSecond());
      } else {
        Serial.println("No hay hora inicio definida");
        return;
      };
        // Process stop time
      if (t.hasStopTime()) {
        Serial.println(String("Stop: ") + t.getStopHour() + ":" + t.getStopMinute() + ":" + t.getStopSecond());
      } else {
        Serial.println("No hay hora final definida");
        return;
      };

      zonas[0]->establecerHorario(t.getStartHour(),t.getStartMinute(),t.getStartSecond(),t.getStopHour(),t.getStopMinute(),t.getStopSecond());
      break;
    case blynk_horario2_riego:
      Serial.printf("Processing blynk_hum3_max datastream downlink\n");

      Serial.println("Detectado DOWNLINK para HORARIO. Valor recibido: ");
      // Process start time
      if (t.hasStartTime()) {
        Serial.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute() + ":" + t.getStartSecond());
      } else {
        Serial.println("No hay hora inicio definida");
        return;
      };
        // Process stop time
      if (t.hasStopTime()) {
        Serial.println(String("Stop: ") + t.getStopHour() + ":" + t.getStopMinute() + ":" + t.getStopSecond());
      } else {
        Serial.println("No hay hora final definida");
        return;
      };

      zonas[1]->establecerHorario(t.getStartHour(),t.getStartMinute(),t.getStartSecond(),t.getStopHour(),t.getStopMinute(),t.getStopSecond());      
      break;
    case blynk_horario3_riego:
      Serial.printf("Processing blynk_estado1_riego datastream downlink\n");

      Serial.println("Detectado DOWNLINK para HORARIO. Valor recibido: ");
      // Process start time
      if (t.hasStartTime()) {
        Serial.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute() + ":" + t.getStartSecond());
      } else {
        Serial.println("No hay hora inicio definida");
        return;
      };
        // Process stop time
      if (t.hasStopTime()) {
        Serial.println(String("Stop: ") + t.getStopHour() + ":" + t.getStopMinute() + ":" + t.getStopSecond());
      } else {
        Serial.println("No hay hora final definida");
        return;
      };

      zonas[2]->establecerHorario(t.getStartHour(),t.getStartMinute(),t.getStartSecond(),t.getStopHour(),t.getStopMinute(),t.getStopSecond());      
      break;
    case blynk_limite_lluvia_z1:
      Serial.printf("Processing blynk_limite_lluvia_z1 datastream downlink\n");
      zonas[0]->establecerLimiteLluvia(valueInt);
      break;
    case blynk_limite_lluvia_z2:
      Serial.printf("Processing blynk_limite_lluvia_z2 datastream downlink\n");
      zonas[1]->establecerLimiteLluvia(valueInt);
      break;
    case blynk_limite_lluvia_z3:
      Serial.printf("Processing blynk_limite_lluvia_z3 datastream downlink\n");
      zonas[2]->establecerLimiteLluvia(valueInt);
      break;      
    default:
      Serial.printf("Datastream downlink won't be processed\n");
      break;
  }
}

