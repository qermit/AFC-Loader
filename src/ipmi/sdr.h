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

ipmiProcessFunc ipmi_se_get_sdr_info(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_se_get_sdr(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_se_get_sensor_reading(struct ipmi_msg *req, struct ipmi_msg* rsp);



ipmiProcessFunc ipmi_se_reserve_device_sdr(struct ipmi_msg *req, struct ipmi_msg* rsp);


#endif /* IPMI_SDR_H_ */
