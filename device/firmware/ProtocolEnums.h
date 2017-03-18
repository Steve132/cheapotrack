#ifndef PROTOCOL_ENUMS_H
#define PROTOCOL_ENUMS_H

namespace ct
{
static const int CT_PROTOCOL_BAUD=19200;
static const int CT_NOP=0;

enum ClientCommand
{
	CT_CLIENT_NOP=CT_NOP,
	CT_RESET=1,
	CT_STOP_TRACKING=2,
	CT_START_TRACKING=3,
	
	CT_READ=5,
	CT_WRITE=4,
};

enum ServerResponse
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


enum TrackingFlagsDataType
{

	CT_NONE=0,
	CT_BYTE=1,
	CT_USHORT=2,
	CT_FLOAT=3,
};

struct TrackingFlags2D
{
	uint8_t num_cameras;
	uint8_t num_points;
	TrackingFlagsDataType dtype;
	TrackingFlagsDataType intensity_type;

	uint8_t flagbits;
	
	TrackingFlags2D(uint8_t treg=0xF1)://default=0'11110001
		num_cameras( ((treg >> 6) & 0x3)+1),
		num_points(((treg >> 4) & 0x3)+1),
		dtype((treg >> 2) & 0x3),
		intensity_type((treg >> 2) & 0x3),
		reg(treg)
	{}	
	
	uint8_t bytes_per_point() const
	{
		uint8_t bpp=(2*(1UB << ((uint8_t)(dtype)-1)));
		if(dtype == CT_BYTE)
		{
			bpp+=1;
		}
		else if(intensity_type != CT_NONE)
		{
			bpp+=(1UB << ((uint8_t)(intensity_type)-1));
		}
		return bpp;
	}
	uint8_t buildflags() const
	{
		return ((num_cameras-1) << 6) | ((num_points-1) << 4) | (dtype << 2) | (intensity_type);
	}
	uint8_t bufsize() const
	{
		//can't be more than 50...
		return num_cameras*num_points*bytes_per_point();
	}
	
};


enum ServerRegister
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
