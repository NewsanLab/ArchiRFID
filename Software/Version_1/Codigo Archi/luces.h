#ifndef LUCES_H
#define LUCES_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIN            24       // GP24
#define NUMPIXELS      64       // Número total de LEDs en la matriz (8x8)

// Colores para los estados
#define COLOR_TEXT     0xFFFFFF // Blanco para el estado OK
#define COLOR_BG       0xFF0000 // Rojo para los LEDs apagados en estado OK
#define COLOR_ALL_RED  0xFF0000 // Rojo para el estado de alerta
// Inicializa los NeoPixels
extern Adafruit_NeoPixel pixels;

// Función para pintar la matriz
void pintar_aleatorio(uint8_t dibujo[8][8], uint32_t color, uint32_t colorFondo);


void setupNeoPixel();
void loopNeoPixel();

#endif 
