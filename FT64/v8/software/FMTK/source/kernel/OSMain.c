#include <fmtk/config.h>
#include <fmtk/device.h>

extern DCB DeviceTable[NR_DCB];
extern int pti_CmdProc(int cmd, int p1, int p2, int p3, int p4);

void OSMain()
{
	DCB *p;

	p = &DeviceTable[0];
	strncpy(p->name,"\x04NULL",12);
	p->type = DVT_Unit;
	p->UnitSize = 0;
	
	p = &DeviceTable[9];
	strncpy(p->name,"\x03PTI",12);
	p->type = DVT_Unit;
	p->UnitSize = 1;
	p->CmdProc = pti_CmdProc;
	(*(p->CmdProc))(DVC_Setup, 0, 0, 0, 0);

}
