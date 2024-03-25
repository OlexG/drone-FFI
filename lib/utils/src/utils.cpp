
#include <Arduino.h>
#include "utils.h"

void generateRandomKey(byte *keyArray, int keySize)
{
    for (int i = 0; i < keySize; i++)
    {
        keyArray[i] = random(0, 256); // Replace this with ESP32 true random later
    }
}