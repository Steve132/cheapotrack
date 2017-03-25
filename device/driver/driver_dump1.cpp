#include<cstdint>
#include "ct_driver.h"
#include "ProtocolEnums.h"
#include "ProtocolClient.h"
using namespace std;

int main(int argc,char** argv)
{
	try
	{
		ct::ProtocolClient pc("/dev/ttyACM0");
		pc.callbacks.onDebugText=[](const std::string& text){cerr << text;};
		pc.callbacks.onPacket2D=[](const ct::BarPacket2D& bp2d){cerr << bp2d;};
		while(pc.blockForMessage());
	}
	catch(const std::exception& e)
	{
		cerr << e.what() << endl;
	}
	return 0;
	
}
