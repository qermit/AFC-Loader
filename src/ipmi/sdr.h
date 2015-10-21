/*
 *   sdr.h
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


#ifndef IPMI_SDR_H_
#define IPMI_SDR_H_

#include "ipmb.h"




void sdr_init(uint8_t ipmiID);

typedef struct {
  uint8_t recID_LSB;
  uint8_t recID_MSB;
  uint8_t SDRversion;
  uint8_t rectype;
  uint8_t reclength;
} SDR_entry_hdr_t;

typedef struct {
  SDR_entry_hdr_t hdr;
  uint8_t ownerID;                                                              // 6
  uint8_t ownerLUN;                                                             // 7
  uint8_t sensornum;                                                    // 8
  uint8_t entityID;                                                             // 9
  uint8_t entityinstance;                                               // 10
  uint8_t sensorinit;                                                   // 11
  uint8_t sensorcap;                                                    // 12
  uint8_t sensortype;                                                   // 13
  uint8_t event_reading_type;                                   // 14
  uint8_t assertion_event_mask[2];              // 15-16 <- to jest zmieniane
  uint8_t deassertion_event_mask[2];            // 17-18 <- to jest zmieniane
  uint8_t readable_threshold_mask;              // 19
  uint8_t settable_threshold_mask;              // 20
  uint8_t sensor_units_1;                                               // 21
  uint8_t sensor_units_2;                                               // 22
  uint8_t sensor_units_3;                                               // 23
  uint8_t linearization;                                                // 24
  uint8_t M;                                                                    // 25
  uint8_t M_tol;                                                                // 26
  uint8_t B;                                                                    // 27
  uint8_t B_accuracy;                                                   // 28
  uint8_t acc_exp_sensor_dir;                                   // 29
  uint8_t Rexp_Bexp;                                                    // 30
  uint8_t analog_flags;                                                 // 31
  uint8_t nominal_reading;                                              // 32
  uint8_t normal_max;                                                   // 33
  uint8_t normal_min;                                                   // 34
  uint8_t sensor_max_reading;                                   // 35
  uint8_t sensor_min_reading;                                   // 36
  uint8_t upper_nonrecover_thr;                                 // 37
  uint8_t upper_critical_thr;                                   // 38
  uint8_t upper_noncritical_thr;                                // 39
  uint8_t lower_nonrecover_thr;                                 // 40
  uint8_t lower_critical_thr;                                   // 41
  uint8_t lower_noncritical_thr;                                // 42
  uint8_t pos_thr_hysteresis;                                   // 43
  uint8_t neg_thr_hysteresis;                                   // 44
  uint8_t reserved1;                                                    // 45
  uint8_t reserved2;                                                    // 46
  uint8_t OEM;                                                                  // 47
  uint8_t IDtypelen;                                                    // 48
  char IDstring[16];                                                    // 49-64 (0x40 length max)
} SDR_type_01h_t;


typedef struct {
  SDR_entry_hdr_t hdr;
  // RECORD KEY BYTES
  uint8_t ownerID;                                                              // 6
  uint8_t ownerLUN;                                                             // 7
  uint8_t sensornum;                                                    // 8
  // record body bytes
  uint8_t entityID;                                                             // 9
  uint8_t entityinstance;                                               // 10
  uint8_t sensorinit;                                                   // 11
  uint8_t sensorcap;                                                    // 12
  uint8_t sensortype;                                                   // 13
  uint8_t event_reading_type;                                   // 14
  uint8_t assertion_event_mask[2];              // 15-16 <- to jest zmieniane
  uint8_t deassertion_event_mask[2];            // 17-18 <- to jest zmieniane
  uint8_t readable_threshold_mask;              // 19
  uint8_t settable_threshold_mask;              // 20
  uint8_t sensor_units_1;                                               // 21
  uint8_t sensor_units_2;                                               // 22
  uint8_t sensor_units_3;                                               // 23
  uint8_t record_sharing[2];                                                // 24-25
  uint8_t pos_thr_hysteresis;                                               // 25
  uint8_t neg_thr_hysteresis;                                               // 26
  uint8_t reserved1;                                                                    // 27
  uint8_t reserved2;                                                   // 28
  uint8_t reserved3;                                   // 29
  uint8_t OEM;                                                    // 30
  uint8_t IDtypelen;                                                    // 31
  char IDstring[16];                                                    // 32-64 (0x40 length max)
} SDR_type_02h_t;

typedef struct {
  SDR_entry_hdr_t hdr;
  uint8_t slaveaddr;
  uint8_t chnum;
  uint8_t power_notification_global_init;
  uint8_t device_cap;
  uint8_t reserved[3];
  uint8_t entityID;
  uint8_t entityinstance;
  uint8_t OEM;
  uint8_t IDtypelen;
  char IDstring[16];
} SDR_type_12h_t;


typedef struct {
  uint8_t ownerID;
  uint8_t entityID;
  uint8_t entityinstance;

  uint8_t readout_value;
  uint8_t comparator_status;

  /*uint8_t event_msg_ctl;

  uint16_t cur_masked_comp;
  uint16_t prev_masked_comp;

  uint8_t comparator_status;              // for IPMI comparator readout for get sensor reading command
  uint8_t readout_value;

  pGetReadoutVal readout_function;
  uint8_t readout_func_arg;

  uint8_t active_context_code;            // context code for when sensor is active*/

} sensor_data_entry_t;

//void ipmi_se_get_sdr_info(struct ipmi_msg *req, struct ipmi_msg* rsp);
//void ipmi_se_get_sdr(struct ipmi_msg *req, struct ipmi_msg* rsp);
//void ipmi_se_get_sensor_reading(struct ipmi_msg *req, struct ipmi_msg* rsp);



//void ipmi_se_reserve_device_sdr(struct ipmi_msg *req, struct ipmi_msg* rsp);

void initializeDCDC();

void do_quiesced_init();

void do_quiesced(unsigned char ctlcode);


void vTaskSensor( void *pvParmeters );

#endif /* IPMI_SDR_H_ */
