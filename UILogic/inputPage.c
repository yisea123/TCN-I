#include "pageCommon.h"
#include "../Logic/managerment.h"
#include "CPrintf.h"

#ifdef __cplusplus
extern "C" {
#endif

void inputPageButtonProcess(uint16 control_id, uint8 state)
{
	switch(control_id)
	{
		case INPUT_BACK_BUTTON:
		break;
		case INPUT_FORWARD_BUTTON:
		break;
		default:
			cDebug("inputPage BUTTON error!\n");
		break;
	}
}

#ifdef __cplusplus
}
#endif
