#ifndef PIXART_H
#define PIXART_H


#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)) && defined(TEENSY_CORE)
#define PA_TEENSY3_WIRE
#include<i2c_t3.h>
typedef i2c_t3 WireType;

#else

#include<Wire.h>
#define PA_ARDUINO_WIRE
typedef TwoWire WireType;
#endif


class PixArt
{
public:
	enum Mode {BASIC=1,EXTENDED=3,FULL=5,OTHER=0x33}; //is OTHER extended plus a 1-byte dontcare prefix?
	enum Status {IDLE=1,WAITING,COMPLETE};
private:
	TwoWire& w;
	static const uint8_t IRsensorAddress = 0xB0;
	static const uint8_t IRslaveAddress = IRsensorAddress >> 1;   // This results in 0x21 as the address to pass to TWI
	
	void write_2byte(uint8_t d1,uint8_t d2)
	{
		w.beginTransmission(slaveAddress);
		w.write(d1); Wire.write(d2);
		w.endTransmission();
	}
	
	Mode mode;
	Status status;
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
			return 16;
		};
	}
public:
	struct Settings
	{
	private:
		Mode mode;
		uint8_t gain;  //The byte labeled GAIN controls the camera gain. Lower values result in higher sensitivity. Numerical gain is proportional to 1/2^(n/16) for n<0x40, but decreases more linearly after that. Translation: for small values, dropping this value by 0x10 doubles sensitivity. Usable values are from about 15 to 254. Below that results in noise and streaking becomes significant and the camera doesn't function for a value of 255...gain
			//instead we're gonna do gain as gain 254-x ...default value for standard wiimote is 64, so 254-190...but we can look at blobsize average and autoadjust?
		uint8_t max_blobsize; //0x62 to 0xc8.
		uint8_t min_blobsize; //3,4 or 5,or 0
		enum Unknown {DEFAULT=2,MAXSENSITIVITY=7,OFF=0} 
		Unknown sensitivity_unknown;
		
		Settings(Mode m=FULL,uint8_t g=190,uint8_t mx_bs=0x90,uint8_t mn_bs=3,Unknown su=DEFAULT):
			mode(m),gain(254-g),max_blobsize(mx_bs),min_blobsize(mn_bs),sensitivity_unkown(su)
		{}
	};
	
	PixArt(TwoWire& tw):w(tw),mode(BASIC),status(IDLE)
	{}
	void reset(const Settings s=Settings())
	{
		mode=s.mode;
		static const uint8_t DELAYAMOUNT=50;
		write_2byte(0x30,0x01);                           delay(DELAYAMOUNT);

	        write_2byte(0x00,(uint8_t)s.sensitivity_unknown); delay(DELAYAMOUNT);
		write_2byte(0x01,0x00);                           delay(DELAYAMOUNT);
		write_2byte(0x02,0x00);                           delay(DELAYAMOUNT);
		write_2byte(0x03,0x71);                           delay(DELAYAMOUNT);
		write_2byte(0x04,0x01);                           delay(DELAYAMOUNT);
		write_2byte(0x05,0x00);                           delay(DELAYAMOUNT);
		write_2byte(0x06,s.max_blobsize);                 delay(DELAYAMOUNT);
		write_2byte(0x07,0x00);                           delay(DELAYAMOUNT);
		write_2byte(0x08,s.gain);                         delay(DELAYAMOUNT);
		write_2byte(0x1a,s.gain-1);                       delay(DELAYAMOUNT);
		write_2byte(0x1b,s.min_blobsize);                 delay(DELAYAMOUNT);
		write_2byte(0x33,(uint8_t)s.mode);                delay(DELAYAMOUNT);

   		write_2byte(0x30,0x08);                           delay(DELAYAMOUNT);
		status=IDLE;
	}

	void request()
	{
		uint8_t sz=modesize();
		status=WAITING;
		#if defined(PA_ARDUINO_WIRE)
			w.requestFrom(IRslaveAddress,sz);
			status=COMPLETE;
		#elif defined(PA_TEENSY3_WIRE)
			w.sendRequest(IRslaveAddress,sz);
		#endif
	}
	Status check_status()
			
		#if defined(PA_TEENSY3_WIRE)
			status=w.done()==1 ? COMPLETE : WAITING;
		#elif defined(PA_ARDUINO_WIRE)
		#endif
		return status;
	}
	TrackingPacket read()
	{

		#if defined(PA_TEENSY3_WIRE)
			if(status==WAITING) w.finish();
		#elif defined(PA_ARDUINO_WIRE)
			while(check_status() == WAITING);
		#endif
		if(status==IDLE) 
		{
			return TrackingPacket();
		}

		switch(mode)
		{
		case BASIC:
			return getBasic();
		case EXTENDED:
			return getExtended();
		case FULL:
			return getFull();
		case OTHER:
			return getOther();
		};
	}
private:
	static void readbuffer(uint8_t* buf,uint8_t count)
	{
		for(register uint8_t i=0;i<count;)
		{
			if(w.available())
			{
				buf[i++]=w.read();
			}
		}
	}
	static inline uint8_t maskdown(const uint8_t* buf,uint8_t count)
	{
		uint8_t cm=0xFF;
		for(register uint8_t i=0;i<count;i++)
		{
			cm&=buf[i];
		}
		return cm;
	}
	TrackingPacket getBasic()
	{
		TrackingPacket r;
		uint8_t b[10];
		readbuffer(b,10);
		for(uint8_t s=0;s<4;s+=2)
		{
			uint8_t b3=b[3];
			const uint8_t* ib=b;
			for(uint8_t tb=0;tb<1;tb++)
			{
				uint8_t tbs=tb+s;
				uint16_t xp=b3 & 0x03;
				uint16_t yp=b3 & 0x0C;
				xp=(xp<<8) | ib[0];
				yp=(yp<<6) | ib[1];
				
				if((maskdown(ib,2)!=0xFF) || ((b3 & 0xF)!=0xF))
				{
					r.points[tbs].intensity=0x800;
				}
				else
				{
					return r;
				}
				r.points[tbs].x=yp;
				r.points[tbs].y=xp;

				b3>>=4;
				ib+=3;
			}
			b+=5;
		}
		return r;
	}

	bool getSubExtendedMode(Point& point,const uint8_t* b)
	{
		if(maskdown(b,3)==0xFF)
		{
			return false;
		}
		uint8_t b2=b[2];
		uint16_t xp=b2 & 0x30;
		uint16_t yp=b2 & 0xC0;
		uint16_t ip=b2 & 0x0F;
		xp=(xp << 4) | b[0];
		yp=(yp << 2) | b[1];
		ip<<=8;
		point.x=xp;
		point.y=yp;
		point.intensity=ip;
		return true;
	}
	TrackingPacket getExtended()
	{
		TrackingPacket r;
		uint8_t b[12];
		readbuffer(b,12);
		for(uint8_t s=0;s<4;s++)
		{
			if(!getSubExtendedMode(r.points[s],b))
				return r;
			b+=3;
		}
		return r;		
	}
	TrackingPacket getFull()
	{
		TrackingPacket r;
		uint8_t b[36];
		readbuffer(b,36);
		for(uint8_t s=0;s<4;s++)
		{
			if(!getSubExtendedMode(r.points[s],b))
				return r;
			r.points[s].intensity |= b[8];
			b+=9;
		}
		return r;		
	}	
	TrackingPacket getOther()
	{
		TrackingPacket r;
		uint8_t b[16];
		readbuffer(b,16);
		for(uint8_t s=0;s<4;s++)
		{
			if(!getSubExtendedMode(r.points[s],b+1))
				return r;
			b+=4;
		}
		return r;		
	}
};

#endif
