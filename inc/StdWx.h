/*
 *  StdWx.h
 */
 
#ifndef _COMMON_STD_WX_H_
#define _COMMON_STD_WX_H_

#ifdef WIN32
#include "targetver.h"

 /*
  * When Winuser.h is defined GetClassInfo is is a macro defined as:
  *
  * #define GetClassInfo GetClassInfoW
  *
  * wxWidgets macros such as:
  *
  * wxDECLARE_..._CLASS
  *
  * declares method GetClassInfo so when Winuser.h is included method GetClassInfo is renamed to GetClassInfoW. That's why we define NOUSER.
  */
#define WIN32_LEAN_AND_MEAN

#define NOUSER
#define NOGDI
#define NOMB
#define NOCOMM

  /*
   * Dummy definition of MSG (LPMSG) to make
   *
   * oleidl.h ole2.h
   *
   * happy.
   */
typedef struct tagMSG
{
} MSG, * LPMSG;
#endif

#ifdef NDEBUG
#define wxDEBUG_LEVEL 0
#else
#define wxDEBUG_LEVEL 1
#endif

#endif
