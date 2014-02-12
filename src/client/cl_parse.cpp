#include <cassert>
#include "../defs.h"

#if 0


void CL_ParseSnapshot(szb_t *msg, clSnapshot_t *from, clSnapshot_t *to) {
	assert(msg != NULL);
	assert(from != NULL);
	assert(to != NULL);

	ptrdiff_t snapNum = szb_read32(msg);
	//


}


// Client has received a packet from server
// Process it
void CL_ReadPacket(szb_t *msg) {
	assert(msg != NULL);


}
#endif