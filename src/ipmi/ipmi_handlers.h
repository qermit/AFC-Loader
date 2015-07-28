/*
 * ipmi_handlers.h
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

#ifndef IPMI_IPMI_HANDLERS_H_
#define IPMI_IPMI_HANDLERS_H_
#include "ipmi.h"

ipmiProcessFunc ipmi_get_device_id(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_picmg_get_PROPERTIES(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_picmg_cmd_fru_control(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_picmg_set_fru_led_state(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_picmg_get_device_locator_record(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_se_set_event_reciever(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_storage_get_fru_info(struct ipmi_msg *req, struct ipmi_msg* rsp);
ipmiProcessFunc ipmi_storage_read_fru_data_cmd(struct ipmi_msg *req, struct ipmi_msg* rsp);

ipmiProcessFunc ipmi_general_invalid(struct ipmi_msg *req, struct ipmi_msg* rsp) ;


#endif /* IPMI_IPMI_HANDLERS_H_ */
