#ifndef hidrosmart_h 
#define hidrosmart_h

// Hidrosmart configuracion
const String API_KEY = "1d8f3dd4721fe20a2224b286112fe0ec"; // OpenWeatherMap apikey
const String CITY    = "Cartago,CR";  // Openweather location
#define V1_PIN 17     //Rele zona 1
#define V2_PIN 13     //Rele zona 2
#define V3_PIN 15     //Rele zona 3
#define HUM1_PIN 39   //Pin sensor  capacitivo humedad z1
#define HUM2_PIN 32   //Pin sensor  humedad z2
#define HUM3_PIN 33   //Pin sensor  humedad z3

class SensorHumedad
{  
  private:
  int rawSeco = 4095;  //Valor raw (data sin procesar) del sensor cuando el suelo está seco
  int rawSaturado = 100;  //Valor raw del sensor cuando el suelo esta saturado de agua
  int pin; //Pin del sensor
  int raw; //Raw son los volores crudos
  int humedad;

  public: SensorHumedad(){};  //metodo constructor
  
  void iniciar(int _pin) //Asigna el pin
  { 
    pin = _pin;
  }

  int obtenerHumedad() //Metodo que obtiene la humedad y procesa el resultado del sensor
  {
    raw = analogRead(pin); //valores obtenidos por el sensor (sin procesar)
    int porcentaje = map(raw,rawSeco,rawSaturado,0,100); //Ajusta el valor del sensor a la escala 0-100
    int porcentajeOk = constrain(porcentaje,0,100); //Garantiza que porcentaje este entre 0 y 100
    //Serial.printf("obtenerHumedad: | raw : %d | porcentaje : %d | porcentajeOk : %d\n",raw,porcentaje,porcentajeOk);
    return porcentajeOk;
  };
};


////////////////////////////////
//##############################
////////////////////////////////

class ZonaRiego
{
  private:
  int id; // ID de la zona
  int sensorPin; //Pin del sensor de humedad
  int relePin; //Pin del rele
  int humedadMin; 
  int humedadMax;
  int horaInicio, minutoInicio, segInicio;
  int horaFin, minutoFin, segFin;
  int limiteProbLluvia; // % limite de probabilidad de lluvia
  bool activo; //Estado de la zona
  bool estadoRiego; //Estado del riego (true=esta regando)(false=no esta rangando)
  SensorHumedad sensorHum;  //Objeto del sensor;

  public: //Constructor
  ZonaRiego(int _id, int _sensorPin, int _relePin,
            int _hMin, int _hMax,
            int _hIni, int _mIni, int _sIni, int _hFin, int _mFin, int _sFin,
            int _limitProbLluvia,
            bool _activo = true
            ) : id(_id), sensorPin(_sensorPin), relePin(_relePin),
            humedadMin(_hMin), humedadMax(_hMax),
            horaInicio(_hIni), minutoInicio(_mIni), segInicio(_sIni),
            horaFin(_hFin), minutoFin(_mFin), segFin(_sFin),
            limiteProbLluvia(_limitProbLluvia),
            activo(_activo){}; //Asigna los valores 
  
  void iniciar()
  {
    sensorHum.iniciar(sensorPin); //Le asigna el pin al sensor de humedad
    pinMode(relePin,OUTPUT); //Configura el pin del rele como salida
    digitalWrite(relePin, HIGH); //Para que el pin del rele por default envie 3.3V
  };

  void actualizar(int hora, int minuto, int segundo, float probLluvia) //Actualiza el estado de la zona
  {
    if (!activo) return; //Si la zona esta inactiva no devuelve nada
    int humedad = sensorHum.obtenerHumedad(); // Toma la humedad del sensor
    //Serial.printf("Sensor humedad z%d | pin: %d | raw: %d\n",id,sensorPin,humedad);
    bool dentroHorario = estaDentroHorario(hora, minuto, segundo); //Verifica que se este en el horario de riego

    if ((int(probLluvia) < limiteProbLluvia) && dentroHorario && (humedad < humedadMin)) //Analiza los datos para verificar que se cumplan las condiciones
    {
      estadoRiego = true; //Si todas las condiciones se cumplen
      encender();
    } else {
      estadoRiego = false;
      apagar();
    };

    Serial.printf("Zona %d | Riego: %s| Hum rango: %d a %d | Hum actual: %d | Dentro Horario: %s (%d:%d:%d-%d:%d:%d) | Prob Lluvia: %.2f | Limite Lluvia: %d\n",
                  id, 
                  estadoRiego ? "ACTIVO" : "INACTIVO",
                  humedadMin, humedadMax, humedad,
                  dentroHorario ? "Sí" : "No",
                  horaInicio,minutoInicio,segInicio,horaFin,minutoFin,segFin,
                  probLluvia,
                  limiteProbLluvia);
  };

  void establecerHorario(int _hIni, int _mIni, int _sIni, int _hFin, int _mFin, int _sFin) //Establecer horario de riego
  {
    horaInicio = _hIni;
    minutoInicio = _mIni;
    segInicio = _sIni;
    horaFin = _hFin;
    minutoFin = _mFin;
    segFin = _sFin;
  }

  void establecerHumMin(int humMin) 
  {
    humedadMin = humMin;
  };

  void establecerHumMax(int humMax){
    humedadMax = humMax;
  };

  void establecerLimiteLluvia(int limiteLLuvia) // de probabilidad de lluvia
  {
    limiteProbLluvia = limiteLLuvia;
  };

  int obtenerHumedadActual()
  {
    return sensorHum.obtenerHumedad(); //Obtiene la humedad actual del sensor
  }

  bool obtenerEstadoRiego() //Returna si esta regando o no
  {
    return estadoRiego;
  }

  private:
  bool estaDentroHorario(int hora, int minuto, int segundo) //Verifica que se esta dentro de horario
  { //Convierte todo a segundos para realizar la comparacion
    int actual = hora * 60 * 60 + minuto * 60 + segundo;
    int inicio = horaInicio * 60 * 60 + minutoInicio * 60 + segInicio;
    int fin = horaFin * 60 *60 + minutoFin * 60 + segFin;
    return (actual >= inicio && actual <= fin); //Si la hora actual esta dentro del rango, devuelve true
  };

  void encender() {
    //Serial.printf(" Encendiendo riego zona %d\n", id);
    digitalWrite(relePin, LOW); //Enciende el rele "enviando" 0
  };

  void apagar() {
    //Serial.printf(" Apagando riego zona %d\n", id);
    digitalWrite(relePin, HIGH); //Apaga el rele enviando 1 (3.3V)
  };

};

////////////////////////
//#####################
///////////////////////

class Clima {
  private:
  String url; //Direccion URL del API

  public:
  Clima(String apiKey, String ciudad)
  {
    //url = "https://api.openweathermap.org/data/2.5/weather?q=" + ciudad + "&APPID=" + apiKey + "&units=metric";
    //url = "https://api.openweathermap.org/data/2.5/weather?q=Cartago,CR&APPID=1d8f3dd4721fe20a2224b286112fe0ec&units=metric";
    url = "https://pro.openweathermap.org/data/2.5/forecast/hourly?q=" + ciudad + "&appid=" + apiKey;
  };//Constructor Clima

  float obtenerProbabilidad() {
    //if (WiFi.status() != WL_CONNECTED) return;
    HTTPClient http;
    http.begin(url); //inicia la conexion con el servidor
    int httpCode = http.GET(); //Codigo de estado
    float probabilidad;
    String payload = http.getString(); //Obtiene string de respuesta enviado por la API
    //Serial.println("OPENWEATHER response");
    //Serial.println(payload);

    if (httpCode == 200) //200 significa que la solicitud fue exitosa 
    {
      StaticJsonDocument<2048> doc; //Crea un objeto JSON con 2048 bytes como capacidad
      deserializeJson(doc, payload); //Convierte el texto recibido en un objeto JSON dentro de doc
      probabilidad = doc["list"][0]["pop"]; // Busca las posibilidad de lluvia dentro del JSON 
      Serial.printf(" Probabilidad de lluvia: %.0f%%\n", probabilidad * 100); //Imprime la posibilidad
    } else {
      Serial.printf("Error HTTP: %d\n", httpCode); 
    };
    http.end();
    return (probabilidad*100); //Retorna la posibilidad de lluvia en porcentaje
  } //Actualizar
};

#endif
