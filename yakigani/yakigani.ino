//=======================================
// Yakigani RTL8370 iROM burner
//
// Board: 'Generic STM32F1 Series', 'Bluepill F103C8', enable USB CDC Serial
// https://github.com/libc0607/yakigani
//
// Hardware: connect PIN_SMI_SDA and PIN_SMI_SCL to RTL8370 SDA(MDIO) & SCL(MDC), with pull-up resistors (~ 1k Ohm)
// Programming is triggered by (PIN_KEY == LOW)
//
// @libc0607 (libc0607@gmail.com)
//=======================================

#include <stdint.h>

//  Pin definitions
#define PIN_SMI_SDA PA10
#define PIN_SMI_SCL PA9
#define PIN_KEY PA8

// TFTP IP Address
#define IP_ADDR_SWITCH {192, 168, 1, 1}
#define IP_ADDR_SERVER {192, 168, 1, 2}

// irom: to-do
const uint8_t irom[9049] = {0};

void _smi_start(void)
{

  pinMode(PIN_SMI_SDA, OUTPUT);
  pinMode(PIN_SMI_SCL, OUTPUT);

  digitalWrite(PIN_SMI_SCL, LOW);
  digitalWrite(PIN_SMI_SDA, HIGH);
  delayMicroseconds(10);

  digitalWrite(PIN_SMI_SCL, HIGH);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SCL, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_SMI_SCL, HIGH);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SDA, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SCL, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SDA, HIGH);
}

void _smi_writeBit(uint16_t sig, uint32_t bitLen)
{
  for( ; bitLen > 0; bitLen--)
  {
    delayMicroseconds(2);

    if ( sig & (1<<(bitLen-1)) )
    digitalWrite(PIN_SMI_SDA, HIGH);
    else
    digitalWrite(PIN_SMI_SDA, LOW);
    delayMicroseconds(2);

    digitalWrite(PIN_SMI_SCL, HIGH);
    delayMicroseconds(2);
    digitalWrite(PIN_SMI_SCL, LOW);
  }
}

void _smi_readBit(uint32_t bitLen, uint32_t *rData)
{
  uint32_t u;

  pinMode(PIN_SMI_SDA, INPUT_PULLUP);
  
  for (*rData = 0; bitLen > 0; bitLen--) {
    delayMicroseconds(2);
    digitalWrite(PIN_SMI_SCL, HIGH);
    delayMicroseconds(2);
  
    u = (digitalRead(PIN_SMI_SDA) == HIGH)? 1: 0;
    digitalWrite(PIN_SMI_SCL, LOW);

    *rData |= (u << (bitLen - 1));
  }

  pinMode(PIN_SMI_SDA, OUTPUT);
}

void _smi_stop(void)
{

  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SDA, LOW);
  digitalWrite(PIN_SMI_SCL, HIGH);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SDA, HIGH);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SCL, HIGH);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SCL, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SCL, HIGH);

  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SCL, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SMI_SCL, HIGH);

  pinMode(PIN_SMI_SDA, OUTPUT);
  pinMode(PIN_SMI_SCL, OUTPUT);

}

int32_t smi_read(uint32_t mAddrs, uint32_t *rData)
{

  uint32_t rawData=0, ACK;
  uint8_t  con;
  uint32_t ret = 0;

  _smi_start();                                /* Start SMI */

  _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370 */

  _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

  _smi_writeBit(0x1, 1);                        /* 1: issue READ command */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for issuing READ command*/
  } while ((ACK != 0) && (con < 5));

  if (ACK != 0)
    ret = -1;

  _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
  } while ((ACK != 0) && (con < 5));

  if (ACK != 0)
    ret = -1;

  _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK by RTL8369 */
  } while ((ACK != 0) && (con < 5));
  if (ACK != 0)
    ret = -1;

  _smi_readBit(8, &rawData);                    /* Read DATA [7:0] */
  *rData = rawData&0xff;

  _smi_writeBit(0x00, 1);                        /* ACK by CPU */

  _smi_readBit(8, &rawData);                    /* Read DATA [15: 8] */

  _smi_writeBit(0x01, 1);                        /* ACK by CPU */
  *rData |= (rawData<<8);

  _smi_stop();

  if (ret) {
    SerialUSB.print("read reg ");SerialUSB.print(mAddrs);SerialUSB.println(" failed");
    return ret;
  }

  return ret;
}

int32_t smi_write(uint32_t mAddrs, uint32_t rData)
{

  int8_t con;
  uint32_t ACK;
  uint32_t ret = 0;

  _smi_start();                                /* Start SMI */

  _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370*/

  _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

  _smi_writeBit(0x0, 1);                        /* 0: issue WRITE command */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for issuing WRITE command*/
  } while ((ACK != 0) && (con < 5));
  if (ACK != 0)
    ret = -1;

  _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
  } while ((ACK != 0) && (con < 5));
  if (ACK != 0)
    ret = -1;

  _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[15:8] */
  } while ((ACK != 0) && (con < 5));
  if (ACK != 0)
    ret = -1;

  _smi_writeBit(rData&0xff, 8);                /* Write Data [7:0] out */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for writting data [7:0] */
  } while ((ACK != 0) && (con < 5));
  if (ACK != 0)
    ret = -1;

  _smi_writeBit(rData>>8, 8);                    /* Write Data [15:8] out */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                        /* ACK for writting data [15:8] */
  } while ((ACK != 0) && (con < 5));
  if (ACK != 0)
    ret = -1;

  _smi_stop();

  if (ret) {
    SerialUSB.print("write reg ");SerialUSB.print(mAddrs);SerialUSB.println(" failed");
    return ret;
  }
  return ret;
}

int32_t smi_setRegBit(uint32_t mAddrs, uint32_t shift, uint32_t b)
{
  uint32_t buf = 0;
  
  smi_read(mAddrs, &buf);
  
  return smi_write(mAddrs, ( (b)? buf|(1<<shift): buf&(~(1<<shift)) ));
}

void setup() {
  pinMode(PIN_SMI_SDA, OUTPUT);
  pinMode(PIN_SMI_SCL, OUTPUT);
  pinMode(PIN_KEY, INPUT_PULLUP);
  digitalWrite(PIN_SMI_SDA, HIGH);
  digitalWrite(PIN_SMI_SCL, HIGH);
  SerialUSB.begin(115200);
  delay(500);
}

void loop() {
  
  int i, key, ret;
  uint8_t ipaddr_client[] = IP_ADDR_SWITCH;
  uint8_t ipaddr_server[] = IP_ADDR_SERVER;
  uint32_t addr, dat;
  uint16_t buf;
  
  if (digitalRead(PIN_KEY) == LOW) {
  	delay(50);
  	if (digitalRead(PIN_KEY) == LOW) {

      SerialUSB.println("====================================");
      SerialUSB.println("Yakigani RTL8370 iROM Burner");
      SerialUSB.println("https://github.com/libc0607/yakigani");
      SerialUSB.println("====================================");

      
  		// 1. write tftp ip addr config to reg
  		smi_write(0x13A4, (ipaddr_server[0] << 8) | ipaddr_server[1] );
  		smi_write(0x13A5, (ipaddr_server[2] << 8) | ipaddr_server[3] );
  		smi_write(0x13A6, (ipaddr_client[0] << 8) | ipaddr_client[1] );
  		smi_write(0x13A7, (ipaddr_client[2] << 8) | ipaddr_client[3] );
      SerialUSB.println("write ipaddr finish");
      delay(100);
      
  		// 2. write irom	
      SerialUSB.print("write irom, size: "); SerialUSB.println(sizeof(irom));
      
      smi_setRegBit(0x1322, 4, 1);	// CHIP_RST_8051
  		smi_setRegBit(0x130c, 5, 1);	// ?
  		smi_setRegBit(0x1336, 1, 1);	// DW8051_STATUS_REG
  		smi_setRegBit(0x1322, 2, 0);	// CHIP_RST_CFG
      
      delay(100);
      
      for (i=0; i<sizeof(irom); i++) {
  			if (i == 0x2000) {
          smi_setRegBit(0x1336, 2, 1);  // DW8051_STATUS_REG  // change page?
          SerialUSB.println("change page");
  			}
        smi_write( ((i&0x1FFF) + 0xE000), irom[i]);
  		}
      
      delay(100);
      
      smi_setRegBit(0x1336, 2, 0);	// DW8051_STATUS_REG
  		smi_setRegBit(0x1336, 1, 0);	// DW8051_STATUS_REG
  		smi_setRegBit(0x1322, 4, 0);	// CHIP_RST_8051
      
      SerialUSB.println("finish");
      
  	}
  }
} // loop
