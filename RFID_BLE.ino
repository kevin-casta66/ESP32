#include <SPI.h>      // incluye libreria bus SPI
#include <MFRC522.h>      // incluye libreria especifica para MFRC522
#include <EEPROM.h>  
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


#define RST_PIN  22    // constante para referenciar pin de reset
#define SS_PIN   21     // constante para referenciar pin de slave select
#define TIEMPOS_MOTOR1_ADDR 250
#define TIEMPOS_MOTOR2_ADDR 260
#define TIEMPOS_MOTOR3_ADDR 270
#define TIEMPOS_MOTOR4_ADDR 280
#define MAX_TEMP_LEN 10  // Longitud máxima del tiempo
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristicRX = NULL;
BLECharacteristic* pCharacteristicTX = NULL;

std::string previousdata = "";


MFRC522 mfrc522(SS_PIN, RST_PIN); // crea objeto mfrc522 enviando pines de slave select y reset

byte LecturaUID[4];         // crea array para almacenar el UID leido
byte Usuario1[4]= {0xAE, 0xEF, 0xEC, 0x60} ;    // UID de tarjeta leido en programa 1
byte Usuario2[4]= {0xAE, 0x5E, 0xCB, 0x9E} ;    // UID de llavero leido en programa 1



const int M1 = 12;
const int M2 = 13;
const int M3 = 26;
const int M4 = 27;
int ESTADO1;
int ESTADO2;
unsigned long tiempoMotor1 = 0;
unsigned long tiempoMotor2 = 0;
unsigned long tiempoMotor3 = 0;
unsigned long tiempoMotor4 = 0;
unsigned long nuevoTiempo=0;
bool deviceConnected = false;
std::string data ="";
String mensajeDef="ERROR EN PROCESAMIENTO";
String mensaje=mensajeDef;

String receivedData="";

void removeSpaces(std::string& str) {
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
}

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;

        // Reiniciar el servidor BLE
        BLEDevice::startAdvertising();
        pServer->startAdvertising();
    }
};




void setup() {
  EEPROM.begin(600);
  Serial.begin(115200);

  SPI.begin();        // inicializa bus SPI
  mfrc522.PCD_Init();     // inicializa modulo lector
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(M3, OUTPUT);
  pinMode(M4, OUTPUT);


BLEDevice::init("ROVERE");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

    pCharacteristicRX = pService->createCharacteristic(
                        BLEUUID(CHARACTERISTIC_UUID_RX),
                        BLECharacteristic::PROPERTY_WRITE
                    );

    pCharacteristicTX = pService->createCharacteristic(
                        BLEUUID(CHARACTERISTIC_UUID_TX),
                        BLECharacteristic::PROPERTY_NOTIFY
                    );

    pCharacteristicTX->addDescriptor(new BLE2902());

    pService->start();
  
    BLEDevice::startAdvertising();
  ReadTime();

 // while(!SerialBT.connected()) {
   // delay(1000);
  //}

}



void loop() {


    
  if (mfrc522.PICC_IsNewCardPresent()){ 
    
  if (mfrc522.PICC_ReadCardSerial()){ 
  // si no puede obtener datos de la tarjeta
              // retorna al loop esperando por otra tarjeta
      Serial.println("TARJETA INGRESADA");
    
    Serial.print("UID:");       // muestra texto UID:
    for (byte i = 0; i < mfrc522.uid.size; i++) { // bucle recorre de a un byte por vez el UID
      if (mfrc522.uid.uidByte[i] < 0x10){   // si el byte leido es menor a 0x10
        Serial.print(" 0");       // imprime espacio en blanco y numero cero
        }
        else{           // sino
          Serial.print(" ");        // imprime un espacio en blanco
          }
          Serial.print(mfrc522.uid.uidByte[i], HEX);    // imprime el byte del UID leido en hexadecimal
          LecturaUID[i]=mfrc522.uid.uidByte[i];     // almacena en array el byte del UID leido      
          }
          
          Serial.print("\t");         // imprime un espacio de tabulacion             
                    
          if(comparaUID(LecturaUID, Usuario1)){   // llama a funcion comparaUID con Usuario1
          Serial.println("token 1");
              if(ESTADO1==0){
                digitalWrite(M1, HIGH);
                delay(tiempoMotor1);
                Serial.println("Motor 1 abierto");
                digitalWrite(M1, LOW);
                ESTADO1=1;
                EEPROM.put(500,ESTADO1);
                EEPROM.commit();

                  }else if(ESTADO1==1){
                    digitalWrite(M2, HIGH);
                    delay(tiempoMotor2);
                    Serial.println("Motor 1 cerrado");
                    digitalWrite(M2, LOW);
                    ESTADO1=0;
                    EEPROM.put(500,ESTADO1);
                    EEPROM.commit();

                  }
                 // si retorna verdadero muestra texto bienvenida

          }else if(comparaUID(LecturaUID, Usuario2)){ 
            Serial.println("token 2");
             if(ESTADO2==0){
                digitalWrite(M3, HIGH);
                delay(tiempoMotor3);
                Serial.println("Motor 2 abierto");
                 digitalWrite(M3, LOW);
                 ESTADO2=1;
                EEPROM.put(501,ESTADO2);
                EEPROM.commit();

                  }else if(ESTADO2==1){
                    digitalWrite(M4, HIGH);
                    delay(tiempoMotor4);
                    Serial.println("Motor 2 cerrado");
                    digitalWrite(M4, LOW);
                    ESTADO2=0;
                    EEPROM.put(501,ESTADO2);
                    EEPROM.commit();
                   }

          }else {          // si retorna falso
            Serial.println("No te conozco");    // muestra texto equivalente a acceso denegado          
          
          }
          mfrc522.PICC_HaltA();     // detiene comunicacion con tarjeta      
  }
  }      

    if (deviceConnected) {
        data = std::string(pCharacteristicRX->getValue().c_str());
        if (data.length() > 0 && data != previousdata) {
            Serial.print("Datos recibidos: ");
            Serial.println(data.c_str());
            previousdata = data;
            removeSpaces(data);
            Serial.println("Datos despues de remove: ");
            Serial.println(data.c_str());

            if(data=="tiempo"){ 
               Guardartiempos();
              }   
              if(data=="info"){ 
               mensaje="El tiempo del motor 1 es de: "+String(tiempoMotor1)+ " milisegundos \n"
               "El tiempo del motor 2 es de: "+String(tiempoMotor2)+ " milisegundos \n"
               "El tiempo del motor 3 es de: "+String(tiempoMotor3)+ " milisegundos \n"
               "El tiempo del motor 4 es de: "+String(tiempoMotor4)+ " milisegundos \n";
               pCharacteristicTX->setValue(mensaje.c_str());
               pCharacteristicTX->notify();
               Serial.println(mensaje);
              }
              if(data=="programar"){
                mensaje="Elige una opción: \n"
                "1. Configurar Tiempo de motores \n";
                Serial.println(mensaje);
                pCharacteristicTX->setValue(mensaje.c_str());
                pCharacteristicTX->notify();
                while (pCharacteristicRX->getValue().indexOf('\n') == -1) {
                delay(100);
                }
                receivedData = String(pCharacteristicRX->getValue().c_str());
                // Limpia el valor de la característica RX
                pCharacteristicRX->setValue("");
                data="";
                menu();
              }   
            if(data=="A"){
              if(!ESTADO1){ 
              digitalWrite(M1, HIGH);
              delay(tiempoMotor1);
              digitalWrite(M1,LOW);
              Serial.println("Abierto en: "+ String(tiempoMotor1)+" milisegundos \n");
              mensaje=("Abierto en: "+ String(tiempoMotor1)+" milisegundos \n");
              pCharacteristicTX->setValue(mensaje.c_str());
              pCharacteristicTX->notify();
              ESTADO1=1;
              EEPROM.put(500,ESTADO1);
              EEPROM.commit();
              
                }else if(ESTADO1){
                  Serial.println("El compartimiento ya se encuentra abierto \n");
                  mensaje=("El compartimiento ya se encuentra abierto \n");
                  pCharacteristicTX->setValue(mensaje.c_str());
                  pCharacteristicTX->notify();
                }
                
            }
            if(data=="B" ){
              if(ESTADO1){ 
              digitalWrite(M2, HIGH);
              delay(tiempoMotor2);
              digitalWrite(M2,LOW);
              Serial.println("Cerrado en:"+ String(tiempoMotor2)+" milisegundos \n");
              mensaje=("Cerrado en:"+ String(tiempoMotor2)+" milisegundos \n");
              pCharacteristicTX->setValue(mensaje.c_str());
              pCharacteristicTX->notify();
              ESTADO1=0;
              EEPROM.put(500,ESTADO1);
              EEPROM.commit();
              
            }else if(!ESTADO1){ 
              Serial.println("El compartimiento ya se encuentra cerrado \n");
              mensaje=("El compartimiento ya se encuentra cerrado \n");
              pCharacteristicTX->setValue(mensaje.c_str());
              pCharacteristicTX->notify();
            }
             }

            if(data=="C"){
              if(!ESTADO2){ 
              digitalWrite(M3, HIGH);
              delay(tiempoMotor3);
              digitalWrite(M3,LOW);
              Serial.println("Abierto en: "+ String(tiempoMotor3)+" milisegundos \n");
              mensaje=("Abierto en: "+ String(tiempoMotor3)+" milisegundos \n");
              pCharacteristicTX->setValue(mensaje.c_str());
              pCharacteristicTX->notify();
              ESTADO2=1;
              EEPROM.put(501,ESTADO2);
              EEPROM.commit();
              
                }else if(ESTADO2){
                  Serial.println("El compartimiento ya se encuentra abierto \n");
                  mensaje=("El compartimiento ya se encuentra abierto \n");
                  pCharacteristicTX->setValue(mensaje.c_str());
                  pCharacteristicTX->notify();
                }
                
            }
            if(data=="D" ){
              if(ESTADO2){ 
              digitalWrite(M4, HIGH);
              delay(tiempoMotor4);
              digitalWrite(M4,LOW);
              Serial.println("Cerrado en:"+ String(tiempoMotor4)+" milisegundos \n");
              mensaje=("Cerrado en:"+ String(tiempoMotor4)+" milisegundos \n");
              pCharacteristicTX->setValue(mensaje.c_str());
              pCharacteristicTX->notify();
              ESTADO2=0;
              EEPROM.put(501,ESTADO2);
              EEPROM.commit();
              
            }else if(!ESTADO1){ 
              Serial.println("El compartimiento ya se encuentra cerrado \n");
              mensaje=("El compartimiento ya se encuentra cerrado \n");
              pCharacteristicTX->setValue(mensaje.c_str());
              pCharacteristicTX->notify();
            }

            
            }
            
            
        
      }
  }
  
}  


void menu(){
  while (pCharacteristicRX->getValue().length() == 0) {
    delay(100);
   }
   Serial.println("Avanzo el while");
   data = std::string(pCharacteristicRX->getValue().c_str());
  if (data.length() > 0 && data != previousdata) {
   if (data.length() > 0) {
    Serial.println("El dato que entra al switch case es: ");
    Serial.print(data.c_str());
    switch (data[0]) {
        case '1':
          Serial.println("Eligió opcion 1");  
          mensaje=("Eligió opcion 1 \n");
          pCharacteristicTX->setValue(mensaje.c_str());
          pCharacteristicTX->notify();
          Guardartiempos();
          data="";
          break;  
        default:
          mensaje=("opcion invalida \n");
          pCharacteristicTX->setValue(mensaje.c_str());
          pCharacteristicTX->notify();
          return;
          break;
        
      }
   }
    
   else{
    data=""; 
    return;
   }
  }
} 


void Guardartiempos(){
    Serial.println("Seleccione el motor para configurar el tiempo: \n"
                    "1. Motor 1 \n"
                    "2. Motor 2 \n"
                    "3. Motor 3 \n"
                    "4. Motor 4 \n");


 mensaje= "Seleccione el motor para configurar el tiempo: \n"
                    "1. Motor 1 \n"
                    "2. Motor 2 \n"
                    "3. Motor 3 \n"
                    "4. Motor 4 \n";
  pCharacteristicTX->setValue(mensaje.c_str());
  pCharacteristicTX->notify();

  while (pCharacteristicRX->getValue().indexOf('\n') == -1)  {
    delay(100);
  }
    receivedData = String(pCharacteristicRX->getValue().c_str());
  // Limpia el valor de la característica RX
  pCharacteristicRX->setValue("");

  Serial.println("hasta aqui llega el codigo 1");

  while (pCharacteristicRX->getValue().length() == 0) {
    delay(100);
   }
  
  Serial.println("hasta aqui llega el codigo 2");
  
  char opcion = pCharacteristicRX->getValue()[0];
  Serial.println("Opción recibida: " + String(opcion)); // Agregar mensaje de depuración

  
  switch (opcion) {
  case'1': 
    Serial.println("Ingrese el nuevo tiempo para el Motor 1 en milisegundos:");
     mensaje=("Ingrese el nuevo tiempo para el Motor 1 en milisegundos: \n");
     pCharacteristicTX->setValue(mensaje.c_str());
     pCharacteristicTX->notify();
     break;
  case'2': 
    Serial.println("Ingrese el nuevo tiempo para el Motor 2 en milisegundos:");
    mensaje=("Ingrese el nuevo tiempo para el Motor 2 en milisegundos: \n");
    pCharacteristicTX->setValue(mensaje.c_str());
    pCharacteristicTX->notify();
    break;
  case'3': 
    Serial.println("Ingrese el nuevo tiempo para el Motor 3 en milisegundos:");
     mensaje=("Ingrese el nuevo tiempo para el Motor 3 en milisegundos: \n");
     pCharacteristicTX->setValue(mensaje.c_str());
     pCharacteristicTX->notify();
     break;
  case'4': 
    Serial.println("Ingrese el nuevo tiempo para el Motor 4 en milisegundos:");
    mensaje=("Ingrese el nuevo tiempo para el Motor 4 en milisegundos: \n");
    pCharacteristicTX->setValue(mensaje.c_str());
    pCharacteristicTX->notify(); 
    break;
  default: 
    Serial.println("Opción inválida.");
    mensaje=("Opción inválida. \n");
     pCharacteristicTX->setValue(mensaje.c_str());
     pCharacteristicTX->notify();
    return;
    break;
 }

  while (pCharacteristicRX->getValue().indexOf('\n') == -1)  {
    delay(100);
  }
   receivedData = String(pCharacteristicRX->getValue().c_str());
  // Limpia el valor de la característica RX
  pCharacteristicRX->setValue("");

  while (pCharacteristicRX->getValue().length() == 0) {
    delay(100);
   }

  String nuevoTiempoStr = pCharacteristicRX->getValue().c_str();
  nuevoTiempo = nuevoTiempoStr.toInt();

  if (opcion == '1') {
    tiempoMotor1 = nuevoTiempo;
    Serial.println("Tiempo del Motor 1 actualizado.");
    mensaje=("Tiempo del Motor 1 actualizado. \n");
    pCharacteristicTX->setValue(mensaje.c_str());
    pCharacteristicTX->notify();
  }if (opcion == '2') {
    tiempoMotor2 = nuevoTiempo;
    Serial.println("Tiempo del Motor 2 actualizado.");
    mensaje=("Tiempo del Motor 2 actualizado. \n");
    pCharacteristicTX->setValue(mensaje.c_str());
    pCharacteristicTX->notify();
  }if (opcion == '3') {
    tiempoMotor3 = nuevoTiempo;
    Serial.println("Tiempo del Motor 3 actualizado.");
    mensaje=("Tiempo del Motor 3 actualizado. \n");
    pCharacteristicTX->setValue(mensaje.c_str());
    pCharacteristicTX->notify();
  }if (opcion == '4') {
    tiempoMotor4 = nuevoTiempo;
    Serial.println("Tiempo del Motor 4 actualizado.");
    mensaje=("Tiempo del Motor 4 actualizado. \n");
    pCharacteristicTX->setValue(mensaje.c_str());
    pCharacteristicTX->notify();
  }

  SaveTime();
  while (pCharacteristicRX->getValue().indexOf('\n') == -1)  {
    delay(100);
  }
   receivedData = String(pCharacteristicRX->getValue().c_str());
  // Limpia el valor de la característica RX
  pCharacteristicRX->setValue("");

  data="";
  previousdata = data;
  mensaje=mensajeDef;
  
}

void SaveTime(){ 
  EEPROM.put(TIEMPOS_MOTOR1_ADDR, tiempoMotor1);
  EEPROM.put(TIEMPOS_MOTOR2_ADDR, tiempoMotor2);
  EEPROM.put(TIEMPOS_MOTOR3_ADDR, tiempoMotor3);
  EEPROM.put(TIEMPOS_MOTOR4_ADDR, tiempoMotor4);
  EEPROM.commit();
}

void ReadTime(){

  EEPROM.get(TIEMPOS_MOTOR1_ADDR, tiempoMotor1);
  EEPROM.get(TIEMPOS_MOTOR2_ADDR, tiempoMotor2);
  EEPROM.get(TIEMPOS_MOTOR3_ADDR, tiempoMotor3);
  EEPROM.get(TIEMPOS_MOTOR4_ADDR, tiempoMotor4);
  ESTADO1=EEPROM.read(500);
  ESTADO2=EEPROM.read(501);
}

  
    
  

boolean comparaUID(byte lectura[],byte usuario[]) // funcion comparaUID
{
  for (byte i=0; i < mfrc522.uid.size; i++){    // bucle recorre de a un byte por vez el UID
  if(lectura[i] != usuario[i])        // si byte de UID leido es distinto a usuario
    return(false);          // retorna falso
  }
  return(true);           // si los 4 bytes coinciden retorna verdadero
}           

