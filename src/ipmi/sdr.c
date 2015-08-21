/*
 * sdr.c
 *
 *   AFCIPMI  --
 *
 *   Copyright (C) 2015  Piotr Miedzik  <P.Miedzik@gsi.de>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sdr.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "board.h"
#include "ipmi_oem.h"

#define NUM_SENSOR 		2	/* Number of sensors */
#define NUM_SDR         (NUM_SENSOR+1)   /* Number of SDRs */

#define HOT_SWAP_SENSOR     1
#define NUM_SDR_FMC_12V 	2

#define HOT_SWAP        0xF2

//module handle sensor status
#define HOT_SWAP_CLOSED   0x00
#define HOT_SWAP_OPENED   0x01
#define HOT_SWAP_QUIESCED 0x02

#define HOT_SWAP_STATE_HANDLE_CLOSED     (1 << 0)
#define HOT_SWAP_STATE_HANDLE_OPENED     (1 << 1)
#define HOT_SWAP_STATE_QUIESCED          (1 << 2)
#define HOT_SWAP_STATE_BP_SDOWN          (1 << 3)
#define HOT_SWAP_STATE_BP_FAIL           (1 << 4)
#define HOT_SWAP_STATE_URTM_PRSENT       (1 << 5)
#define HOT_SWAP_STATE_URTM_ABSENT       (1 << 6)
#define HOT_SWAP_STATE_URTM_COMPATIBLE   (1 << 7)
#define HOT_SWAP_STATE_URTM_INCOMPATIBLE (1 << 0)


enum SDR_TYPE {
	TYPE_01 = 0x1,
	TYPE_02 = 0x2,
	TYPE_12 = 0x12
};

/* Management Controller Device Locator Record 37.9 SDR Type 12h */
static const SDR_type_12h_t  SDR0 = {
		.hdr.recID_LSB = 0x00, /* record number, LSB - filled by sdr_init() */
		.hdr.recID_MSB = 0x00, /* record number, MSB - filled by sdr_init() */
		.hdr.SDRversion = 0x51, /* IPMI protocol version */
		.hdr.rectype = 0x12, /* record type: device locator record */
		.hdr.reclength = sizeof(SDR_type_12h_t) - sizeof(SDR_entry_hdr_t), /* record length - filled by sdr_init() */

/* record key bytes */
		.slaveaddr = 0x00, // owner ID??
		.chnum = 0x00,
		.power_notification_global_init = NUM_SDR,
		.device_cap = 0x3b,
		.reserved[0] = 0x00,
		.reserved[1] = 0x00,
		.reserved[2] = 0x00,
		.entityID = 0x60, //0x60 | ((0x76-0x70) >>1), // Entry ID?
		.entityinstance = 0x00,
		.OEM = 0x00,
		.IDtypelen = 0xcc, /* 8 bit ASCII, number of bytes */
		.IDstring = {'A', 'F', 'C', ' ', 'L', 'O', 'A', 'D', 'E', '2' }
};

/* Hot-Swap sensor */
 static const SDR_type_02h_t SDR_HOT_SWAP = {

.hdr.recID_LSB = 0x01,
.hdr.recID_MSB = 0x00,
.hdr.SDRversion = 0x51,
.hdr.rectype = TYPE_02,
.hdr.reclength = sizeof(SDR_type_02h_t) - sizeof(SDR_entry_hdr_t),

.ownerID = 0x00, /* i2c address, -> SDR_Init */
.ownerLUN = 0x00, /* sensor owner LUN */
.sensornum = HOT_SWAP_SENSOR, /* sensor number */

/* record body bytes */
.entityID = 0x60, /* entity id: AMC Module */
.entityinstance = 0x00, /* entity instance -> SDR_Init */
.sensorinit = 0x03, /* init: event generation + scanning enabled */
.sensorcap = 0xc1, /* capabilities: auto re-arm,*/
.sensortype = HOT_SWAP, /* sensor type: HOT SWAP*/
.event_reading_type = 0x6f, /* sensor reading*/
.assertion_event_mask = { 0x07, /* LSB assert event mask: 3 bit value */
0x00 }, /* MSB assert event mask */
.deassertion_event_mask = { 0x07, /* LSB deassert event mask: 3 bit value */
0x00 }, /* MSB deassert event mask */
.readable_threshold_mask = 0x00, /* LSB: readabled Threshold mask: all thresholds are readabled:  */
.settable_threshold_mask = 0x00, /* MSB: setabled Threshold mask: all thresholds are setabled: */
.sensor_units_1 = 0xc0, /* sensor units 1 :*/
.sensor_units_2 = 0x00, /* sensor units 2 :*/
.sensor_units_3 = 0x00, /* sensor units 3 :*/
.record_sharing[0] = 0x00, /* Linearization */
.record_sharing[1] = 0x00, /* Linearization */
.pos_thr_hysteresis = 0x00, /* positive going Threshold hysteresis value */
.neg_thr_hysteresis = 0x00, /* negative going Threshold hysteresis value */
.reserved1 = 0x00, /* reserved */
.reserved2 = 0x00, /* reserved */
.reserved3 = 0x00, /* reserved */
.OEM = 0x00, /* OEM reserved */
.IDtypelen = 0xcc, /* 8 bit ASCII, number of bytes */
.IDstring = { 'F', 'R', 'U', ' ', 'H', 'O', 'T', '_', 'S', 'W', 'A', 'P' } /* sensor string */
};

 /* 12V sensor */
 static const SDR_type_01h_t SDR_FMC1_12V = {

 .hdr.recID_LSB = NUM_SDR_FMC_12V+1,
 .hdr.recID_MSB = 0x00,
 .hdr.SDRversion = 0x51,
 .hdr.rectype = TYPE_01,
 .hdr.reclength = sizeof(SDR_type_01h_t) - sizeof(SDR_entry_hdr_t),

 .ownerID = 0x00, /* i2c address, -> SDR_Init */
 .ownerLUN = 0x00, /* sensor owner LUN */
 .sensornum = NUM_SDR_FMC_12V, /* sensor number */

 /* record body bytes */
 .entityID = 0x60, /* entity id: AMC Module */
 .entityinstance = 0x00, /* entity instance -> SDR_Init */
 .sensorinit = 0x7f, /* init: event generation + scanning enabled */
 .sensorcap = 0x58, /* capabilities: auto re-arm,*/
 .sensortype = 0x02, /* sensor type: HOT SWAP*/
 .event_reading_type = 0x01, /* sensor reading*/
 .assertion_event_mask = { 0x07, /* LSB assert event mask: 3 bit value */
 0x00 }, /* MSB assert event mask */
 .deassertion_event_mask = { 0x07, /* LSB deassert event mask: 3 bit value */
 0x00 }, /* MSB deassert event mask */
 .readable_threshold_mask = 0x00, /* LSB: readabled Threshold mask: all thresholds are readabled:  */
 .settable_threshold_mask = 0x00, /* MSB: setabled Threshold mask: all thresholds are setabled: */
 .sensor_units_1 = 0xc0, /* sensor units 1 :*/
 .sensor_units_2 = 0x4, /* sensor units 2 :*/
 .sensor_units_3 = 0x00, /* sensor units 3 :*/
 .linearization = 0x00, /* Linearization */
 .M = 63, /* M */
 .M_tol = 0x00, /* M - Tolerance */
 .B = 0x00, /* B */
 .B_accuracy = 0x00, /* B - Accuracy */
 .acc_exp_sensor_dir = 0x00, /* Sensor direction */
 .Rexp_Bexp = 0xD0, /* R-Exp , B-Exp */
 .analog_flags = 0x00, /* Analogue characteristics flags */
 .nominal_reading = 195, /* Nominal reading */
 .normal_max = 255, /* Normal maximum */
 .normal_min = 0x00, /* Normal minimum */
 .sensor_max_reading = 255, /* Sensor Maximum reading */
 .sensor_min_reading = 0x00, /* Sensor Minimum reading */
 .upper_nonrecover_thr = 250, /* Upper non-recoverable Threshold */
 .upper_critical_thr = 236, /* Upper critical Threshold */
 .upper_noncritical_thr = 225, /* Upper non critical Threshold */
 .lower_nonrecover_thr = 193, /* Lower non-recoverable Threshold */
 .lower_critical_thr = 174, /* Lower critical Threshold */
 .lower_noncritical_thr = 178, /* Lower non-critical Threshold */
 .pos_thr_hysteresis = 2, /* positive going Threshold hysteresis value */
 .neg_thr_hysteresis = 2, /* negative going Threshold hysteresis value */
 .reserved1 = 0x00, /* reserved */
 .reserved2 = 0x00, /* reserved */
 .OEM = 0x00, /* OEM reserved */
 .IDtypelen = 0xc0 | 5, /* 8 bit ASCII, number of bytes */
 .IDstring = { '+', '1', '2', 'V' } /* sensor string */
 };

typedef struct {
	enum SDR_TYPE type;
	void * sdr;
	unsigned char sdr_length;
	sensor_data_entry_t * data;
	void * callback_function;
} sensor_t;

sensor_data_entry_t sdrData[NUM_SDR];
const sensor_t const sensor_array[NUM_SDR]  = {
	{ SDR0.hdr.rectype,         (void *) & SDR0,         sizeof(SDR0),  &sdrData[0] },
	{ SDR_HOT_SWAP.hdr.rectype, (void *) & SDR_HOT_SWAP, sizeof(SDR_HOT_SWAP), &sdrData[1] },
	{ SDR_FMC1_12V.hdr.rectype, (void *) & SDR_FMC1_12V, sizeof(SDR_FMC1_12V), &sdrData[2] }
};

// @todo: check if valid
#define SDR_ARRAY_LENGTH (sizeof(sensor_array) / sizeof(sensor_t))

// @todo: check if valid
size_t sdr_get_size_by_type(enum SDR_TYPE type){
	switch (SDR_TYPE) {
		case TYPE_01:
			return sizeof(SDR_type_01h_t);
			break;
		case TYPE_02:
			return sizeof(SDR_type_02h_t);
			break;
		case TYPE_12:
			return sizeof(SDR_type_12h_t);
			break;
		default:
			return 0;
			break;
	}

}

// @todo: check if valid
size_t sdr_get_size_by_entry(int id){
	if (id >= SDR_ARRAY_LENGTH) return 0;
	return sdr_get_size_by_type(sensor_array[i]);
}


static unsigned short reservationID;

void ipmi_se_get_sdr_info(struct ipmi_msg *req, struct ipmi_msg* rsp) {
	int len = rsp->msg.data_len;

	if (req->msg.data_len == 0 || req->msg_data[0] == 0) {
		rsp->msg_data[len++] = NUM_SENSOR;
	} else {
		rsp->msg_data[len++] = NUM_SDR;
	}
	rsp->msg_data[len++] = 0x01; // if dynamic additional 4 bytes required (see Table 20-2 Get Device SDR INFO Command
	rsp->msg.data_len = len;
	rsp->retcode = IPMI_CC_OK;
}

struct __attribute__((__packed__)) ipmi_se_get_sdr_param {
	unsigned char reservation_id[2];
	unsigned char record_id[2];
	unsigned char offset;
	unsigned char size;

} ipmi_se_get_sdr_param_t;

#define FRU_CTLCODE_COLD_RST          (0)       // FRU Control command cold reset code
#define FRU_CTLCODE_WARM_RST          (1)       // FRU Control command warm reset code
#define FRU_CTLCODE_REBOOT            (2)       // FRU Control command reboot code
#define FRU_CTLCODE_QUIESCE           (4)       // FRU Control command quiesce code
SemaphoreHandle_t semaphore_fru_control;

unsigned char payload_ctrl_code;
void do_quiesced_init() {
	payload_ctrl_code = 0;
	semaphore_fru_control = xSemaphoreCreateBinary();
}
void do_quiesced(unsigned char ctlcode){
	payload_ctrl_code = ctlcode;
	xSemaphoreGive(semaphore_fru_control);
}

void ipmi_se_get_sdr(struct ipmi_msg *req, struct ipmi_msg* rsp) {
	struct ipmi_se_get_sdr_param * params;
	params = (struct ipmi_se_get_sdr_param *) req->msg_data;
	//unsigned short test = params->
	unsigned short record_id = params->record_id[0]
			| (params->record_id[1] << 8);
	unsigned short reservation_id = params->reservation_id[0]
			| (params->reservation_id[1] << 8);
	int len = rsp->msg.data_len;
	int i;
	int index;

	(void )reservation_id; // @todo: unused variable


	// @todo: add reservation check

	if (record_id >= NUM_SDR
			|| (params->offset + params->size) > sensor_array[record_id].sdr_length) {
		rsp->retcode = IPMI_CC_REQ_DATA_NOT_PRESENT;
		return;
	} else if (record_id == NUM_SDR - 1) {
		rsp->msg_data[len++] = 0xFF;
		rsp->msg_data[len++] = 0xFF;
	} else {
		rsp->msg_data[len++] = (record_id + 1) & 0xff; /* next record ID */
		rsp->msg_data[len++] = (record_id + 1) >> 8; /* next record ID */
	}

	SDR_entry_hdr_t *pHDR;
	unsigned char tmp_c;
	pHDR = (SDR_entry_hdr_t*) sensor_array[record_id].sdr;
	unsigned char * pSDR = (unsigned char*) sensor_array[record_id].sdr;



	for (i = 0; i < params->size; i++) {
		index = i + params->offset;
		tmp_c = pSDR[i + params->offset];

		if (index == 5) {
			tmp_c = sensor_array[record_id].data->ownerID;
		} else if ( pHDR->rectype == 0x02 || pHDR->rectype == 0x01 ) {
			if (index == 8 ) {
				tmp_c = sensor_array[record_id].data->entityID;
			}
		}else if ( pHDR->rectype == 0x12 || pHDR->rectype == 0x11 ) {
			if (index == 12 ) {
				tmp_c = sensor_array[record_id].data->entityID;
			}
		}

		rsp->msg_data[len++] = tmp_c;
	}

	rsp->msg.data_len = len;
	rsp->retcode = IPMI_CC_OK;

}

void ipmi_se_reserve_device_sdr(struct ipmi_msg *req,
		struct ipmi_msg* rsp) {
	int len = rsp->msg.data_len;

	reservationID++;
	if (reservationID == 0)
		reservationID = 1;
	rsp->msg_data[len++] = reservationID & 0xff;
	rsp->msg_data[len++] = reservationID >> 8;

	rsp->msg.data_len = len;
	rsp->retcode = IPMI_CC_OK;

}

// NETFN_SE = 0x04,
// IPMI_GET_SENSOR_READING_CMD = 0x2d,
void ipmi_se_get_sensor_reading(struct ipmi_msg *req,
		struct ipmi_msg* rsp) {
	int sensor_number = req->msg_data[0];
	int sensor_data_index = req->msg_data[0] +1;
	int len = rsp->msg.data_len;

	if (sensor_number >= NUM_SENSOR) {
		rsp->retcode = IPMI_CC_REQ_DATA_NOT_PRESENT;
		return;
	}

	//SDR_entry_hdr_t * pHDR =  sdrPtr[sensor_number];

	rsp->msg_data[len++] = sensor_array[sensor_data_index].data->readout_value;
	rsp->msg_data[len++] = 0x40;
	rsp->msg_data[len++] = sensor_array[sensor_data_index].data->comparator_status;

	rsp->msg.data_len = len;
	rsp->retcode = IPMI_CC_OK;

}

void sdr_init(uint8_t ipmiID) {

	unsigned char i;
	for (i = 0; i < NUM_SDR; i++) {
		sensor_array[i].data->entityID =  0x60 | ((ipmiID - 0x70) >> 1);
		sensor_array[i].data->ownerID = ipmiID;

		// @todo: remove this HOT_SWAP_SENSOR case, to enable first event
		if (i == HOT_SWAP_SENSOR) {
			sensor_array[i].data->comparator_status = HOT_SWAP_STATE_HANDLE_OPENED;
		} else
			sensor_array[i].data->comparator_status = 0;
		sensor_array[i].data->readout_value = 0;
	}
}



#ifdef FREERTOS_CONFIG_H


#define SENSOR_DELAY_PERIOD 1000
extern struct I2C_Mutex i2c_mutex_array[2];


#define INA220_BUS_REG 0x02

static void INA222_init(I2C_ID_T i2c)
{
	uint8_t ch[3];
	ch[0] = 0 ; // INA220_CFG_REG
	ch[1] = 0x01;
	ch[2] = 0x9f;
	Chip_I2C_MasterSend(i2c, 0x40, ch, 3);
}

static uint16_t INA222_readVolt(I2C_ID_T i2c, bool raw) {
	uint8_t ch[2];
	Chip_I2C_MasterCmdRead(i2c, 0x40, INA220_BUS_REG, ch, 2);
	uint16_t tmpVal = 0.0;
	tmpVal = (0x1fE0 & (ch[0] << 5)) | (0x1f & (ch[1] >> 3));
	if (raw == false)
		tmpVal = tmpVal * 4;
	return tmpVal;
}

void setDC_DC_ConvertersON(bool on) {
	bool _on = on;
	//_on = false;

	// @todo: check vadj relationship
	bool _on_fmc1 = false | on;
	bool _on_fmc2 = false | on;
	//Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 17, true);
	//if (!on)
//		Chip_GPIO_SetPinState(LPC_GPIO, 0, 17, on );


	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_FMC1_PVADJ_PORT, GPIO_EN_FMC1_PVADJ_PIN, _on_fmc1);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EM_FMC1_P12V_PORT, GPIO_EM_FMC1_P12V_PIN, _on_fmc1);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_FMC1_P3V3_PORT, GPIO_EN_FMC1_P3V3_PIN, _on_fmc1);

	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_FMC2_PVADJ_PORT, GPIO_EN_FMC2_PVADJ_PIN, _on_fmc2);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EM_FMC2_P12V_PORT, GPIO_EM_FMC2_P12V_PIN, _on_fmc2);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_FMC2_P3V3_PORT, GPIO_EN_FMC2_P3V3_PIN, _on_fmc2);


	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_P1V0_PORT, GPIO_EN_P1V0_PIN, _on);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_P1V8_PORT, GPIO_EN_P1V8_PIN, _on); // <- this one causes problems if not switched off before power loss

	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_P1V2_PORT, GPIO_EN_P1V2_PIN, _on);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_1V5_VTT_PORT, GPIO_EN_1V5_VTT_PIN, _on);
	Chip_GPIO_SetPinState(LPC_GPIO, GPIO_EN_P3V3_PORT, GPIO_EN_P3V3_PIN, _on);

//if (on)
//		Chip_GPIO_SetPinState(LPC_GPIO, 0, 17, on );

}
void initializeDCDC() {
	setDC_DC_ConvertersON(false);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_P1V2_PORT, GPIO_EN_P1V2_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_P1V8_PORT, GPIO_EN_P1V8_PIN, true);

	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_FMC2_P3V3_PORT, GPIO_EN_FMC2_P3V3_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_FMC2_PVADJ_PORT, GPIO_EN_FMC2_PVADJ_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EM_FMC2_P12V_PORT, GPIO_EM_FMC2_P12V_PIN, true);

	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EM_FMC1_P12V_PORT, GPIO_EM_FMC1_P12V_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_FMC1_P3V3_PORT, GPIO_EN_FMC1_P3V3_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_FMC1_PVADJ_PORT,	GPIO_EN_FMC1_PVADJ_PIN, true);

	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_P3V3_PORT, GPIO_EN_P3V3_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_1V5_VTT_PORT, GPIO_EN_1V5_VTT_PIN, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, GPIO_EN_P1V0_PORT, GPIO_EN_P1V0_PIN, true);
}



void vTaskSensor( void *pvParmeters )
{
	//TickType_t xLastWakeTime;
   // xLastWakeTime = xTaskGetTickCount();
    SDR_type_01h_t *pSDR = NULL;
    sensor_data_entry_t * pDATA;
    bool dc_dc_enabled = false;

	if (xSemaphoreTake(i2c_mutex_array[0].semaphore, (TickType_t)0) == pdTRUE) {
		// @todo: implement i2c muxing routines

		// switch bus
		Chip_I2C_Disable(i2c_mutex_array[0].i2c_bus);

		Chip_I2C_DeInit(i2c_mutex_array[0].i2c_bus);

		Chip_IOCON_PinMux(LPC_IOCON, 0, 0, IOCON_MODE_INACT, IOCON_FUNC3);
		Chip_IOCON_PinMux(LPC_IOCON, 0, 1, IOCON_MODE_INACT, IOCON_FUNC3);
		Chip_IOCON_EnableOD(LPC_IOCON, 0, 0);
		Chip_IOCON_EnableOD(LPC_IOCON, 0, 1);

		Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC0);
		Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC0);
		//Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
		//Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);

		Chip_I2C_Init(i2c_mutex_array[0].i2c_bus);
		Chip_I2C_Enable(i2c_mutex_array[0].i2c_bus);

		INA222_init(i2c_mutex_array[0].i2c_bus);

		Chip_I2C_Disable(i2c_mutex_array[0].i2c_bus);

		Chip_I2C_DeInit(i2c_mutex_array[0].i2c_bus);

		Chip_IOCON_PinMux(LPC_IOCON, 0, 0, IOCON_MODE_INACT, IOCON_FUNC0);
		Chip_IOCON_PinMux(LPC_IOCON, 0, 1, IOCON_MODE_INACT, IOCON_FUNC0);
		//Chip_IOCON_EnableOD(LPC_IOCON, 0,  0);
		//Chip_IOCON_EnableOD(LPC_IOCON, 0,  1);

		Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC3);
		Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC3);
		Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
		Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);
		Chip_I2C_Init(i2c_mutex_array[0].i2c_bus);
		Chip_I2C_Enable(i2c_mutex_array[0].i2c_bus);

		xSemaphoreGive(i2c_mutex_array[0].semaphore);
	} else {
		// fatal error
	}

    int i;
    for( ;; )
    {
        // @todo: move to payload task
    	if (xSemaphoreTake(semaphore_fru_control, SENSOR_DELAY_PERIOD) == pdTRUE) {
    		if (payload_ctrl_code == FRU_CTLCODE_QUIESCE) {
    			sensor_array[HOT_SWAP_SENSOR].data->comparator_status |= HOT_SWAP_STATE_QUIESCED;
    			setDC_DC_ConvertersON(false);

    		    struct ipmi_msg *pmsg = IPMI_alloc();
    		    struct ipmi_ipmb_addr *dst_addr =(struct ipmi_ipmb_addr *) &pmsg->daddr;
    		    struct ipmi_ipmb_addr *src_addr =(struct ipmi_ipmb_addr *) &pmsg->saddr;
    		    src_addr->lun = 0;
    		    src_addr->slave_addr = 0x76;
    		    dst_addr->lun = 0;
    		    dst_addr->slave_addr = 0x20;
    		    pmsg->msg.lun = 0;
    		    pmsg->msg.netfn = NETFN_SE;
    		    pmsg->msg.cmd = IPMI_PLATFORM_EVENT_CMD;
    		    int data_len = 0;
    		    pmsg->msg_data[data_len++] = 0x04;
    		    pmsg->msg_data[data_len++] = 0xf2;
    		    pmsg->msg_data[data_len++] = 0;
    		    pmsg->msg_data[data_len++] = 0x6f;
    		    pmsg->msg_data[data_len++] = HOT_SWAP_QUIESCED; // hot swap state
    		    pmsg->msg.data_len = data_len;
    		    IPMI_event_queue_append(pmsg);

    		}
    		continue;
    	}
    	/* @todo: add real delay checking */
    	if (xSemaphoreTake(i2c_mutex_array[0].semaphore, (TickType_t)100) == pdTRUE) {
    		// switch bus

			Chip_I2C_Disable(i2c_mutex_array[0].i2c_bus);

			Chip_I2C_DeInit(i2c_mutex_array[0].i2c_bus);

			Chip_IOCON_PinMux(LPC_IOCON, 0,  0, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_PinMux(LPC_IOCON, 0,  1, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_EnableOD(LPC_IOCON, 0,  0);
			Chip_IOCON_EnableOD(LPC_IOCON, 0,  1);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC0);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC0);
		//Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
		//Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);

			Chip_I2C_Init(i2c_mutex_array[0].i2c_bus);
			Chip_I2C_Enable(i2c_mutex_array[0].i2c_bus);


			// @todo: add sensor check loop
			// @todo: add timestamping??

			// HotSwap
			i=1;
			pSDR = (SDR_type_01h_t *) sensor_array[i].sdr;
			pDATA = sensor_array[i].data;
			if (pSDR->sensornum == HOT_SWAP_SENSOR) {
				//{ 2, 13, IOCON_MODE_INACT | IOCON_FUNC0 }, /* MOD HANDLE */
				bool tmp_val = Chip_GPIO_GetPinState(LPC_GPIO, 2, 13);
				uint8_t new_flag = 0;
				uint8_t old_flag = pDATA->comparator_status & 0x03;

				if (tmp_val) {
					// handle opened
					new_flag = HOT_SWAP_STATE_HANDLE_OPENED;
				} else {
					// handle closed
					new_flag = HOT_SWAP_STATE_HANDLE_CLOSED;
				}

				if (new_flag != old_flag) {
	    		    struct ipmi_msg *pmsg = IPMI_alloc();
	    		    struct ipmi_ipmb_addr *dst_addr =(struct ipmi_ipmb_addr *) &pmsg->daddr;
	    		    struct ipmi_ipmb_addr *src_addr =(struct ipmi_ipmb_addr *) &pmsg->saddr;
	    		    src_addr->lun = 0;
	    		    src_addr->slave_addr = 0x76;
	    		    dst_addr->lun = 0;
	    		    dst_addr->slave_addr = 0x20;
	    		    pmsg->msg.lun = 0;
	    		    pmsg->msg.netfn = NETFN_SE;
	    		    pmsg->msg.cmd = IPMI_PLATFORM_EVENT_CMD;
	    		    int data_len = 0;
	    		    pmsg->msg_data[data_len++] = 0x04;
	    		    pmsg->msg_data[data_len++] = 0xf2;
	    		    pmsg->msg_data[data_len++] = 0; // todo sensor numer
	    		    pmsg->msg_data[data_len++] = 0x6f;
	    		    pmsg->msg_data[data_len++] = (new_flag >> 1); // hot swap state
	    		    pmsg->msg.data_len = data_len;
	    		    IPMI_event_queue_append(pmsg);

	    		    pDATA->comparator_status = (pDATA->comparator_status & 0xFC) | new_flag;
				}


			}

			// 12v dcdc
			i = 2;
			pSDR = (SDR_type_01h_t *) sensor_array[i].sdr;
			pDATA = sensor_array[i].data;
			if (pSDR->sensornum == NUM_SDR_FMC_12V) {
				pDATA->readout_value = INA222_readVolt(i2c_mutex_array[0].i2c_bus, true) / 16;

				if (pDATA->readout_value > pSDR->lower_noncritical_thr &&
						dc_dc_enabled == false &&
						(~sensor_array[HOT_SWAP_SENSOR].data->comparator_status & HOT_SWAP_STATE_QUIESCED)) {

					setDC_DC_ConvertersON(true);
					dc_dc_enabled = true;

				} else if (pDATA->readout_value < pSDR->lower_critical_thr) {
					if (dc_dc_enabled == true) {
						setDC_DC_ConvertersON(false);
					}
					sensor_array[HOT_SWAP_SENSOR].data->comparator_status &= ~HOT_SWAP_STATE_QUIESCED;
					dc_dc_enabled = false;

				}
			}

			// restore bus;
			Chip_I2C_Disable(i2c_mutex_array[0].i2c_bus);

			Chip_I2C_DeInit(i2c_mutex_array[0].i2c_bus);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 0, IOCON_MODE_INACT, IOCON_FUNC0);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 1, IOCON_MODE_INACT, IOCON_FUNC0);
			//Chip_IOCON_EnableOD(LPC_IOCON, 0,  0);
			//Chip_IOCON_EnableOD(LPC_IOCON, 0,  1);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
			Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);
			Chip_I2C_Init(i2c_mutex_array[0].i2c_bus);
			Chip_I2C_Enable(i2c_mutex_array[0].i2c_bus);

			xSemaphoreGive(i2c_mutex_array[0].semaphore);
    	}


       // Board_LED_Toggle(2);
    }
}
#endif
