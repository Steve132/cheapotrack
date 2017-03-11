#include "PixArt.h"
#include "Arduino.h"

#define PA_TEENSY3_ASYNC

void PixArt::write_2byte(uint8_t d1,uint8_t d2)
{
	w.beginTransmission(IRslaveAddress);
	w.write(d1); 
	w.write(d2);
	w.endTransmission();
}
	

void PixArt::reset(const PixArt::Settings s)
{
	mode=s.mode;
	static const uint8_t DELAYAMOUNT=100;
	

	if(mode != SIMPLE)
	{
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
	}
	else
	{
		write_2byte(0x30,0x01); delay(10);
		write_2byte(0x30,0x08); delay(10);
		write_2byte(0x06,0x90); delay(10);
		write_2byte(0x08,0xC0); delay(10);
		write_2byte(0x1A,0x40); delay(10);
		write_2byte(0x33,0x33); delay(10);
	}
	status=IDLE;
	delay(100);
}
void PixArt::request()
{
	w.beginTransmission(IRslaveAddress);
	w.write(0x36);
	w.endTransmission();
	uint8_t sz=modesize();
	status=WAITING;
	
	#if defined(PA_TEENSY3_WIRE) && defined(PA_TEENSY3_ASYNC)
		w.sendRequest(IRslaveAddress,sz,I2C_NOSTOP);
	#elif defined(PA_TEENSY3_WIRE)
		w.requestFrom(IRslaveAddress,sz);
		status=COMPLETE;
	#else // defined(PA_ARDUINO_WIRE)
		w.requestFrom(IRslaveAddress,sz);
		status=COMPLETE;
	#endif
}
PixArt::Status PixArt::check_status()
{
	#if defined(PA_TEENSY3_WIRE) && defined(PA_TEENSY3_ASYNC)
		status=w.done()==1 ? COMPLETE : WAITING;
	#else // defined(PA_ARDUINO_WIRE)
	#endif
	return status;
}
TrackingPacket PixArt::read()
{

	#if defined(PA_TEENSY3_WIRE) && defined(PA_TEENSY3_ASYNC)
		if(status==WAITING) w.finish();
	#else//if defined(PA_ARDUINO_WIRE)
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
	case SIMPLE:
		return getOther();
	};
}

void PixArt::readbuffer(uint8_t* buf,uint8_t count)
{
	if(mode==SIMPLE)
	{
		Serial.print("simple\n");
		for(uint8_t i=0;w.available() && i < count;i++)
		{ 
			buf[i] = w.read();
		}

		Serial.print("b:");
		for(uint8_t i=0;i<count;i++)
		{
			Serial.print(buf[i],HEX);
			Serial.print(" ");
		}
		Serial.print("\n");
	}
	else
	{
		if(w.available()>=count)
		{
			for(register uint8_t i=0;i<count;)
			{
				if(w.available())
				{
					buf[i++]=w.read();
				}
			}
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
TrackingPacket PixArt::getBasic()
{
	TrackingPacket r;
	uint8_t b[10];
	readbuffer(b,10);
	uint8_t* bp=b;
	for(uint8_t s=0;s<4;s+=2)
	{
		uint8_t b3=bp[3];
		const uint8_t* ib=bp;
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
		bp+=5;
	}
	return r;
}

bool PixArt::getSubExtendedMode(Point& point,const uint8_t* b)
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
TrackingPacket PixArt::getExtended()
{
	TrackingPacket r;
	uint8_t b[12];
	readbuffer(b,12);
	uint8_t* bp=b;
	for(uint8_t s=0;s<4;s++)
	{
		if(!getSubExtendedMode(r.points[s],bp))
			return r;
		bp+=3;
	}
	return r;		
}
TrackingPacket PixArt::getFull()
{
	TrackingPacket r;
	uint8_t b[36];
	readbuffer(b,36);
	uint8_t* bp=b;
	for(uint8_t s=0;s<4;s++)
	{
		if(!getSubExtendedMode(r.points[s],bp))
			return r;
		r.points[s].intensity |= bp[8];
		bp+=9;
	}
	return r;		
}	
TrackingPacket PixArt::getOther()
{
	TrackingPacket r;
	uint8_t b[16];
	readbuffer(b,16);
	if(mode==SIMPLE)
	{
		uint8_t* bp=b+1;
		for(int i=0;i<4;i++)
		{
			uint8_t s;
			r.points[i].x=bp[0];
			r.points[i].y=bp[1];
			s=bp[2];
			r.points[i].x += static_cast<uint16_t>(s & 0x30) <<4;
			r.points[i].y += static_cast<uint16_t>(s & 0xC0) <<2;
			bp+=3;
		}

	}
	else
	{
		uint8_t* bp=b;
		for(uint8_t s=0;s<4;s++)
		{
			if(!getSubExtendedMode(r.points[s],bp+1))
				return r;
			bp+=4;
		}
	}
	return r;		
}

