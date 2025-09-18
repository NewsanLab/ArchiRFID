#include "luces.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void pintar_aleatorio(uint8_t dibujo[8][8], uint32_t color, uint32_t colorFondo) {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      int pixelNumber = i * 8 + j; // Calcula la posición del LED en la matriz
      if (dibujo[i][j] == 1) {
        pixels.setPixelColor(pixelNumber, color);        // Establece el color blanco p
      } else {
       // pixels.setPixelColor(pixelNumber, colorFondo);   // Establece el color rojo 
      }
    }
  }
  pixels.show();  // Actualiza los LEDs para mostrar los cambios
}

void resetPixels() {
  // Apaga todos los LEDs estableciendo su color a negro
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, 0);  // 0 es el color negro/apagado
  }
  pixels.show();  // Actualiza los LEDs para mostrar los cambios
}

void setupNeoPixel() {
  pixels.begin();            // Inicializa la biblioteca NeoPixel
  pixels.setBrightness(25);  // Configura el brillo (0 a 255)
}

void loopNeoPixel() {
  // Estado "OK": Mostrar el patrón con blanco 
  uint8_t dibujo_ok[8][8] = {
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1}
  };

  // Resetear los LEDs antes de pintar el nuevo estado
  resetPixels();
  pintar_aleatorio(dibujo_ok, COLOR_TEXT, COLOR_BG);  // Estado OK: Blanco y Rojo
  delay(500);  // Espera dos segundos [nota: se pone este tiempo para no interrupir con la decodificación]

  // Estado de alerta: Todos los LEDs en rojo
  uint8_t dibujo_alerta[8][8] = {
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1}
  };

  // Resetear los LEDs antes de pintar el nuevo estado, asi no acopla al ok status blanco
  resetPixels();
  pintar_aleatorio(dibujo_alerta, COLOR_ALL_RED, COLOR_ALL_RED);  // Estado de alerta: Todos en Rojo
}
