#ifndef blynk_data_h 
#define blynk_data_h
//Referencia:https://examples.blynk.cc/?board=ESP32&shield=ESP32%20WiFi&example=GettingStarted%2FPushData
/* BLYNK INFORMACION TEMPLATE */
#define BLYNK_TEMPLATE_ID "TMPL2dWfY6YWP"
#define BLYNK_TEMPLATE_NAME "Hidrosmart"
#define BLYNK_AUTH_TOKEN "KObwQyFNI_MoKEiU3b_OSBnk__WOF6fe"
#define BLYNK_PRINT Serial   // Comment this out to disable prints and save space


//Asignacion de los pines virtuales
//Downlink son los pines que reciben datos de Blynk
//En blynk cada widget tien un pin asociado (v0,v1,v2,etc..)
#define blynk_hum1_actual 0
#define blynk_hum2_actual 1
#define blynk_hum3_actual 2
#define blynk_hum1_min 3 //Downlink
#define blynk_hum2_min 4 //Downlink
#define blynk_hum3_min 5 //Downlink
#define blynk_hum1_max 6 //Downlink
#define blynk_hum2_max 7 //Downlink
#define blynk_hum3_max 8 //Downlink
#define blynk_estado1_riego 9  //Downlink
#define blynk_estado2_riego 10  //Downlink
#define blynk_estado3_riego 11  //Downlink
#define blynk_horario1_riego 12  //Downlink
#define blynk_horario2_riego 13  //Downlink
#define blynk_horario3_riego 14  //Downlink
#define blynk_probabilidad_lluvia 15
#define blynk_limite_lluvia_z1 16
#define blynk_limite_lluvia_z2 17
#define blynk_limite_lluvia_z3 18
#endif