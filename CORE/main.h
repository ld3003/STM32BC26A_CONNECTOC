#ifndef __main_h__
#define __main_h__

#include "stm32f10x.h"
#include "bsp.h"
#include <stdio.h>
#include <stdlib.h>
#include "utimer.h"
#include "task.h"
#include "mem.h"
#include "serialport.h"
#include "mainloop.h"
#include "bsp.h"
#include "config.h"

enum SYSTEM_STATUS {
	SYSTEM_STATUS_INIT = 0,
	SYSTEM_STATUS_WAIT_WAKEUP,
	SYSTEM_STATUS_DEEPSLEEP,		//���˯�ߣ�ֻ�е��´�����ʱ�Żᱻ����
	SYSTEM_STATUS_RUN,
	SYSTEM_STATUS_CHECKALARM,
};



#endif
