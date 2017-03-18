#include "ProtocolServer.h"
#include "Arduino.h"
using namespace ct;

static uint8_t blockingRead1()
{
	int next;
	do {next=Serial.read();} while(next == -1);
	return next;
}

static uint16_t blockingRead2()
{
	uint16_t b2=blockingRead1();
	b2 = (b2 << 8)+blockingRead1();
	return b2;
}

static uint32_t blockingRead4()
{
	uint32_t b4=blockingRead2();
	b4 = (b4 << 16) + blockingRead2();
	return b4;
}
static void blockingRead4f(float v)		
{
	union { uint32_t iv; float fv };
	iv=blockingRead4();
	return fv;
}
static void blockingWrite2(uint16_t val)
{
	uint8_t v;
	v=(val >> 8) & 0xFF;Serial.write(v);
	v=val & 0xFF;	    Serial.write(v);
}

static void blockingWrite4(uint32_t val)
{
	uint8_t v;
	v=((val >> 24) & 0xFF);Serial.write(v);
	v=((val >> 16) & 0xFF);Serial.write(v);
	v=((val >>  8) & 0xFF);Serial.write(v);
	v=(val & 0xFF);        Serial.write(v);
}
static void blockingWrite4f(float v)		
{
	union { uint32_t iv; float fv };
	fv=v;
	blockingWrite4(iv);
}
static inline void wsuccess()
{
	Serial.write(CT_SUCCESS);
}


void ct::ProtocolServer::cmdStopTracking()
{
	if(tracking_active)
	{
		tracking_active=false;
		wsuccess();
	}
	else
	{
		beginSerialError();
		Serial.print("Cannot stop tracking when it is already started!");
		endSerialError();
	}
}

void ct::ProtocolServer::cmdStartTracking()
{
	if(!tracking_active)
	{
		tracking_active=true;//TODO: reset cameras here too.
		wsuccess();
	}
	else
	{
		beginSerialError();
		Serial.print("Cannot Start Tracking before tracking is stopped!");
		endSerialError();
	}
}

static void persistentWrite(const uint8_t* dat,uint32_t address,uint32_t amount)
{
}



void ct::ProtocolServer::cmdRead()
{
	uint16_t address=blockingRead2();
	if(tracking_active)
	{

	}	
}
void ct::ProtocolServer::cmdWrite()
{
	if(tracking_active)
	{
		beginSerialError();
		Serial.print("Cannot write registers while tracking is enabled");
		endSerialError();
		return;
	}
	uint16_t address=blockingRead2();
	if((address & 0xC000) == P_READONLY_BIT)
	{
		beginSerialError();
		Serial.print("Cannot set read only register ");
		Serial.print(address,HEX);
		endSerialError();
		return;
	}
	
}


void ct::ProtocolServer::onCommandReady()
{
	uint8_t cmd=blockingRead1();
	switch(cmd)
	{
	case CT_STOP_TRACKING:
		cmdStopTracking();
		break;
	case CT_START_TRACKING:
		cmdStartTracking();
		break;
	case CT_READ:
		cmdRead();
		break;
	case CT_WRITE:
		cmdWrite();
		break;
	default:
		break;
	};
}

void ct::ProtocolServer::beginSerialDebug()
{
	Serial.write(CT_DEBUGTEXT);
}
void ct::ProtocolServer::endSerialDebug()
{
	Serial.write('\0');
	Serial.flush();
}
void ct::ProtocolServer::beginSerialError()
{
	Serial.write(CT_ERROR);
}
void ct::ProtocolServer::endSerialError()
{
	Serial.write('\0');
	Serial.flush();
}


//Okay, no sorting for now
//Don't sort cameras.
//if active mask is > num_cameras then do one send per group where a group is popcnt active/2.

//invalid if number of cameras*number of points*(3 if byte type==0 otherwise (2*2^dtype)+ intensity*idtype)) > 48
/*enum TrackingPacketTypes
{
	CT_TP_1CAMERA4POINTS_2D_8F=0x80+0,	//(2 byte header)(4)frame,4x(2 us) delays,(1)idmask,	1x(4x(3x(4 byte float)))
//
/*
void write2DPacketAdvanced(const TrackingPacket2D& tptype,uint8_t activemask,uint32_t frameid,const uint16_t* delay,const PointGroup* pgroups)
{
	Serial.write(CT_2D_PACKET_TYPE);
	Serial.write(tptype.reg);
	blockingWrite4(frameid);

	uint_fast8_t am_popcnt=0;
	for(uint_fast8_t ci=0;ci < 4;ci++)
	{
		if((activemask >> ci) & 1)
		{
			am_popcnt++;
			blockingWrite2(delay[ci]);	//if the cam is inactive the delay will be 0xFFFF
		}
		else
		{
			blockingWrite2(0xFFFF);
		}
	}

	uint_fast8_t num_cams_per_group=tptype.num_cameras;
	uint_fast8_t num_points_per_cam=tptype.num_points;
	for(uint8_t ci=0;ci<num_cams_per_group)
	{
		
	}
}*/

static inline void writeIntensity(const TrackingPacketDataType& idt,const Point& p)
{
	switch(idt)
	{
		case CT_BYTE:
		{
			Serial.write(p.intensity >> 4);
			break;
		}
		case CT_USHORT:
		{
			blockingWrite2(p.intensity);	
			break;
		}
		case CT_FLOAT:
		{
			blockingWrite4f(p.intensity);
			break;
		}
		default:
		{
			break;
		}
	};
}

static inline void writePoint(const TrackingPacketDataType& pdt,const TrackingPacketDataType& idt,const Point& p)
{
	switch(pdt)
	{
		case CT_BYTE:
		{
			uint8_t s=(p.x >> 8) | (p.y >> 8) | ((p.intensity >> 4) & 0xF);
			Serial.write(p.x & 0xFF);
			Serial.write(p.y & 0xFF);
			Serial.write(s);
			return;
		}
		case CT_USHORT:
		{
			blockingWrite2(p.x);
			blockingWrite2(p.y);
			break;
		}
		case CT_FLOAT:
		{
			blockingWrite4f(static_cast<float>(p.x)/1023.0f);
			blockingWrite4f(static_cast<float>(p.y)/1023.0f);
			break;
		}
		default:
		{
			return;
		}
	};
	writeIntensity(idt,p);
}

static inline void write2DPacketSimple(const TrackingPacket2D& tptype,uint8_t activemask,uint32_t frameid,const uint16_t* delay,const PointGroup* pgroups)
{
	if(tptype.num_points != 4 || tptype.num_cameras !=4 || tptype.dtype !=CT_BYTE)
	{
		beginSerialError();
		Serial.print("Selected Packet Type MUST be (4,4,byte,1) (0xF1).");
		endSerialError();
		
	}
	Serial.write(CT_2D_PACKET_TYPE);
	Serial.write(tptype.reg);
	blockingWrite4(frameid);

	for(uint_fast8_t ci=0;ci<4;ci++)
	{
		blockingWrite2(delay[ci]); //MUST be 0xFFFF for an inactive cam.
	}
	for(uint_fast8_t ci=0;ci<4;ci++)
	{
		for(uint_fast8_t pi=0;pi<4;pi++)
		{
			writePoint(pgroups[ci].points[pi]);
		}
	}
	Serial.send_now();	
}


void ct::ProtocolServer::onFrameCapture(const PointGroup* pointgroups,const uint16_t* delay,uint8_t num_cams)
{
	if(tracking_active)
	{
		write2DPacketSimple(0xF1,cam_mask & 0xF,current_frame,delay,pointgroups);
	}
	current_frame++;
}







