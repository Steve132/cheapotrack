#ifndef CHEAPOTRACK_H
#define CHEAPOTRACK_H

struct cheapocam_t_s;
typedef struct cheapocam_t_s cheapocam_t;
typedef struct cheapocam_options_t_s
{} cheapocam_options_t;

cheapocam_t* cheapotrack_open(const char* devicefile);
void cheapotrack_close(cheapocam_t* device);

void cheapotrack_set_options(cheapocam_t* device,const cheapocam_options_t* options);

size_t cheapotrack_get_param(cheapocam_t* device,unsigned int param_name,void* buffer,size_t* num_bytes); //returns number of bytes needed to get that parameter
void cheapotrack_set_param(cheapocam_t* device,unsigned int param_name,const void* data,size_t num_bytes);

typedef struct cheapocam_point_t_s
{
	int16_t x,y;
	uint8_t size;
} cheapocam_point_t;

struct cheapocam_track_message_t_s
{
	cheapocam_point_t points[4];
	uint64_t timestamp;
} cheapocam_track_message_t;

size_t cheapotrack_poll(cheapocam_t* device,cheapocam_track_message_t* messagebuffer,size_t message_buffer_length);

#endif
