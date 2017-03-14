#include "ProtocolServer.h"
#include "Arduino.h"
using namespace ct;

static uint8_t blockingRead()
{
	int next;
	do {next=Serial.read();} while(next == -1);
	return next;
}



void ct::ProtocolServer::onCommandReady()
{
	uint8_t cmd=blockingRead();
	
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
