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
    int porcentaje = map(raw,rawSeco,rawSaturado,0,100); //ajusta el valor del sensor a la escala 0-100
    int porcentajeOk = constrain(porcentaje,0,100); //Garantiza que porcentaje este entre 0 y 100
    //Serial.printf("obtenerHumedad: | raw : %d | porcentaje : %d | porcentajeOk : %d\n",raw,porcentaje,porcentajeOk);
    return porcentajeOk;
  };
};


