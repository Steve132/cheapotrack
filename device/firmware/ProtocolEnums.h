#ifndef PROTOCOL_ENUMS_H
#define PROTOCOL_ENUMS_H

namespace ct
{
static const int CT_PROTOCOL_BAUD=19200;
static const int CT_NOP=0;

enum ClientCommands
{
	CT_CLIENT_NOP=CT_NOP,
	CT_RESET=1,
	CT_STOP_TRACKING=2,
	CT_START_TRACKING=3,
	
	CT_READ=5,
	CT_WRITE=4,
};

enum ServerResponses
{
	CT_SERVER_NOP=CT_NOP,
	CT_READ_RESULT=1,
	CT_ERROR=2,
	CT_SUCCESS=3,
	CT_DEBUGTEXT=4,
	CT_2D_PACKET_TYPE=0x10,
	CT_3D_PACKET_TYPE=0x20
};

//number of cameras:1,2,4,(8 invalid)...so 0,1,2 (3 invalid)
//number of points: 1,2,3,4 (0 invalid) so two bits....0,1,2,3
//data type=byte,short,float,

//header (2 bytes, first is CT_PACKET, second is TP flags)
//tp flags contains: 
////number of cameras:1,2,3,4
//number of points to seek: 1,2,3,4 (0 invalid) so two bits y=x+1 where x is....0,1,2,3
//data_dtype: 
//0 is byte (implies 4 bit intensity mode), 1 (1 << 1==2) is short, 3 is float, (1 << 2==4) (8 is invalid).
//0 is no intensity, 1 is byte intensity, 2 is short intensity, 3 is float intensity.


enum TrackingFlagsDataTypes
{
	CT_BYTE=0,
	CT_USHORT=1,
	CT_FLOAT=2,
	CT_NONE=3
};

struct TrackingFlags2D
{
	uint8_t num_cameras;
	uint8_t num_points;
	TrackingFlagsDataTypes dtype;
	TrackingFlagsDataTypes intensity_type;
	
	TrackingFlags2D(uint8_t reg=0xF1)://default=0'11110001
		num_cameras((reg >> 6) & 0x3),
		num_points((reg >> 4) & 0x3),
		dtype((reg >> 2) & 0x3),
		intensity_type((reg >> 2) & 0x3)
	{}	
	
	uint8_t bytes_per_point() const
	{
		uint8_t bpp=(2*(1UB << (uint8_t)dtype);
		if(dtype == CT_BYTE)
		{
			bpp+=1;
		}
		else if(intensity_type != CT_NONE)
		{
			bpp+=(1UB << (uint8_t)intensity_type);
		}
		return bpp;
	}
	uint8_t bufsize() const
	{
		//can't be more than 50...
		return num_cameras*num_points*bytes_per_point();
	}
	uint8_t header() const
	{
		return (num_cameras << 6) | (num_points << 4) | (dtype << 2) | (intensity_type);
	}
};
//invalid if number of cameras*number of points*(3 if byte type==0 otherwise (2*2^dtype)+ intensity*idtype)) > 48
/*enum TrackingPacketTypes
{
	CT_TP_1CAMERA4POINTS_2D_8F=0x80+0,	//(2 byte header)(4)frame,4x(2 us) delays,(1)idmask,	1x(4x(3x(4 byte float)))
	CT_TP_4CAMERA1POINTS_2D_8F=0x90+0,	//(2 byte header)(4)frame,4x(2 us) delays,(1)idmask 	4x(1x(3x(4 byte float))
	CT_TP_2CAMERA4POINTS_2D_24US=0xA0+0, 	//(2 byte header)(4)frame,4x(2 us) delays,(1)idmask, 	2x(4x(3x(2 byte short)
	CT_TP_4CAMERA4POINTS_2D_48UB=0xB0+0	//(2 byte header)(4)frame,4x(2 us) delays,(1)idmask,    4x(4x(3x(1 byte))//always 62 bytes to fit into a single USB packet.  Consider flushing the packet first.
	CT_TP_4CAMERA4POINTS_2D_48UB=0xB0+0	//(2 byte header)(4)frame,4x(2 us) delays,(1)idmask,    4x(4x(3x(1 byte))//always 62 bytes to fit into a single USB packet.  Consider flushing the packet first.
}*/

enum ServerRegisters
{
	R_CAMERA_MASK=1,
	R_FRAMERATE_DELAY_US=2,
	R_3DTRACKING_OBJECT_PLANAR_LOCATIONS=3,
	P_READONLY_BIT=0x8000,
	R_ACTIVE=P_READONLY_BIT+1,
	P_PERSISTENT_BIT=0xC000,
	R_CAMERA0_INTRINSIC_4F=P_PERSISTENT_BIT+0x10,
	R_CAMERA1_INTRINSIC_4F=P_PERSISTENT_BIT+0x20,
	R_CAMERA2_INTRINSIC_4F=P_PERSISTENT_BIT+0x30,
	R_CAMERA3_INTRINSIC_4F=P_PERSISTENT_BIT+0x40,
};

}


#endif
