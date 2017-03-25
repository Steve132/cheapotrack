#include "ProtocolClient.h"
#include <sstream>
#include <iostream>
#include <serial/serial.h>
using namespace std;
using namespace ct;

static inline uint8_t blockingRead1(serial::Serial& cts)
{
	uint8_t c;
	cts.read(&c,1);
	return c;
}

static uint16_t blockingRead2(serial::Serial& cts)
{
	uint16_t b2=blockingRead1(cts);
	b2 = (b2 << 8)+blockingRead1(cts);
	return b2;
}

static uint32_t blockingRead4(serial::Serial& cts)
{
	uint32_t b4=blockingRead2(cts);
	b4 = (b4 << 16) + blockingRead2(cts);
	return b4;
}
static float blockingRead4f(serial::Serial& cts)		
{
	union { uint32_t iv; float fv; };
	iv=blockingRead4(cts);
	return fv;
}


static inline Point2D normalizePoint2D(uint16_t x,uint16_t y)
{
	Point2D p;
	p.x=static_cast<float>(x)/1023.0f;
	p.y=static_cast<float>(y)/1023.0f;
	return p;
}

static inline Point2D readPoint2D(const TrackingFlagsDataType& pdt,const TrackingFlagsDataType& idt,serial::Serial& cts)
{
	Point2D p;
	uint16_t ux,uy;
	switch(pdt)
	{
		case CT_BYTE:
		{
			ux=blockingRead1(cts);
			uy=blockingRead1(cts);
			uint16_t us=blockingRead1(cts);
			ux |= (us & 0x30) << 4;
			uy |= (us & 0xC0) << 2;
			uint16_t ui=us & 0xF;
			
			p=normalizePoint2D(ux,uy);
			p.intensity=ui;
			return p;
		}
		case CT_USHORT:
		{
			uint16_t ux,uy;
			ux=blockingRead2(cts);
			uy=blockingRead2(cts);
			p=normalizePoint2D(ux,uy);
			break;
		}
		case CT_FLOAT:
		{
			p.x=blockingRead4f(cts);
			p.y=blockingRead4f(cts);
			break;
		}
	};
	switch(idt)
	{
		case CT_BYTE:
		{
			p.intensity=blockingRead1(cts);
			break;
		}
		case CT_USHORT:
		{
			p.intensity=blockingRead2(cts);	
			break;
		}
		case CT_FLOAT:
		{
			p.intensity=blockingRead4f(cts);
			break;
		}
		default:
		{
			break;
		}
	};
	return p;
}



static BarPacket2D blockForBarPacket2DSimple(serial::Serial& cts)
{
	BarPacket2D bp2d;
	bp2d.flags=TrackingFlags2D(blockingRead1(cts));
	bp2d.frame=blockingRead4(cts);
	uint16_t llat[4];
	for(uint_fast8_t i=0;i<4;i++)
	{
		uint16_t latus=blockingRead2(cts);
		llat[i]=latus;		
		bp2d.mask |= ((latus!=0xFFFF) << i);
	}
	TrackingFlagsDataType itype=bp2d.flags.intensity_type,dtype=bp2d.flags.dtype;
	for(uint_fast8_t ci=0;ci<4;ci++)
	{
		CameraPacket2D cp;
		cp.latency=llat[ci];
		for(uint_fast8_t pi=0;pi<4;pi++)
		{
			Point2D p=readPoint2D(dtype,itype,cts);
			cp.points[pi]=p;
		}
		bp2d.cameras[ci]=cp;
	}
	return bp2d;
}





class ProtocolClient::Impl
{
public:
	std::ostream& log;
	std::shared_ptr<serial::Serial> pcts;

	Impl(const std::string& port,ostream& tlog):log(tlog)
	{
		open(port);
	}
	void open(const std::string& port)
	{
		unsigned long baud=ct::CT_PROTOCOL_BAUD;
		pcts=make_shared<serial::Serial>(port, baud, serial::Timeout::simpleTimeout(2000));

		if(!pcts->isOpen())
		{
			throw std::runtime_error("Failed to open "+port);
		}
	}
};

ProtocolClient::ProtocolClient(const std::string& port,ostream& tlog):
	impl(make_shared<ProtocolClient::Impl>(port,tlog))
{
	
}

bool ProtocolClient::blockForMessage()
{
	serial::Serial& cts=*(impl->pcts);
	const ProtocolClient::CallbackSet& cbs=callbacks;

	if(!cts.waitReadable())
	{
		return false;
	}
	uint8_t cresponse=blockingRead1(cts);
	ServerResponse response=(ServerResponse)cresponse;
	switch(response)
	{
		case CT_SERVER_NOP:
			break;
		case CT_ERROR:
		case CT_DEBUGTEXT:
		{
			ostringstream oss;
			oss << (response == CT_ERROR ? "<CT_ERROR>:" : "<CT_DEBUGTEXT>:");
			for(uint8_t c=blockingRead1(cts);c!=0;c=blockingRead1(cts))
			{
				oss.put(c);
			}
			auto fc=(response == CT_ERROR ? cbs.onError : cbs.onDebugText);
			if(fc)
			{
				fc(oss.str());
			}
			break;
		}
		case CT_2D_PACKET_TYPE:
		{
			BarPacket2D bp2d=blockForBarPacket2DSimple(cts);
			if(cbs.onPacket2D)
			{
				cbs.onPacket2D(bp2d);
			}
			break;
		}					
		case CT_SUCCESS:
		case CT_3D_PACKET_TYPE:
		case CT_READ_RESULT:
		default:
		{
			if(cbs.onError)
			{
				cbs.onError("<CT_DRIVER_ERROR>:Unsupported tracking device response byte");
			}
			return false;
		}
	}
	return true;
}


std::ostream& operator<<(std::ostream& out,const ct::Point2D& pt)
{
	return out << "{\"_\":\"ct::Point2D\""
		 << ",\"x\":" << pt.x 
		<< ",\"y\":" << pt.y
		<< ",\"i\":" << pt.intensity
		<< "}";
}
std::ostream& operator<<(std::ostream& out,const ct::CameraPacket2D& cp)
{
	return out << "\t{\"_\":\"ct::CameraPacket2D\",\n"
		<< "\"points\":["
		<< "\n\t\t" << cp.points[0] << ","
		<< "\n\t\t" << cp.points[1] << ","
		<< "\n\t\t" << cp.points[2] << ","
		<< "\n\t\t" << cp.points[3] 
	    	 << "\n\t],"
		<< "\"latency\":" << cp.latency 
		<< "\n}";	
}
std::ostream& operator<<(std::ostream& out,const ct::BarPacket2D& bp2d)
{
	return out << "\t{\"_\":\"ct::BarPacket2D\",\n"
		<< "\"cameras\":["
		<< "\n\t\t" << bp2d.cameras[0] << ","
		<< "\n\t\t" << bp2d.cameras[1] << ","
		<< "\n\t\t" << bp2d.cameras[2] << ","
		<< "\n\t\t" << bp2d.cameras[3] 
	     << "\n\t],"
		<< "\"frame\":" << bp2d.frame
		<< "\n}";
	
}
ProtocolClient::operator bool() const
{
	return impl->pcts->isOpen();
}

