#ifndef PIXART_H
#define PIXART_H


#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__MK66FX1M0__))
#define PA_TEENSY3_WIRE
#include<i2c_t3.h>
typedef i2c_t3 WireType;

#else

#include<Wire.h>
#define PA_ARDUINO_WIRE
typedef TwoWire WireType;
#endif

#include "TrackingProtocol.h"

class PixArt
{
public:
	//only simple mode works 
	enum Mode {BASIC=1,EXTENDED=3,FULL=5,OTHER=0x33,SIMPLE}; //is OTHER extended plus a 1-byte dontcare prefix?
	enum Status {IDLE=1,WAITING,COMPLETE};
private:
	WireType& w;
	static const uint8_t IRsensorAddress = 0xB0;
	static const uint8_t IRslaveAddress = IRsensorAddress >> 1;   // This results in 0x21 as the address to pass to TWI
	
	Mode mode;
	volatile Status status;
	uint8_t modesize() const
	{
		switch(mode)
		{
		case BASIC:
			return 10;
		case EXTENDED:
			return 12;
		case FULL:
			return 36;
		case OTHER:
		case SIMPLE:
			return 16;
		};
		return 1;
	}
	void write_2byte(uint8_t d1,uint8_t d2);
public:
	struct Settings
	{
	public:
		Mode mode;
		uint8_t gain;  //The byte labeled GAIN controls the camera gain. Lower values result in higher sensitivity. Numerical gain is proportional to 1/2^(n/16) for n<0x40, but decreases more linearly after that. Translation: for small values, dropping this value by 0x10 doubles sensitivity. Usable values are from about 15 to 254. Below that results in noise and streaking becomes significant and the camera doesn't function for a value of 255...gain
			//instead we're gonna do gain as gain 254-x ...default value for standard wiimote is 64, so 254-190...but we can look at blobsize average and autoadjust?
		uint8_t max_blobsize; //0x62 to 0xc8.
		uint8_t min_blobsize; //3,4 or 5,or 0
		enum Unknown {DEFAULTSENSITIVITY=2,MAXSENSITIVITY=7,OFF=0};
		Unknown sensitivity_unknown;
		
		Settings(Mode m=FULL,uint8_t g=190,uint8_t mx_bs=0x90,uint8_t mn_bs=3,Unknown su=DEFAULTSENSITIVITY):
			mode(m),gain(254-g),max_blobsize(mx_bs),min_blobsize(mn_bs),sensitivity_unknown(su)
		{}
	};
	
	PixArt(WireType& tw):w(tw),mode(BASIC),status(IDLE)
	{}
	void reset(const Settings s=Settings());

	void request();
	Status check_status();
	TrackingPacket read();
	
	WireType& wire() { return w; }

private:
	void readbuffer(uint8_t* buf,uint8_t count);
	TrackingPacket getBasic();

	bool getSubExtendedMode(Point& point,const uint8_t* b);
	TrackingPacket getExtended();
	TrackingPacket getFull();
	TrackingPacket getOther();
};

#endif
