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
}
void ct::ProtocolServer::beginSerialError()
{
	Serial.write(CT_ERROR);
}
void ct::ProtocolServer::endSerialError()
{
	Serial.write('\0');
}

void ct::ProtocolServer::onFrameCapture(const PointGroup* pointgroups,const uint16_t* delay,uint8_t num_cams)
{
	
	current_frame++;
}
