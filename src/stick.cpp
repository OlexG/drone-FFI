
/*#include <Arduino.h>
#include <utils.h>
#include <uECC.h>

void debug(byte *keyArray, int keySize)
{
    for (int i = 0; i < keySize; i++)
    {
        if (keyArray[i] < 16)
        { 
            Serial.print("0");
        }
        Serial.print(keyArray[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

static int RNG(uint8_t *dest, unsigned size)
{
    while (size)
    {
        uint8_t val = 0;
        for (unsigned i = 0; i < 8; ++i)
        {
            int init = analogRead(0);
            int count = 0;
            while (analogRead(0) == init)
            {
                ++count;
            }

            if (count == 0)
            {
                val = (val << 1) | (init & 0x01);
            }
            else
            {
                val = (val << 1) | (count & 0x01);
            }
        }
        *dest = val;
        ++dest;
        --size;
    }
    return 1;
}

uint8_t privateKey[21];
uint8_t publicKey[40];

void setup()
{
    Serial.begin(9600);
    uECC_set_rng(&RNG);
    Serial.println("Generating key pair");
    const struct uECC_Curve_t *curve = uECC_secp160r1();
    uECC_make_key(publicKey, privateKey, curve);
}

void loop()
{
    Serial.println("Public key:");
    debug(publicKey, 40);
    Serial.println("Private key:");
    debug(privateKey, 21);
}*/
