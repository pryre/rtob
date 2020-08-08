#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "io_type.h"

#include "fix16.h"
#include "fixextra.h"

// TODO: Double check
static mixer_t mixer_quadrotor_plus = {
	.mixer_ok = true,

	.output_type = {IO_TYPE_OM, IO_TYPE_OM, IO_TYPE_OM, IO_TYPE_OM,
					 IO_TYPE_N,  IO_TYPE_N,  IO_TYPE_N,  IO_TYPE_N}, // output_type

	//wx, wy, wz, vz
	.map = {.rows = 4, .columns = 4, .errors = 0,
			.data = {{-_fc_1,     0, _fc_1, _fc_1},
					 { _fc_1,     0, _fc_1, _fc_1},
					 {     0,-_fc_1,-_fc_1, _fc_1},
					 {     0, _fc_1,-_fc_1, _fc_1}}}
	/*
	{ _fc_1, _fc_1, _fc_1, _fc_1, 0, 0, 0, 0},  // F Mix
	{-_fc_1, _fc_1,     0,     0, 0, 0, 0, 0},		   // X Mix
	{     0,     0,-_fc_1, _fc_1, 0, 0, 0, 0},		   // Y Mix
	{ _fc_1, _fc_1,-_fc_1,-_fc_1, 0, 0, 0, 0} // Z Mix
	*/
};

#ifdef __cplusplus
}
#endif
