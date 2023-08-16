//
// SQImports: to be used by plugins that wish to interface with the
//     Squirrel host.
//
// Requires SQModule.h for full functionality of HSQAPI.
//

#ifndef SQIMPORTS_H
#define SQIMPORTS_H

#include "SQModule.h"

typedef HSQAPI * (*Sq_GetSquirrelAPI) (void);
typedef HSQUIRRELVM * (*Sq_GetSquirrelVM) (void);

typedef struct
{
	unsigned int		uStructSize;
	Sq_GetSquirrelAPI	GetSquirrelAPI;
	Sq_GetSquirrelVM	GetSquirrelVM;
} SquirrelImports;

#endif
