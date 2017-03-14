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

void doTracking()
{
	uint8_t complete_values[NUM_PA];
	for(uint8_t i=0;i<NUM_PA;i++)
	{
		parray[i]->request();
		complete_values[i]=0;
	}

	uint8_t num_complete;
	while(num_complete<NUM_PA)
	{
		num_complete=0;
		for(uint8_t i=0;i<NUM_PA;i++)
		{
			//TODO: volatile this variable and protect it..more specificaly 
			complete_values[i]=(uint8_t)(parray[i]->check_status() != PixArt::COMPLETE);
		}
		noInterrupts();
		for(uint8_t i=0;i<NUM_PA;i++)
		{
			num_complete+=complete_values[i];
		}
		Interrupts();
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
}

elapsedMicros capture_frametimer;
uint32_t current_framerate_delay_us=8333; //120hz

void loop()
{
	if(capture_framerate >= current_framerate_delay_us)
	{	
		doTracking();
		capture_frametimer -= current_framerate_delay_us;
	}
	//no more work to do till next boundary...this doesn't exactly save power because it doesn't sleep properly...but it is correct and who cares about saving power for this thing.
}