// Wii Remote IR sensor  test sample code  by kako http://www.kako.com
// modified output for Wii-BlobTrack program by RobotFreak http://www.letsmakerobots.com/user/1433
// modified for http://DFRobot.com by Lumi, Jan. 2014

#include "PixArt.h"

static const int ledPin=13;
bool ledState;

#ifdef PA_TEENSY3_WIRE

PixArt pa0(Wire);
PixArt pa1(Wire1);

PixArt *parray[2]={&pa0,&pa1};
#else
PixArt pa(Wire);
PixArt *parray[1]={&pa};
#endif

static const uint8_t NUM_PA=sizeof(parray)/sizeof(PixArt*);

void psetup(PixArt& pa,const PixArt::Settings& s)
{
	#ifdef PA_TEENSY3_WIRE
		//Wire.begin(I2C_MASTER,0xB0,I2C_PINS_3_4,I2C_PULLUP_EXT,I2C_RATE_100);
		
		//Wire.pinConfigure(I2C_PINS_18_19, I2C_PULLUP_EXT); 
		//Wire.begin(I2C_MASTER,0xB0,I2C_PINS_18_19,I2C_PULLUP_EXT,I2C_RATE_100,I2C_OP_MODE_ISR);
		//Wire1.begin();
		pa.wire().begin();
		//pa.wire().pinConfigure(I2C_PINS_18_19, I2C_PULLUP_EXT);
		pa.wire().setClock(2400000);  //must be more than 100000
		//pa.wire().begin(I2C_MASTER,0xB0,WireType::getDefaultPins(pa.wire().bus),I2C_PULLUP_EXT,I2C_RATE_100,I2C_OP_MODE_ISR
	#else
		pa.wire().begin();
	#endif

	pa.reset(s);
}

void setup()
{	
	ledState=true;
	Serial.begin(19200);
	pinMode(ledPin, OUTPUT);      // Set the LED pin as output
	

	PixArt::Settings s;
	s.mode=PixArt::SIMPLE;
	s.gain=10;

	for(uint8_t i=0;i<NUM_PA;i++)
	{
		psetup(*parray[i],s);
	}
	// IR sensor initialize


}

void loop()
{
	ledState = !ledState;
	if (ledState) { digitalWrite(ledPin,HIGH); } else { digitalWrite(ledPin,LOW); }

	for(uint8_t i=0;i<NUM_PA;i++)
	{
		parray[i]->request();
	}

	for(uint8_t num_complete=0;num_complete<NUM_PA;)
	{
		for(uint8_t i=0;i<NUM_PA;i++)
		{
			num_complete+=(uint8_t)(parray[i]->check_status() != PixArt::COMPLETE);
		}
	}
	for(uint8_t i=0;i<NUM_PA;i++)
	{
		TrackingPacket tp=parray[i]->read();
		Serial.print("CAM ");
		Serial.print(i,DEC);
		for(int pi=0;pi<4;pi++)
		{
			Serial.print("(");
			Serial.print((int)tp.points[pi].x,DEC);
			Serial.print(",");
			Serial.print((int)tp.points[pi].y,DEC);
			Serial.print(")");
			//Serial.print(tp.points[i].intensity);
		
		}
		Serial.print("\n");
	}
	delay(10);
}
