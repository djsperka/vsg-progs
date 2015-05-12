#include "NetworkData.h"

APacketP createPacket(unsigned int type, unsigned int length)
{
    APacketP output = (APacketP) malloc((length) + sizeof(unsigned int));
	output->type = type;
    output->length = length;
    return output;
};
