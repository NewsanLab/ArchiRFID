#ifndef RFIDREAD_H
#define RFIDREAD_H

#include <Arduino.h>

#define DELAYVAL 384   
#define TIMEOUT 5000 

class RFIDDecoder {
private:
    int demodOut; // GP18 - Archie
    byte tagData[5]; // Contiene los números de identificación de la etiqueta

public:
    // Constructor
    RFIDDecoder(int demodPin);

    // Métodos
    bool decodeTag(unsigned char *buf);
    bool compareTagData(byte *tagData1, byte *tagData2);
    void transferToBuffer(byte *tagData, byte *tagDataBuffer);
    bool scanForTag(byte *tagData);
};

#endif 
