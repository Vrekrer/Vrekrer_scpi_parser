/**
 * Arduino & Ethernet Shield V2 & DHT-22 Sensor @Vrekrer & Metaln00b
 * Build with Arduino IDE v1.8.13
 * Install:
 * - Ethernet.zip
 * - Adafruit_Unified_Sensor.zip
 * - DHT_sensor_library.zip
 * - Vrekrer_scpi_parser.zip
 */

#include "Arduino.h"
#include "EEPROM.h"
#include <Ethernet.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "Vrekrer_scpi_parser.h"

/* DHT 22 */
#define DHTPIN      5
#define DHTTYPE     DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

/* Ethernet Shield W5500 */
const int eeprom_eth_data_start = 0;
byte mac[6] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };
byte ip[4] = {192, 168, 10, 7};
byte gw[4] = {192, 168, 10, 1};
byte mask[4] = {255, 255, 255, 0};
byte dns[4] = {192, 168, 10, 1};
EthernetServer server = EthernetServer(5025); // SCPI PORT

/* SCPI Parser */
SCPI_Parser my_instrument;

sensors_event_t event_temp;
sensors_event_t event_humi;

void setup() {
    Serial.begin(115200);

    LoadEthernetData();
    LoadEthernet();

    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    dht.humidity().getSensor(&sensor);
    delayMS = sensor.min_delay / 1000;

    my_instrument.RegisterCommand(strdup("*IDN?"), &Identify);
    my_instrument.RegisterCommand(strdup("DHT?"), &GetDHT);
    my_instrument.SetCommandTreeBase(strdup("SYSTem:COMMunicate:NETWork"));
        my_instrument.RegisterCommand(strdup(":MAC"), &SetMAC);
        my_instrument.RegisterCommand(strdup(":MAC?"), &GetMAC);
        my_instrument.RegisterCommand(strdup(":IP"), &SetIP);
        my_instrument.RegisterCommand(strdup(":IP?"), &GetIP);
        my_instrument.RegisterCommand(strdup(":GATeway"), &SetGW);
        my_instrument.RegisterCommand(strdup(":GATeway?"), &GetGW);
        my_instrument.RegisterCommand(strdup(":SUBNet"), &SetMASK);
        my_instrument.RegisterCommand(strdup(":SUBNet?"), &GetMASK);
        my_instrument.RegisterCommand(strdup(":DNS"), &SetDNS);
        my_instrument.RegisterCommand(strdup(":DNS?"), &GetDNS);


    while (!Serial);
    my_instrument.PrintDebugInfo();
}


void loop() {
    char *token = strdup("\r");
    EthernetClient client;
    client = server.available();
    if (client)
    {
        my_instrument.ProcessInput(client, token);
    }
    free(token);

    dht.temperature().getEvent(&event_temp);
    dht.humidity().getEvent(&event_humi);
}


/* SCPI FUNCTIONS */

void Identify(SCPI_C commands __attribute__((unused)), SCPI_P parameters __attribute__((unused)), Stream &interface) {
    interface.println("Vrekrer & Metaln00b, Ethernet Test, P/N: none, S/N: none, HR: 1.0, SR: 1.0.0");
}


void SetMAC(SCPI_C commands __attribute__((unused)), SCPI_P parameters, Stream &interface __attribute__((unused))) {
    int this_mac[6];
    int macOk = sscanf(parameters.First(), "%x:%x:%x:%x:%x:%x", &this_mac[0], &this_mac[1], &this_mac[2], &this_mac[3], &this_mac[4], &this_mac[5]);

    int eeprom_address = eeprom_eth_data_start + 1;

    if (macOk == 6)
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            EEPROM.write(eeprom_address, byte(this_mac[i]));
            ++eeprom_address;
        }
        Serial.println("MAC SAVED");
        ReloadEthernet();
    }
}


void GetMAC(SCPI_C commands __attribute__((unused)), SCPI_P parameters __attribute__((unused)), Stream &interface) {
    byte macBuffer[6];
    Ethernet.MACAddress(macBuffer);

    for (byte octet = 0; octet < 6; octet++)
    {
        interface.print(macBuffer[octet], HEX);

        if (octet < 5)
        {
            interface.print(":");
        }
    }
    interface.println("");
}


void SetIP(SCPI_C commands __attribute__((unused)), SCPI_P parameters, Stream &interface __attribute__((unused))) {
    SaveIP(parameters.First(), eeprom_eth_data_start + 7 );
}


void GetIP(SCPI_C commands __attribute__((unused)), SCPI_P parameters __attribute__((unused)), Stream &interface) {
    interface.println(Ethernet.localIP());
}


void SetGW(SCPI_C commands __attribute__((unused)), SCPI_P parameters, Stream &interface __attribute__((unused))) {
    SaveIP(parameters.First(), eeprom_eth_data_start + 11 );
}


void GetGW(SCPI_C commands __attribute__((unused)), SCPI_P parameters __attribute__((unused)), Stream &interface) {
    interface.println(Ethernet.gatewayIP());
}


void SetMASK(SCPI_C commands __attribute__((unused)), SCPI_P parameters, Stream &interface __attribute__((unused))) {
    SaveIP(parameters.First(), eeprom_eth_data_start + 15 );
}


void GetMASK(SCPI_C commands __attribute__((unused)), SCPI_P parameters __attribute__((unused)), Stream &interface) {
    interface.println(Ethernet.subnetMask());
}


void SetDNS(SCPI_C commands __attribute__((unused)), SCPI_P parameters, Stream &interface __attribute__((unused))) {
    SaveIP(parameters.First(), eeprom_eth_data_start + 19 );
}


void GetDNS(SCPI_C commands __attribute__((unused)), SCPI_P parameters __attribute__((unused)), Stream &interface) {
    interface.println(Ethernet.dnsServerIP());
}


void GetDHT(SCPI_C commands __attribute__((unused)), SCPI_P parameters __attribute__((unused)), Stream &interface) {
    if (isnan(event_humi.relative_humidity) || isnan(event_temp.temperature))
    {
        interface.println("Error reading temperature and humidity!");
    }
    else
    {
        interface.print(event_temp.temperature);
        interface.print(F(", "));
        interface.println(event_humi.relative_humidity);
    }
}


/* HELPER FUNCTIONS */

void SaveIP(char* ip_str, int eeprom_address) {
    int this_ip[4];
    int ipOk = sscanf(ip_str, "%d.%d.%d.%d", &this_ip[0], &this_ip[1], &this_ip[2], &this_ip[3]);
    if (ipOk == 4)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            EEPROM.write(eeprom_address, byte(this_ip[i]));
            ++eeprom_address;
        }
        Serial.println("IP/GATEWAY/MASK/DNS SAVED");
        ReloadEthernet();
    }
}


void LoadEthernetData() {
    int eeprom_address = eeprom_eth_data_start;
    if (EEPROM.read(eeprom_address) == 'V') // Already initialized
    {
        Serial.println("EEPROM ALREADY INITIALIZED");

        ++eeprom_address;
        EEPROM.get(eeprom_address, mac);
        eeprom_address += sizeof(mac);
        Serial.print("MAC:");
        for (size_t i = 0; i < sizeof(mac); i++)
        {
            Serial.print(mac[i],HEX);
            if (i != sizeof(mac)-1)
            {
                Serial.print(":");
            }
        }
        Serial.println("");

        EEPROM.get(eeprom_address, ip);
        eeprom_address += sizeof(ip);
        Serial.print("IP:");
        for (size_t i = 0; i < sizeof(ip); i++)
        {
            Serial.print(ip[i]);
            if (i != sizeof(ip)-1)
            {
                Serial.print(".");
            }
        }
        Serial.println("");

        EEPROM.get(eeprom_address, gw);
        eeprom_address += sizeof(gw);
        Serial.print("GATEWAY:");
        for (size_t i = 0; i < sizeof(gw); i++)
        {
            Serial.print(gw[i]);
            if (i != sizeof(gw)-1)
            {
                Serial.print(".");
            }
        }
        Serial.println("");

        EEPROM.get(eeprom_address, mask);
        eeprom_address += sizeof(mask);
        Serial.print("MASK:");
        for (size_t i = 0; i < sizeof(mask); i++)
        {
            Serial.print(mask[i]);
            if (i != sizeof(mask)-1)
            {
                Serial.print(".");
            }
        }
        Serial.println("");

        EEPROM.get(eeprom_address, dns);
        Serial.print("DNS:");
        for (size_t i = 0; i < sizeof(dns); i++)
        {
            Serial.print(dns[i]);
            if (i != sizeof(dns)-1)
            {
                Serial.print(".");
            }
        }
        Serial.println("");
    }
    else // Write default values to EEPROM
    {
        Serial.println("WRITE EEPROM");
        EEPROM.write(eeprom_address, 'V');
        ++eeprom_address;
        EEPROM.put(eeprom_address, mac);
        eeprom_address += sizeof(mac);
        EEPROM.put(eeprom_address, ip);
        eeprom_address += sizeof(ip);
        EEPROM.put(eeprom_address, gw);
        eeprom_address += sizeof(gw);
        EEPROM.put(eeprom_address, mask);
        eeprom_address += sizeof(mask);
        EEPROM.put(eeprom_address, dns);
    }
}


void LoadEthernet() {
    Ethernet.begin(mac, ip, dns, gw, mask);
    server.begin();
}


void ReloadEthernet() {
    LoadEthernetData();
    Ethernet.begin(mac, ip, dns, gw, mask);
    server.begin();
}
