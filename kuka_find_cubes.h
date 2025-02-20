#ifndef CUSTOM_PDO_NAME_H
#define CUSTOM_PDO_NAME_H

//-------------------------------------------------------------------//
//                                                                   //
//     This file has been created by the Easy Configurator tool      //
//                                                                   //
//     Easy Configurator project kuka_find_cubes.prj
//                                                                   //
//-------------------------------------------------------------------//


#define CUST_BYTE_NUM_OUT	0
#define CUST_BYTE_NUM_IN	3
#define TOT_BYTE_NUM_ROUND_OUT	0
#define TOT_BYTE_NUM_ROUND_IN	4


typedef union												//---- output buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_OUT];
	struct
	{
	}Cust;
} PROCBUFFER_OUT;


typedef union												//---- input buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_IN];
	struct
	{
		uint8_t     x;
		uint8_t     y;
		uint8_t     index;
	}Cust;
} PROCBUFFER_IN;

#endif
