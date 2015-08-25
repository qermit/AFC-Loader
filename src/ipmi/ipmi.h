/*
 * ipmi.h
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


#ifndef IPMI_H_
#define IPMI_H_



#include <stdint.h>
#if USE_FREERTOS == 1
#include "FreeRTOS.h"
#endif

//#define IPMI_MSG_ADD_DEV_SUPP	0x29	// event receiver, accept sensor cmds
#define IPMI_MSG_ADD_DEV_SUPP	0x3B	// event receiver, accept sensor cmds
//#define IPMI_MSG_ADD_DEV_SUPP	0x01	// event receiver, accept sensor cmds
//#define IPMI_MSG_ADD_DEV_SUPP	0x09	// event receiver, accept sensor cmds
#define MMC_IPMI_REL		0x51	// V1.5

//#define IANA_MANUFACTURER_ID	0x000060	//CERN IANA (3 bytes)
#define IANA_MANUFACTURER_ID	0x000000	//CERN IANA (3 bytes)
//#define PRODUCT_ID				0x1235		//Product ID (2 bytes)
#define PRODUCT_ID				0x0000		//Product ID (2 bytes)
// firmware release
#define MMC_FW_REL_MAJ       1                  // major release, 7 bit
#define MMC_FW_REL_MIN       0x00                  // minor release, 8 bit

// manufacturer specific identifier codes
#define IPMI_MSG_MANU_ID_LSB	(IANA_MANUFACTURER_ID & 0x0000FF)
#define IPMI_MSG_MANU_ID_B2		((IANA_MANUFACTURER_ID & 0x00FF00) >> 8)
#define IPMI_MSG_MANU_ID_MSB	((IANA_MANUFACTURER_ID & 0xFF0000) >> 16)

#define IPMI_MSG_PROD_ID_LSB (PRODUCT_ID & 0x00FF)
#define IPMI_MSG_PROD_ID_MSB ((PRODUCT_ID & 0xFF00) >> 8)


// known netFn codes (even request codes only)
#define NETFN_CHASSIS						(0x00)
#define NETFN_BRIDGE						(0x02)
#define NETFN_SE							(0x04)		// sensor/event netFN
#define NETFN_APP							(0x06)		// application netFN
#define NETFN_FIRMWARE						(0x08)
#define NETFN_STORAGE						(0x0a)
#define NETFN_TRANSPORT						(0x0c)
#define NETFN_GRPEXT						(0x2c)		// group extension, for PICMG, first data byte = 0x00
#define NETFN_CUSTOM						(0x32)		// custom extension for UWHEP MMC functions
#define NETFN_CUSTOM_AFC					(0x30)		// custom extension for UWHEP MMC functions


// IPMI commands
// Chassis netfn (0x00)
#define IPMI_GET_CHASSIS_CAPABILITIES_CMD	                    0x00
#define IPMI_GET_CHASSIS_STATUS_CMD		                        0x01
#define IPMI_CHASSIS_CONTROL_CMD		                        0x02
#define IPMI_CHASSIS_RESET_CMD			                        0x03
#define IPMI_CHASSIS_IDENTIFY_CMD		                        0x04
#define IPMI_SET_CHASSIS_CAPABILITIES_CMD	                    0x05
#define IPMI_SET_POWER_RESTORE_POLICY_CMD	                    0x06
#define IPMI_GET_SYSTEM_RESTART_CAUSE_CMD	                    0x07
#define IPMI_SET_SYSTEM_BOOT_OPTIONS_CMD	                    0x08
#define IPMI_GET_SYSTEM_BOOT_OPTIONS_CMD	                    0x09
#define IPMI_GET_POH_COUNTER_CMD		                        0x0f

// Bridge netfn (0x00)
#define IPMI_GET_BRIDGE_STATE_CMD		                        0x00
#define IPMI_SET_BRIDGE_STATE_CMD		                        0x01
#define IPMI_GET_ICMB_ADDRESS_CMD		                        0x02
#define IPMI_SET_ICMB_ADDRESS_CMD		                        0x03
#define IPMI_SET_BRIDGE_PROXY_ADDRESS_CMD	                    0x04
#define IPMI_GET_BRIDGE_STATISTICS_CMD		                    0x05
#define IPMI_GET_ICMB_CAPABILITIES_CMD	                        0x06
#define IPMI_CLEAR_BRIDGE_STATISTICS_CMD	                    0x08
#define IPMI_GET_BRIDGE_PROXY_ADDRESS_CMD	                    0x09
#define IPMI_GET_ICMB_CONNECTOR_INFO_CMD	                    0x0a
#define IPMI_SET_ICMB_CONNECTOR_INFO_CMD	                    0x0b
#define IPMI_SEND_ICMB_CONNECTION_ID_CMD	                    0x0c
#define IPMI_PREPARE_FOR_DISCOVERY_CMD		                    0x10
#define IPMI_GET_ADDRESSES_CMD			                        0x11
#define IPMI_SET_DISCOVERED_CMD			                        0x12
#define IPMI_GET_CHASSIS_DEVICE_ID_CMD		                    0x13
#define IPMI_SET_CHASSIS_DEVICE_ID_CMD		                    0x14
#define IPMI_BRIDGE_REQUEST_CMD			                        0x20
#define IPMI_BRIDGE_MESSAGE_CMD			                        0x21
#define IPMI_GET_EVENT_COUNT_CMD		                        0x30
#define IPMI_SET_EVENT_DESTINATION_CMD		                    0x31
#define IPMI_SET_EVENT_RECEPTION_STATE_CMD	                    0x32
#define IPMI_SEND_ICMB_EVENT_MESSAGE_CMD	                    0x33
#define IPMI_GET_EVENT_DESTIATION_CMD		                    0x34
#define IPMI_GET_EVENT_RECEPTION_STATE_CMD	                    0x35
#define IPMI_ERROR_REPORT_CMD			                        0xff

// Sensor/Event netfn (0x04)
#define IPMI_SET_EVENT_RECEIVER_CMD		                        0x00
#define IPMI_GET_EVENT_RECEIVER_CMD		                        0x01
#define IPMI_PLATFORM_EVENT_CMD			                        0x02
#define IPMI_GET_PEF_CAPABILITIES_CMD		                    0x10
#define IPMI_ARM_PEF_POSTPONE_TIMER_CMD		                    0x11
#define IPMI_SET_PEF_CONFIG_PARMS_CMD		                    0x12
#define IPMI_GET_PEF_CONFIG_PARMS_CMD		                    0x13
#define IPMI_SET_LAST_PROCESSED_EVENT_ID_CMD	                0x14
#define IPMI_GET_LAST_PROCESSED_EVENT_ID_CMD	                0x15
#define IPMI_ALERT_IMMEDIATE_CMD		                        0x16
#define IPMI_PET_ACKNOWLEDGE_CMD		                        0x17
#define IPMI_GET_DEVICE_SDR_INFO_CMD		                    0x20
#define IPMI_GET_DEVICE_SDR_CMD			                        0x21
#define IPMI_RESERVE_DEVICE_SDR_REPOSITORY_CMD	                0x22
#define IPMI_GET_SENSOR_READING_FACTORS_CMD	                    0x23
#define IPMI_SET_SENSOR_HYSTERESIS_CMD		                    0x24
#define IPMI_GET_SENSOR_HYSTERESIS_CMD		                    0x25
#define IPMI_SET_SENSOR_THRESHOLD_CMD		                    0x26
#define IPMI_GET_SENSOR_THRESHOLD_CMD		                    0x27
#define IPMI_SET_SENSOR_EVENT_ENABLE_CMD	                    0x28
#define IPMI_GET_SENSOR_EVENT_ENABLE_CMD	                    0x29
#define IPMI_REARM_SENSOR_EVENTS_CMD		                    0x2a
#define IPMI_GET_SENSOR_EVENT_STATUS_CMD	                    0x2b
#define IPMI_GET_SENSOR_READING_CMD		                        0x2d
#define IPMI_SET_SENSOR_TYPE_CMD		                        0x2e
#define IPMI_GET_SENSOR_TYPE_CMD		                        0x2f

// App netfn (0x06)
#define IPMI_GET_DEVICE_ID_CMD			                        0x01
#define IPMI_BROADCAST_GET_DEVICE_ID_CMD	                    0x01
#define IPMI_COLD_RESET_CMD			                            0x02
#define IPMI_WARM_RESET_CMD			                            0x03
#define IPMI_GET_SELF_TEST_RESULTS_CMD		                    0x04
#define IPMI_MANUFACTURING_TEST_ON_CMD		                    0x05
#define IPMI_SET_ACPI_POWER_STATE_CMD		                    0x06
#define IPMI_GET_ACPI_POWER_STATE_CMD		                    0x07
#define IPMI_GET_DEVICE_GUID_CMD		                        0x08
#define IPMI_RESET_WATCHDOG_TIMER_CMD		                    0x22
#define IPMI_SET_WATCHDOG_TIMER_CMD		                        0x24
#define IPMI_GET_WATCHDOG_TIMER_CMD		                        0x25
#define IPMI_SET_BMC_GLOBAL_ENABLES_CMD		                    0x2e
#define IPMI_GET_BMC_GLOBAL_ENABLES_CMD	                	    0x2f
#define IPMI_CLEAR_MSG_FLAGS_CMD		                        0x30
#define IPMI_GET_MSG_FLAGS_CMD			                        0x31
#define IPMI_ENABLE_MESSAGE_CHANNEL_RCV_CMD	                    0x32
#define IPMI_GET_MSG_CMD			                            0x33
#define IPMI_SEND_MSG_CMD			                            0x34
#define IPMI_READ_EVENT_MSG_BUFFER_CMD		                    0x35
#define IPMI_GET_BT_INTERFACE_CAPABILITIES_CMD	                0x36
#define IPMI_GET_SYSTEM_GUID_CMD		                        0x37
#define IPMI_GET_CHANNEL_AUTH_CAPABILITIES_CMD                	0x38
#define IPMI_GET_SESSION_CHALLENGE_CMD	                	    0x39
#define IPMI_ACTIVATE_SESSION_CMD		                        0x3a
#define IPMI_SET_SESSION_PRIVILEGE_CMD	                  	    0x3b
#define IPMI_CLOSE_SESSION_CMD			                        0x3c
#define IPMI_GET_SESSION_INFO_CMD		                        0x3d
#define IPMI_GET_AUTHCODE_CMD			                        0x3f
#define IPMI_SET_CHANNEL_ACCESS_CMD		                        0x40
#define IPMI_GET_CHANNEL_ACCESS_CMD		                        0x41
#define IPMI_GET_CHANNEL_INFO_CMD		                        0x42
#define IPMI_SET_USER_ACCESS_CMD		                        0x43
#define IPMI_GET_USER_ACCESS_CMD		                        0x44
#define IPMI_SET_USER_NAME_CMD			                        0x45
#define IPMI_GET_USER_NAME_CMD			                        0x46
#define IPMI_SET_USER_PASSWORD_CMD		                        0x47
#define IPMI_ACTIVATE_PAYLOAD_CMD		                        0x48
#define IPMI_DEACTIVATE_PAYLOAD_CMD	                	        0x49
#define IPMI_GET_PAYLOAD_ACTIVATION_STATUS_CMD	                0x4a
#define IPMI_GET_PAYLOAD_INSTANCE_INFO_CMD	                    0x4b
#define IPMI_SET_USER_PAYLOAD_ACCESS_CMD	                    0x4c
#define IPMI_GET_USER_PAYLOAD_ACCESS_CMD	                    0x4d
#define IPMI_GET_CHANNEL_PAYLOAD_SUPPORT_CMD	                0x4e
#define IPMI_GET_CHANNEL_PAYLOAD_VERSION_CMD	                0x4f
#define IPMI_GET_CHANNEL_OEM_PAYLOAD_INFO_CMD	                0x50
#define IPMI_MASTER_READ_WRITE_CMD		                        0x52
#define IPMI_GET_CHANNEL_CIPHER_SUITES_CMD	                    0x54
#define IPMI_SUSPEND_RESUME_PAYLOAD_ENCRYPTION_CMD              0x55
#define IPMI_SET_CHANNEL_SECURITY_KEY_CMD	                    0x56
#define IPMI_GET_SYSTEM_INTERFACE_CAPABILITIES_CMD              0x57

// Storage netfn (0x0a)
#define IPMI_GET_FRU_INVENTORY_AREA_INFO_CMD	                0x10
#define IPMI_READ_FRU_DATA_CMD			                        0x11
#define IPMI_WRITE_FRU_DATA_CMD			                        0x12
#define IPMI_GET_SDR_REPOSITORY_INFO_CMD	                    0x20
#define IPMI_GET_SDR_REPOSITORY_ALLOC_INFO_CMD	                0x21
#define IPMI_RESERVE_SDR_REPOSITORY_CMD		                    0x22
#define IPMI_GET_SDR_CMD			                            0x23
#define IPMI_ADD_SDR_CMD			                            0x24
#define IPMI_PARTIAL_ADD_SDR_CMD		                        0x25
#define IPMI_DELETE_SDR_CMD			                            0x26
#define IPMI_CLEAR_SDR_REPOSITORY_CMD		                    0x27
#define IPMI_GET_SDR_REPOSITORY_TIME_CMD	                    0x28
#define IPMI_SET_SDR_REPOSITORY_TIME_CMD	                    0x29
#define IPMI_ENTER_SDR_REPOSITORY_UPDATE_CMD	                0x2a
#define IPMI_EXIT_SDR_REPOSITORY_UPDATE_CMD	                    0x2b
#define IPMI_RUN_INITIALIZATION_AGENT_CMD	                    0x2c
#define IPMI_GET_SEL_INFO_CMD			                        0x40
#define IPMI_GET_SEL_ALLOCATION_INFO_CMD   	                    0x41
#define IPMI_RESERVE_SEL_CMD			                        0x42
#define IPMI_GET_SEL_ENTRY_CMD			                        0x43
#define IPMI_ADD_SEL_ENTRY_CMD			                        0x44
#define IPMI_PARTIAL_ADD_SEL_ENTRY_CMD		                    0x45
#define IPMI_DELETE_SEL_ENTRY_CMD		                        0x46
#define IPMI_CLEAR_SEL_CMD			                            0x47
#define IPMI_GET_SEL_TIME_CMD			                        0x48
#define IPMI_SET_SEL_TIME_CMD			                        0x49
#define IPMI_GET_AUXILIARY_LOG_STATUS_CMD	                    0x5a
#define IPMI_SET_AUXILIARY_LOG_STATUS_CMD	                    0x5b

// Transport netfn (0x0c)
#define IPMI_SET_LAN_CONFIG_PARMS_CMD		                    0x01
#define IPMI_GET_LAN_CONFIG_PARMS_CMD		                    0x02
#define IPMI_SUSPEND_BMC_ARPS_CMD		                        0x03
#define IPMI_GET_IP_UDP_RMCP_STATS_CMD		                    0x04
#define IPMI_SET_SERIAL_MODEM_CONFIG_CMD                   	    0x10
#define IPMI_GET_SERIAL_MODEM_CONFIG_CMD	                    0x11
#define IPMI_SET_SERIAL_MODEM_MUX_CMD		                    0x12
#define IPMI_GET_TAP_RESPONSE_CODES_CMD		                    0x13
#define IPMI_SET_PPP_UDP_PROXY_XMIT_DATA_CMD	                0x14
#define IPMI_GET_PPP_UDP_PROXY_XMIT_DATA_CMD	                0x15
#define IPMI_SEND_PPP_UDP_PROXY_PACKET_CMD	                    0x16
#define IPMI_GET_PPP_UDP_PROXY_RECV_DATA_CMD	                0x17
#define IPMI_SERIAL_MODEM_CONN_ACTIVE_CMD	                    0x18
#define IPMI_CALLBACK_CMD			                            0x19
#define IPMI_SET_USER_CALLBACK_OPTIONS_CMD	                    0x1a
#define IPMI_GET_USER_CALLBACK_OPTIONS_CMD	                    0x1b
#define IPMI_SOL_ACTIVATING_CMD			                        0x20
#define IPMI_SET_SOL_CONFIGURATION_PARAMETERS	                0x21
#define IPMI_GET_SOL_CONFIGURATION_PARAMETERS	                0x22

// The Group Extension defined for PICMG.
#define IPMI_PICMG_GRP_EXT		                                0

// PICMG Commands (0x2c)
#define IPMI_PICMG_CMD_GET_PROPERTIES							0x00
#define IPMI_PICMG_CMD_GET_ADDRESS_INFO							0x01
#define IPMI_PICMG_CMD_GET_SHELF_ADDRESS_INFO					0x02
#define IPMI_PICMG_CMD_SET_SHELF_ADDRESS_INFO					0x03
#define IPMI_PICMG_CMD_FRU_CONTROL								0x04
#define IPMI_PICMG_CMD_GET_FRU_LED_PROPERTIES					0x05
#define IPMI_PICMG_CMD_GET_LED_COLOR_CAPABILITIES				0x06
#define IPMI_PICMG_CMD_SET_FRU_LED_STATE						0x07
#define IPMI_PICMG_CMD_GET_FRU_LED_STATE						0x08
#define IPMI_PICMG_CMD_SET_IPMB_STATE							0x09
#define IPMI_PICMG_CMD_SET_FRU_ACTIVATION_POLICY				0x0a
#define IPMI_PICMG_CMD_GET_FRU_ACTIVATION_POLICY				0x0b
#define IPMI_PICMG_CMD_SET_FRU_ACTIVATION						0x0c
#define IPMI_PICMG_CMD_GET_DEVICE_LOCATOR_RECORD				0x0d
#define IPMI_PICMG_CMD_SET_PORT_STATE							0x0e
#define IPMI_PICMG_CMD_GET_PORT_STATE							0x0f
#define IPMI_PICMG_CMD_COMPUTE_POWER_PROPERTIES					0x10
#define IPMI_PICMG_CMD_SET_POWER_LEVEL							0x11
#define IPMI_PICMG_CMD_GET_POWER_LEVEL							0x12
#define IPMI_PICMG_CMD_RENEGOTIATE_POWER						0x13
#define IPMI_PICMG_CMD_GET_FAN_SPEED_PROPERTIES					0x14
#define IPMI_PICMG_CMD_SET_FAN_LEVEL							0x15
#define IPMI_PICMG_CMD_GET_FAN_LEVEL							0x16
#define IPMI_PICMG_CMD_BUSED_RESOURCE							0x17
#define IPMI_PICMG_CMD_IPMB_LINK_INFO							0x18
#define IPMI_PICMG_CMD_SET_AMC_PORT_STATE               		0x19
#define IPMI_PICMG_CMD_GET_AMC_PORT_STATE               		0x1a
#define IPMI_PICMG_CMD_SHELF_MANAGER_IPMB_ADDRESS				0x1b
#define IPMI_PICMG_CMD_SET_FAN_POLICY							0x1c
#define IPMI_PICMG_CMD_GET_FAN_POLICY							0x1d
#define IPMI_PICMG_CMD_FRU_CONTROL_CAPABILITIES					0x1e
#define IPMI_PICMG_CMD_FRU_INVENTORY_DEVICE_LOCK_CONTROL		0x1f
#define IPMI_PICMG_CMD_FRU_INVENTORY_DEVICE_WRITE				0x20
#define IPMI_PICMG_CMD_GET_SHELF_MANAGER_IP_ADDRESSES			0x21
#define IPMI_PICMG_CMD_SHELF_POWER_ALLOCATION           		0x22
#define IPMI_PICMG_CMD_GET_TELCO_ALARM_CAPABILITY				0x29

#define IPMI_CC_OK                                              0x00
#define IPMI_CC_NODE_BUSY                                       0xc0
#define IPMI_CC_INV_CMD                                         0xc1
#define IPMI_CC_INV_CMD_FOR_LUN                                 0xc2
#define IPMI_CC_TIMEOUT                                         0xc3
#define IPMI_CC_OUT_OF_SPACE                                    0xc4
#define IPMI_CC_RES_CANCELED                                    0xc5
#define IPMI_CC_REQ_DATA_TRUNC                                  0xc6
#define IPMI_CC_REQ_DATA_INV_LENGTH                             0xc7
#define IPMI_CC_REQ_DATA_FIELD_EXCEED                           0xc8
#define IPMI_CC_PARAM_OUT_OF_RANGE                              0xc9
#define IPMI_CC_CANT_RET_NUM_REQ_BYTES                          0xca
#define IPMI_CC_REQ_DATA_NOT_PRESENT                            0xcb
#define IPMI_CC_INV_DATA_FIELD_IN_REQ                           0xcc
#define IPMI_CC_ILL_SENSOR_OR_RECORD                            0xcd
#define IPMI_CC_RESP_COULD_NOT_BE_PRV                           0xce
#define IPMI_CC_CANT_RESP_DUPLI_REQ                             0xcf
#define IPMI_CC_CANT_RESP_SDRR_UPDATE                           0xd0
#define IPMI_CC_CANT_RESP_FIRM_UPDATE                           0xd1
#define IPMI_CC_CANT_RESP_BMC_INIT                              0xd2
#define IPMI_CC_DESTINATION_UNAVAILABLE                         0xd3
#define IPMI_CC_INSUFFICIENT_PRIVILEGES                         0xd4
#define IPMI_CC_NOT_SUPPORTED_PRESENT_STATE                     0xd5
#define IPMI_CC_ILLEGAL_COMMAND_DISABLED                        0xd6
#define IPMI_CC_UNSPECIFIED_ERROR                               0xff

#define IPMI_MAX_ADDR_SIZE 2
#define IPMI_MAX_MSG_LENGTH 24


struct ipmi_addr
{
	int   addr_type;
	short channel;
	char  data[IPMI_MAX_ADDR_SIZE];
};

struct ipmi_ipmb_addr
{
	int           addr_type;
	short         channel;
	unsigned char slave_addr;
	unsigned char lun;
};

struct ipmi_system_interface_addr
{
	int   addr_type;
	short channel;
};

struct ipmi_msg_data
{
	unsigned char netfn;
	unsigned char lun;
	unsigned char cmd;
	uint8_t *data; /* pointer to a msg_data */
	int           data_len;
};


struct ipmi_msg
{
	struct ipmi_addr saddr;
	struct ipmi_addr daddr;
	unsigned char sequence;
	struct ipmi_msg_data  msg;
	uint8_t   msg_data[IPMI_MAX_MSG_LENGTH];
	int retries_left;
	unsigned char retcode;
};


typedef void (*ipmiProcessFunc)(struct ipmi_msg *req, struct ipmi_msg* rsp);

typedef struct ipmiFuncEntry {
	unsigned char netfn;
	unsigned char cmd;
	ipmiProcessFunc process;
} ipmiFuncEntry_t;


typedef struct __attribute__((__packed__)) fru_common_header {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	uint8_t	:4,			/* Common Header Format Version
					   7:4 - reserved, write as 0000b */
		format_version:4;	/* 3:0 - format version number = 1h
					   for this specification. */
#else
	uint8_t	format_version:4,
		:4;
#endif
	uint8_t	int_use_offset;		/* Internal Use Area Starting Offset
					   (in multiples of 8 bytes). 00h
					   indicates that this area is not
					   present. */
	uint8_t	chassis_info_offset;	/* Chassis Info Area Starting
					   Offset (in multiples of 8 bytes). 00h
					   indicates that this area is not
					   present. */
	uint8_t	board_offset;		/* Board Area Starting Offset (in
					   multiples of 8 bytes). 00h indicates
					   that this area is not present. */
	uint8_t	product_info_offset;	/* Product Info Area Starting
					   Offset (in multiples of 8 bytes).
					   00h indicates that this area is not
					   present. */
	uint8_t	multirecord_offset;	/* MultiRecord Area Starting Offset
					   (in multiples of 8 bytes). 00h
					   indicates that this area is not
					   present. */
	uint8_t	pad;			/* PAD, write as 00h */
	uint8_t	checksum;		/* Common Header Checksum (zero checksum) */
} fru_common_header_t;


/* AMC Table 3-16 AdvancedMC Point-to-Point Connectivity record */
typedef struct __attribute__((__packed__)) amc_p2p_conn_record {
	uint8_t	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	uint8_t eol:1,		/* [7:7] End of list. Set to one for the last record */
          reserved:3,	/* [6:4] Reserved, write as 0h.*/
          version:4;	/* [3:0] record format version (2h for this definition) */
#else
	uint8_t	version:4,
		reserved:3,
		eol:1;
#endif
	uint8_t	record_len;	/* Record Length. # of bytes following rec cksum */
	uint8_t	record_cksum;	/* Record Checksum. Holds the zero checksum of
				   the record. */
	uint8_t	header_cksum;	/* Header Checksum. Holds the zero checksum of
				   the header. */
	uint8_t	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uint8_t	picmg_rec_id;	/* PICMG Record ID. For the AMC Point-to-Point
				   Connectivity record, the value 19h must be used  */
	uint8_t	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uint8_t	oem_guid_count;	/* OEM GUID Count. The number, n, of OEM GUIDs
				   defined in this record. */
//TODO	OEM_GUID oem_guid_list[n];
				/* A list 16*n bytes of OEM GUIDs. */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	uint8_t	record_type:1,	/* [7] Record Type – 1 AMC Module, 0 On-Carrier device */
		:3,		/* [6:4] Reserved; write as 0h */
		conn_dev_id:4;	/* [3:0] Connected-device ID if Record Type = 0,
				   Reserved, otherwise. */
#else
	uint8_t	conn_dev_id:4,
		:3,
		record_type:1;
#endif
	uint8_t	ch_descr_count;	/* AMC Channel Descriptor Count. The number, m,
				   of AMC Channel Descriptors defined in this record. */
//TODO	AMC_CHANNEL_DESCR ch_descr[m];
				/* AMC Channel Descriptors. A variable length
				   list of m three-byte AMC Channel Descriptors,
				   each defining the Ports that make up an AMC
				   Channel (least significant byte first).*/
//TODO	AMC_LINK_DESCR link_desrc[p];
				/* AMC Link Descriptors. A variable length list
				   of p five-byte AMC Link Descriptors (Least
				   significant byte first) (see Table 3-19, “AMC
				   Link Descriptor”, Table 3-20, “AMC Link Designator”,
				   and Table 3-21, “AMC Link Type”) totaling 5 * p
				   bytes in length. The value of p and the length
				   of the list are implied by Record Length, since
				   the list is at the end of this record.
				   Each AMC Link Descriptor details one type of
				   point-to-point protocol supported by the
				   referenced Ports. */
} amc_p2p_conn_record_t;

/* Table 3-10 Module Current Requirements record */
typedef struct __attribute__((__packed__)) module_current_requirements_record {
	uint8_t	rec_type_id;	/* Record Type ID. For all records
				   defined in this specification,
				   a value of C0h (OEM) must be used. */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	uint8_t	end_list:1,	/* [7] – End of List. Set to one for
				   the last record */
		:3,		/* [6:4] – Reserved, write as 0h */
		rec_format:4;	/* [3:0] – Record format version
				   (= 2h for this definition) */
#else
	uint8_t	rec_format:4,
			:3,
			end_list:1;
#endif
	uint8_t	rec_length;	/* Record Length */
	uint8_t	rec_cksum;	/* Record Checksum. Holds the zero
				   checksum of the record. */
	uint8_t	hdr_cksum;	/* Header Checksum. Holds the zero
				   checksum of the header. */
	uint8_t	manuf_id_lsb;	/* Manufacturer ID. Least significant
				   byte first. Write as the three byte ID
				   assigned to PICMG. For this specification
				   the value 12634 (00315Ah) must be used. */
	uint8_t	manuf_id_midb;
	uint8_t	manuf_id_msb;
	uint8_t	picmg_rec_id;	/* PICMG Record ID. For the Module Power Descriptor
				   table, the value 16h must be used. */
	uint8_t	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h must be used. */
	uint8_t	curr_draw;	/* Current Draw. This field holds the Payload
				   Power (PWR) requirement of the Module given
				   as current requirement in units of 0.1A at 12V.
				   (This equals the value of the power in W
				   divided by 1.2.) */
} module_current_requirements_record_t;

typedef struct __attribute__((__packed__)) multirecord_area_header {
	uint8_t	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	uint8_t 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (2h for this definition) */
#else
	uint8_t	version:4,
		reserved:3,
		eol:1;
#endif
	uint8_t	record_len;	/* Record Length. */
	uint8_t	record_cksum;	/* Record Checksum. Holds the zero checksum of
				   the record. */
	uint8_t	header_cksum;	/* Header Checksum. Holds the zero checksum of
				   the header. */
	uint8_t	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uint8_t	picmg_rec_id;	/* PICMG Record ID. */
	uint8_t	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
} multirecord_area_header_t;


void IPMI_check_req();
void IPMI_put_event_response(struct ipmi_msg * p_ipmi_req) ;
void vTaskIPMI( void *pvParmeters );

void IPMI_init();

#ifdef FREERTOS_CONFIG_H
struct ipmi_msg * IPMI_alloc_fromISR();
void IPMI_free_fromISR(struct ipmi_msg * msg);
int IPMI_req_queue_append_fromISR(struct ipmi_msg * msg);
#else
#define IPMI_alloc_fromISR() IPMI_alloc();
#define IPMI_free_fromISR(_msg) IPMI_free(_msg)
#define IPMI_req_queue_append_fromISR(_msg) IPMI_req_queue_append(_msg)
#endif


struct ipmi_msg * IPMI_alloc();
void IPMI_free(struct ipmi_msg * msg);
int IPMI_req_queue_append(struct ipmi_msg * msg);
int IPMI_event_queue_append(struct ipmi_msg * msg);

void IPMI_req_queue_pushback(struct ipmi_msg * msg);
struct ipmi_msg * IPMI_req_queue_get();
TickType_t getTickDifference(TickType_t current_time, TickType_t start_time) ;

void IPMI_evet_set_address(struct ipmi_ipmb_addr * src, struct ipmi_ipmb_addr * dst);
void IPMI_evet_get_address(struct ipmi_ipmb_addr * src, struct ipmi_ipmb_addr * dst);

#endif /* IPMI_H_ */
