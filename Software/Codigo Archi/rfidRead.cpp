// rfidRead.cpp
#include "rfidRead.h"

// Constructor
RFIDDecoder::RFIDDecoder(int demodPin) {
    demodOut = demodPin;
    pinMode(demodOut, INPUT); // Configura el pin de demodulación como entrada
}

// Decodificación de Manchester
bool RFIDDecoder::decodeTag(unsigned char *buf) {
    unsigned char i = 0;
    unsigned long startMicros;
    unsigned long currentMicros;
    unsigned char timeOutFlag = 0;
    unsigned char row, col;
    unsigned char row_parity;
    unsigned char col_parity[5];
    unsigned char dat;
    unsigned char j;  

    while (true) {
        startMicros = micros();
        while (digitalRead(demodOut) == LOW) {
            currentMicros = micros();
            if (currentMicros - startMicros >= TIMEOUT) {
                return false;
            }
        }

        if (currentMicros - startMicros >= 600 * 1000) {
            return false;
        }

        delayMicroseconds(DELAYVAL);

        if (digitalRead(demodOut)) {
            for (i = 0; i < 8; i++) {
                startMicros = micros();
                while (digitalRead(demodOut) == HIGH) {
                    currentMicros = micros();
                    if (currentMicros - startMicros >= TIMEOUT) {
                        timeOutFlag = 1;
                        break;
                    }
                }

                if (timeOutFlag) {
                    return false;
                }

                delayMicroseconds(DELAYVAL);

                if (digitalRead(demodOut) == LOW) {
                   // Serial.println("hay un cero");
                    break;
                }
            }

            if (timeOutFlag) {
                return false;
            }

            if (i == 8) {
                timeOutFlag = 0;
                startMicros = micros();
                while (digitalRead(demodOut) == HIGH) {
                    currentMicros = micros();
                    if (currentMicros - startMicros >= TIMEOUT) {
                        timeOutFlag = 1;
                        break;
                    }
                }

                if (timeOutFlag) {
                    return false;
                }

                col_parity[0] = col_parity[1] = col_parity[2] = col_parity[3] = col_parity[4] = 0;

                for (row = 0; row < 11; row++) {
                    row_parity = 0;
                    j = row >> 1;

                    for (col = 0; col < 5; col++) {
                        delayMicroseconds(DELAYVAL);
                        dat = digitalRead(demodOut) ? 1 : 0;

                        if (col < 4 && row < 10) {
                            buf[j] <<= 1;
                            buf[j] |= dat;
                        }

                        row_parity += dat;
                        col_parity[col] += dat;

                        startMicros = micros();
                        while (digitalRead(demodOut) == dat) {
                            currentMicros = micros();
                            if (currentMicros - startMicros >= TIMEOUT) {
                                timeOutFlag = 1;
                                break;
                            }
                        }

                        if (timeOutFlag) {
                            break;
                        }
                    }

                    if (row < 10) {
                        if ((row_parity & 0x01) || timeOutFlag) {
                            timeOutFlag = 1;
                            break;
                        }
                    }
                }

                if (timeOutFlag || (col_parity[0] & 0x01) || (col_parity[1] & 0x01) || (col_parity[2] & 0x01) || (col_parity[3] & 0x01)) {
                      //timeOutFlag = 0;

                    return false;
                } else {
                    return true;
                }
            }
            return false;
        }
    }
}

// Comparar matrices de 2 bytes
bool RFIDDecoder::compareTagData(byte *tagData1, byte *tagData2) {
    for (int j = 0; j < 5; j++) {
        if (tagData1[j] != tagData2[j]) {
            return false;
        }
    }
    return true;
}

// Transferir una matriz de bytes a una matriz secundaria
void RFIDDecoder::transferToBuffer(byte *tagData, byte *tagDataBuffer) {
    for (int j = 0; j < 5; j++) {
        tagDataBuffer[j] = tagData[j];
    }
}

// Escanear para una etiqueta
bool RFIDDecoder::scanForTag(byte *tagData) {
    static byte tagDataBuffer[5];
    static int readCount = 0;
    bool verifyRead = false;
    bool tagCheck = false;

    tagCheck = decodeTag(tagData);
    if (tagCheck) {
        readCount++;
        if (readCount == 1) {
            transferToBuffer(tagData, tagDataBuffer);
        } else if (readCount == 2) {
            verifyRead = compareTagData(tagData, tagDataBuffer);
            if (verifyRead) {
                readCount = 0;
                return true;
            }
        }
    } else {
        return false;
    }
    return true;
}
