#include "Arduino.h"
#include "EEPROM.h"
#include "EtherCard.h"
#include "Vrekrer_scpi_parser.h"

const int eeprom_eth_data_start = 0;
const static byte dns[] = {0, 0, 0, 0};
const static byte mask[] = {255, 255, 255, 0};
byte mac[6] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };
byte ip[4] = {192, 168, 10, 7};
byte gw[4] = {192, 168, 10, 1};
byte Ethernet::buffer[128];

const byte csPin = 2; //ChipSelect Pin

SCPI_Parser my_instrument;
boolean fromSerial = true;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  my_instrument.RegisterCommand("*IDN?", &Identify);
  my_instrument.SetCommandTreeBase("SYSTem:COMMunicate:LAN"); 
    my_instrument.RegisterCommand(":ADDRess", &SetIP);
    my_instrument.RegisterCommand(":ADDRess?", &GetIP);
    my_instrument.RegisterCommand(":DGATeway", &SetGW);
    my_instrument.RegisterCommand(":DGATeway?", &GetGW);
    my_instrument.RegisterCommand(":MAC", &SetMAC);
    my_instrument.RegisterCommand(":MAC?", &GetMAC);

  int eeprom_address = eeprom_eth_data_start;
  if (EEPROM.read(eeprom_address) == 'V') { //Already initialized
    //Serial.println("EEPROM OK");
    ++eeprom_address;
    EEPROM.get(eeprom_address, mac);
    eeprom_address += sizeof(mac);
    EEPROM.get(eeprom_address, ip);
    eeprom_address += sizeof(ip);
    EEPROM.get(eeprom_address, gw);
  } else { //Write default values to EEPROM
    EEPROM.write(eeprom_address, 'V');
    ++eeprom_address;
    EEPROM.put(eeprom_address, mac);
    eeprom_address += sizeof(mac);
    EEPROM.put(eeprom_address, ip);
    eeprom_address += sizeof(ip);
    EEPROM.put(eeprom_address, gw);
  }

  ether.hisport = 5025; //SCPI PORT

  boolean eth_enabled = false;
  if (ether.begin(sizeof Ethernet::buffer, mac, csPin))
    eth_enabled = ether.staticSetup(ip, gw, dns, mask);
  if (!eth_enabled) ether.powerDown();
}

void loop() {
  fromSerial = true;
  my_instrument.ProcessInput(Serial, "\n");

  if ( ether.isLinkUp() ) {
      fromSerial = false;
      my_instrument.Execute(GetEthMsg(), Serial);
  }
}

/* SCPI FUNCTIONS */

void Identify(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  char IDN[] = "Vrekrer,SCPI Ethernet Instrument,#00,v0.4.2\n";
  PrintToInterface(IDN);
}

void SetIP(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  SaveIP(parameters.First(), eeprom_eth_data_start + 7 );
}

void GetIP(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  char ip_str[16];
  IpToString(ether.myip, ip_str);
  PrintToInterface( ip_str );
}

void SetGW(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  SaveIP(parameters.First(), eeprom_eth_data_start + 11 );
}

void GetGW(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  char ip_str[16];
  IpToString(ether.gwip, ip_str);
  PrintToInterface( ip_str );
}

void SetMAC(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  int this_mac[6];
  int mac_ok = 0;
  if (parameters.Size() == 6) {
    for (uint8_t i = 0; i < 6; i++) {
      mac_ok += sscanf(parameters[i], "%x", &this_mac[i]);
    }
  }
  int eeprom_address = eeprom_eth_data_start + 1;  
  if (mac_ok == 6) {
    for (uint8_t i = 0; i < 6; i++) {
      EEPROM.write(eeprom_address, byte(this_mac[i]));
      ++eeprom_address;
    }
  }
}

void GetMAC(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  char mac_str[ ] = "0x##, 0x##, 0x##, 0x##, 0x##, 0x##\n";
  for (int i = 0; i < 6; ++i) {
    char u = ether.mymac[i] / 16;
    char l = ether.mymac[i] % 16;
    mac_str[6 * i + 2] = u < 10 ? u + '0' : u + 'A' - 10;
    mac_str[6 * i + 3] = l < 10 ? l + '0' : l + 'A' - 10;
  }
  PrintToInterface(mac_str);
}


/* HELPER FUNCTIONS */

char* GetEthMsg() {
  word pos = ether.packetLoop(ether.packetReceive());
  if (pos) {
    ether.httpServerReplyAck();
    char* msg = Ethernet::buffer + pos;
    msg = strtok(msg, "\n"); //Remove termination char
    return msg;
  } else {
    return NULL;
  }
}

void WriteEthMsg(char* msg) {
  delayMicroseconds(20); //Delay needed for next package
  BufferFiller bfill = ether.tcpOffset();
  bfill.emit_raw(msg, strlen(msg));
  ether.httpServerReply_with_flags(bfill.position(), TCP_FLAGS_ACK_V | TCP_FLAGS_PUSH_V);
}

void PrintToInterface(char* print_str) {
  if (fromSerial)
    Serial.write(print_str, strlen(print_str));
  else
    WriteEthMsg(print_str);
}

void SaveIP(char* ip_str, int eeprom_address) {
  int this_ip[4];
  int ipOk = sscanf(ip_str, "%d.%d.%d.%d", &this_ip[0], &this_ip[1], &this_ip[2], &this_ip[3]);
  if (ipOk == 4) {
    for (uint8_t i = 0; i < 4; i++) {
      EEPROM.write(eeprom_address, byte(this_ip[i]));
      ++eeprom_address;
    }
  }
}

void IpToString(byte* IP, char* ip_str) {
  sprintf(ip_str, "%d.%d.%d.%d\n", IP[0], IP[1], IP[2], IP[3]);
  return ip_str;
}

/* Example code in python 

import visa
rm = visa.ResourceManager('@py')
# use rm.list_resources() to get the resource names

SerialInstrument = rm.get_instrument('ASRL/dev/ttyACM0::INSTR')
SerialInstrument.write_termination = '\n'
SerialInstrument.read_termination = '\n'
# Change mac, ip and default gateway
SerialInstrument.write('SYST:COMM:LAN:MAC 0x74, 0x69, 0x69, 0x2D, 0x30, 0x32')
SerialInstrument.write('SYST:COMM:LAN:ADDR 192.168.0.177')
SerialInstrument.write('SYST:COMM:LAN:DGAT 192.168.0.1')

# **Reset the instrument
# And now connect by ethernet with the new ip.
EthInstrument = rm.get_instrument('TCPIP0::192.168.0.177::5025::SOCKET')
EthInstrument.write_termination = '\n'
EthInstrument.read_termination = '\n'
#Test connection reading mac
print( EthInstrument.query('SYST:COMM:LAN:MAC?') )

*/
