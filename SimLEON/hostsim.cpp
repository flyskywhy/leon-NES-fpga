#include "Hostsim.h"
//*******************************************
//handle of Hostsim.dll
HINSTANCE             hHostSimDll     = NULL ;
//write register function pointer
HostSim_WriteRegister BasicWriteReg8  = NULL ;
//read register function pointer
HostSim_ReadRegister  BasicReadReg8   = NULL ;
//write register function pointer
HostSim_WriteRegister BasicWriteReg32 = NULL ;
//read register function pointer
HostSim_ReadRegister  BasicReadReg32  = NULL ;
//Get/Set Clock Tick
HostSim_GetClkTick    GetClkTick      = NULL ;
HostSim_SetClkTick    SetClkTick      = NULL ;
HostSim_GetClkTick    GetClkTickH     = NULL ;
HostSim_SetClkTick    SetClkTickH     = NULL ;
//Parallel port read/write
HostSim_LPTWrite      LPTWrite        = NULL ;
HostSim_LPTRead       LPTRead         = NULL ;
//PPsim init
HostSim_Init          HostSimInit     = NULL ;
//stopserver
HostSim_VoidVoid      StopServer      = NULL ;
//trap
HostSim_VoidVoid	  Trap		      = NULL ;
//command rti
HostSim_VoidVoid      Command_RTI     = NULL ;
//Enable/disable isr
HostSim_VoidVoid      EnableIsr       = NULL ;
HostSim_VoidVoid      DisableIsr      = NULL ;

//*******************************************
//	release the Hostsim dll.
//	call this function just before the user program exit.
void ReleaseHostSim()
{
#ifdef HOSTSIM_SUPPORT
	if(StopServer!=NULL)
	{
		StopServer();
		StopServer=NULL;
	}
	BasicWriteReg8  = NULL;
	BasicReadReg8   = NULL;
	BasicWriteReg32 = NULL;
	BasicReadReg32  = NULL;
	GetClkTick      = NULL;
	SetClkTick      = NULL;
	GetClkTickH     = NULL;
	SetClkTickH     = NULL;
	LPTWrite        = NULL;
	LPTRead         = NULL;
	Trap            = NULL;
	Command_RTI     = NULL;
	EnableIsr       = NULL;
	DisableIsr      = NULL;
	
	if(hHostSimDll != NULL)
	{
		FreeLibrary(hHostSimDll);
		hHostSimDll = NULL;
	}
#endif
}
//	initialize the Hostsim dll.
//	this function must be called just after the main entrance.
//	return true if successful.
//  nTargetType: 0 - socket(5510)  1:Parallel(1)
BOOL InitHostSim(int nTargetType,int nPort,HostSim_ISR_Routine isr_routine,HostSim_ShowMsg msg)
{
#ifdef HOSTSIM_SUPPORT
	hHostSimDll=LoadLibrary("Hostsim.dll");
	if(hHostSimDll==NULL) return FALSE;

	BasicWriteReg8=(HostSim_WriteRegister)GetProcAddress(hHostSimDll,"BasicWriteReg8");
	BasicReadReg8=(HostSim_ReadRegister)GetProcAddress(hHostSimDll,"BasicReadReg8");
	BasicWriteReg32=(HostSim_WriteRegister)GetProcAddress(hHostSimDll,"BasicWriteReg32");
	BasicReadReg32=(HostSim_ReadRegister)GetProcAddress(hHostSimDll,"BasicReadReg32");
	
	GetClkTick=(HostSim_GetClkTick)GetProcAddress(hHostSimDll,"GetClkTick");
	SetClkTick=(HostSim_SetClkTick)GetProcAddress(hHostSimDll,"SetClkTick");
	GetClkTickH=(HostSim_GetClkTick)GetProcAddress(hHostSimDll,"GetClkTickH");
	SetClkTickH=(HostSim_SetClkTick)GetProcAddress(hHostSimDll,"SetClkTickH");

	LPTWrite=(HostSim_LPTWrite)GetProcAddress(hHostSimDll,"LPT_Write");
	LPTRead=(HostSim_LPTRead)GetProcAddress(hHostSimDll,"LPT_Read");
	Trap=(HostSim_VoidVoid)GetProcAddress(hHostSimDll,"Trap");
	Command_RTI=(HostSim_VoidVoid)GetProcAddress(hHostSimDll,"Command_RTI");
	EnableIsr=(HostSim_VoidVoid)GetProcAddress(hHostSimDll,"EnableIsr");
	DisableIsr=(HostSim_VoidVoid)GetProcAddress(hHostSimDll,"DisableIsr");

	HostSimInit=(HostSim_Init)GetProcAddress(hHostSimDll,"InitPPSim");
	StopServer=(HostSim_VoidVoid)GetProcAddress(hHostSimDll,"StopServer");

	if(    BasicWriteReg8  == NULL 
		|| BasicReadReg8   == NULL 
		|| BasicWriteReg32 == NULL 
		|| BasicReadReg32  == NULL 
		|| GetClkTick      == NULL
		|| SetClkTick      == NULL
		|| LPTWrite        == NULL
		|| LPTRead         == NULL
		|| HostSimInit     == NULL
		|| StopServer      == NULL
		|| Trap			   == NULL
		|| Command_RTI     == NULL
		|| EnableIsr       == NULL
		|| DisableIsr      == NULL
		)
	{
		ReleaseHostSim();
		return FALSE;
	}
	if(!HostSimInit(nTargetType,nPort,isr_routine,msg))
		return FALSE;
#endif
	return TRUE;
}