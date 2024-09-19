#include <Arduino.h>
#include "rfidRead.h"
#include "luces.h"

// Instanciar la clase con el pin de demodulación 
RFIDDecoder decoder(18);  // GP18

unsigned long lastScanTime = 0;  // Variable para almacenar el tiempo del último escaneo
const unsigned long waitTime = 2000;  // Tiempo de espera en milisegundos 

void setup() {
    Serial.begin(9600);
    Serial.println("Bienvenidos. Deslice su etiqueta RFID.");
    setupNeoPixel();
}

void loop() {
    // Si el tiempo actual menos el tiempo del último escaneo es mayor que el tiempo de espera
    if (millis() - lastScanTime > waitTime) {
        byte tagData[5];  // Buffer para almacenar los datos de la etiqueta

        // Buscar una tarjeta usando el método scanForTag de la clase RFIDDecoder
        if (decoder.scanForTag(tagData)) {
            // Construir el JSON
            String json = "{ \"id\": \"Traza_Archi\", \"tarjeta\": \"";
            String tarjetaHex = "";

            for (int n = 2; n < 5; n++) {
                // Convierte el byte a una cadena hexadecimal
                String hexValue = String(tagData[n], HEX);

                // Añade un cero a la izquierda si la longitud es 1
                if (hexValue.length() == 1) {
                    hexValue = "0" + hexValue;
                }

                tarjetaHex += hexValue;
            }

            loopNeoPixel();

            // Convertir la cadena a mayúsculas
            tarjetaHex.toUpperCase();
            json += tarjetaHex + "\" }";
            
            // Enviar el JSON por el puerto serial
            Serial.println(json);

            // Actualizar el tiempo del último escaneo
            lastScanTime = millis();
        }
    }
}
