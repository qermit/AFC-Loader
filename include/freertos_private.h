/*
 * freertos_private.h
 *
 * Created: 2015-10-19 11:32:34
 *  Author: pmiedzik
 */ 


#ifndef FREERTOS_PRIVATE_H_
#define FREERTOS_PRIVATE_H_


struct afc_generic_task_param {
	SemaphoreHandle_t afc_init_semaphore;
	uint8_t do_not_init;
	uint8_t do_loop;
};


#endif /* FREERTOS_PRIVATE_H_ */