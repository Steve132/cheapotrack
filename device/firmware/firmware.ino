// Wii Remote IR sensor  test sample code  by kako http://www.kako.com
// modified output for Wii-BlobTrack program by RobotFreak http://www.letsmakerobots.com/user/1433
// modified for http://DFRobot.com by Lumi, Jan. 2014

#include "PixArt.h"
#include "ProtocolServer.h"

static const int ledPin=13;
bool ledState;


PixArt pa0(Wire);
PixArt pa1(Wire1);

PixArt *parray[2]={&pa0,&pa1};

ct::ProtocolServer server_host;

static const uint8_t NUM_PA=sizeof(parray)/sizeof(PixArt*);

void psetup(PixArt& pa,const PixArt::Settings& s)
{
	pa.wire().begin();
	pa.wire().setClock(2400000);  //must be more than 100000
	
	pa.reset(s);
}

void setup()
{	
	ledState=true;
	Serial.begin(ct::CT_PROTOCOL_BAUD);
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
elapsedMicros capture_frametimer;
void doTracking()
{
	uint8_t complete_values[NUM_PA];
	uint8_t cmask=server_host.cam_mask;
	uint8_t enabled_count=0;
	for(uint8_t i=0;i<NUM_PA;i++)
	{
		if(!((cmask >> i) & 0x1)) continue;
		parray[i]->request();
		complete_values[i]=0;
		enabled_count++;
	}

	uint8_t num_complete;
	uint16_t group_delays[NUM_PA];
	while(num_complete<enabled_count)
	{
		num_complete=0;
		for(uint8_t i=0;i<NUM_PA;i++)
		{
			if(!((cmask >> i) & 0x1)) continue;
			//TODO: volatile this variable and protect it..more specificaly 
			if(parray[i]->check_status() != PixArt::COMPLETE)
			{
				complete_values[i]=1;
			}
			else
			{
				group_delays[i]=static_cast<uint32_t>(capture_frametimer) & 0xFFFF; //#TODO, not less than 15FPS
			}
		}
		noInterrupts();
		for(uint8_t i=0;i<NUM_PA;i++)
		{
			if(!((cmask >> i) & 0x1)) continue;
			num_complete+=complete_values[i];
		}
		interrupts();
	}

	PointGroup pgarray[4];

	for(uint8_t i=0;i<NUM_PA;i++)
	{
		if(!((cmask >> i) & 0x1)) continue;
		pgarray[i]=parray[i]->read();
	}
	server_host.onFrameCapture(pgarray,group_delays,NUM_PA);
}
bool last_tracking=false;
void loop()
{
	if(!Serial.dtr())
	{
		return;
	}
	if(Serial.available() > 0)
	{
		server_host.onCommandReady();
	}

	if(server_host.tracking_active ^ last_tracking)
	{
		digitalWrite(ledPin, last_tracking ? HIGH : LOW);
	}
	last_tracking=server_host.tracking_active;
	if(!server_host.tracking_active)
	{
		return;
	}
	if(capture_frametimer >= server_host.framerate_delay_us)
	{	
		doTracking();//this sends the result via server host...
		capture_frametimer -= server_host.framerate_delay_us; //TODO, cannot go lower than 15 fps
	}
	//no more work to do till next boundary...this doesn't exactly save power because it doesn't sleep properly...but it is correct and who cares about saving power for this thing.
}
