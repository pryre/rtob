#pragma once

#include "fix16.h"
#include "fixvector3d.h"
#include "fixmatrix.h"
#include "fixquat.h"

//Quaternion layout
//q.a -> q.w
//q.b -> q.x
//q.c -> q.y
//q.d -> q.z

//Quick defines of reused fix16 numbers
#define _fc_0_01		0x0000028F	//0.01
#define _fc_0_083_		0x00001555	//0.0833...
#define _fc_0_46_		0x00006AAA	//0.466...
#define _fc_0_5			0x00008000	//0.5
#define _fc_0_6_		0x0000AAAA	//0.66...
#define _fc_0_85		0x0000D999	//0.85
#define _fc_1			0x00010000	//1.0
#define _fc_1_15		0x00012666	//1.15
#define _fc_2			0x00020000	//2.0
#define _fc_3_3			0x00034CCC	//3.3
#define _fc_10			0x000A0000	//10
#define _fc_11			0x000B0000	//11
#define _fc_100			0x00640000	//100
#define _fc_1000		0x03E80000	//1000

#define _fc_sqrt_0_5	0x0000B504	//0.70710754395 (slightly higher than sqrt 0.5)
#define _fc_epsilon		0x0000FFFF	//0.99...
#define _fc_gravity		0x0009CE80	//Is equal to 9.80665 (Positive!) in Q16.16

typedef enum {
	AXIS_LOCK_X,
	AXIS_LOCK_Y,
	AXIS_LOCK_Z
} qf16_axis_lock_t;

static const qf16 NED_ENU_Q = {0, _fc_sqrt_0_5, -_fc_sqrt_0_5, 0};
//static const qf16 NED_IMU_Q = {0, _fc_1, 0, 0};

static inline fix16_t fix16_constrain(fix16_t i, const fix16_t min, const fix16_t max) {
	return (i < min) ? min : (i > max) ? max : i;
}

static inline fix16_t v3d_sq_norm(const v3d *a) {
	return fix16_add(fix16_add(fix16_sq(a->x), fix16_sq(a->y)), fix16_sq(a->z));
}

static inline void qf16_inverse(qf16 *dest, const qf16 *q) {
	qf16 q_temp;

	//q_dot = conjugate(q)/norm(q)
	qf16_conj(&q_temp, q);
	qf16_div_s(dest, &q_temp, qf16_norm(q));
}

static inline void qf16_normalize_to_unit(qf16 *dest, const qf16 *q) {
	fix16_t d = fix16_sqrt( fix16_add(fix16_sq(q->a),
							fix16_add(fix16_sq(q->b),
							fix16_add(fix16_sq(q->c), fix16_sq(q->d)))));

	dest->a = fix16_div(q->a, d);
	dest->b = fix16_div(q->b, d);
	dest->c = fix16_div(q->c, d);
	dest->d = fix16_div(q->d, d);
}

//Returns the rotation between two vectors
static inline void qf16_from_shortest_path(qf16 *dest, const v3d *v1, const v3d *v2) {
	fix16_t v_dot = v3d_dot(v1, v2);

	//Check to see if they are parallel
	if(v_dot > _fc_epsilon) {	//The vectors are parallel
		//Identity quaternion
		dest->a = 1;
		dest->b = 0;
		dest->c = 0;
		dest->d = 0;
	} else if(v_dot < -_fc_epsilon) {	//The vectors are opposite
		//180Deg Roll Quaternion
		dest->a = 0;
		dest->b = 1;
		dest->c = 0;
		dest->d = 0;
	} else {	//The vectors aren't parallel
		qf16 q;

		//Calculate the rotation
		v3d v_c;
		v3d_cross(&v_c, v1, v2);

		//q.w = sqrt((v1.length ^ 2) * (v2.length ^ 2)) + dotproduct(v1, v2)
		//q.a = fix16_add(fix16_sqrt(fix16_mul(fix16_sq(v3d_norm(v1)), fix16_sq(v3d_norm(v2)))), v_dot);

		q.a = fix16_add(_fc_1, v_dot);
		q.b = v_c.x;
		q.c = v_c.y;
		q.d = v_c.z;

		qf16_normalize(dest, &q);
	}
}

//TODO: Should use the mavlink conversions as a base if we move to float
static inline void euler_from_quat(qf16 *q, fix16_t *phi, fix16_t *theta, fix16_t *psi) {
  *phi = fix16_atan2(fix16_mul(_fc_2, fix16_add(fix16_mul(q->a, q->b), fix16_mul(q->c, q->d))),
                      fix16_sub(_fc_1, fix16_mul(_fc_2, fix16_add(fix16_mul(q->b, q->b), fix16_mul(q->c, q->c)))));

  *theta = fix16_asin(fix16_mul(_fc_2, fix16_sub(fix16_mul(q->a, q->c), fix16_mul(q->d, q->b))));

  *psi = fix16_atan2(fix16_mul(_fc_2, fix16_add(fix16_mul(q->a, q->d), fix16_mul(q->b, q->c))),
                     fix16_sub(_fc_1, fix16_mul(_fc_2, fix16_add(fix16_mul(q->c, q->c), fix16_mul(q->d, q->d)))));
}

static inline void quat_from_euler(qf16 *q, fix16_t phi, fix16_t theta, fix16_t psi) {
	fix16_t t0 = fix16_cos(fix16_mul(psi, _fc_0_5));
	fix16_t t1 = fix16_sin(fix16_mul(psi, _fc_0_5));
	fix16_t t2 = fix16_cos(fix16_mul(phi, _fc_0_5));
	fix16_t t3 = fix16_sin(fix16_mul(phi, _fc_0_5));
	fix16_t t4 = fix16_cos(fix16_mul(theta, _fc_0_5));
	fix16_t t5 = fix16_sin(fix16_mul(theta, _fc_0_5));

	q->a = fix16_add(fix16_mul(t0, fix16_mul(t2, t4)), fix16_mul(t1, fix16_mul(t3, t5)));
	q->b = fix16_sub(fix16_mul(t0, fix16_mul(t3, t4)), fix16_mul(t1, fix16_mul(t2, t5)));
	q->c = fix16_add(fix16_mul(t0, fix16_mul(t2, t5)), fix16_mul(t1, fix16_mul(t3, t4)));
	q->d = fix16_sub(fix16_mul(t1, fix16_mul(t2, t4)), fix16_mul(t0, fix16_mul(t3, t5)));
}

static inline void matrix_to_qf16(qf16 *dest, const mf16 *mat) {
	//Method pulled from the ROS tf2 library
	fix16_t trace = fix16_add(mat->data[0][0], fix16_add(mat->data[1][1], mat->data[2][2]));
	fix16_t temp[4];

	if( trace > 0 ) {
		fix16_t s = fix16_sqrt(fix16_add(trace, _fc_1));
		temp[3] = fix16_mul(s, _fc_0_5);
		s = fix16_div(_fc_0_5, s);

		temp[0] = fix16_mul(fix16_sub(mat->data[2][1], mat->data[1][2]), s);
		temp[1] = fix16_mul(fix16_sub(mat->data[0][2], mat->data[2][0]), s);
		temp[2] = fix16_mul(fix16_sub(mat->data[1][0], mat->data[0][1]), s);
	} else {
		int i = mat->data[0][0] < mat->data[1][1] ?
			(mat->data[1][1] < mat->data[2][2] ? 2 : 1) :
			(mat->data[0][0] < mat->data[2][2] ? 2 : 0);
		int j = (i + 1) % 3;
		int k = (i + 2) % 3;

		fix16_t s = fix16_sqrt(fix16_sub(mat->data[i][i], fix16_sub(mat->data[j][j], fix16_add(mat->data[k][k], _fc_1))));
		temp[i] = fix16_mul(s, _fc_0_5);
		s = fix16_div(_fc_0_5, s);

		temp[3] = fix16_mul(fix16_sub(mat->data[k][j], mat->data[j][k]), s);
		temp[j] = fix16_mul(fix16_add(mat->data[j][i], mat->data[i][j]), s);
		temp[k] = fix16_mul(fix16_add(mat->data[k][i], mat->data[i][k]), s);
	}

	dest->a = temp[3];
	dest->b = temp[0];
	dest->c = temp[1];
	dest->d = temp[2];
}

static inline void dcm_to_basis(v3d *b_x, v3d *b_y, v3d *b_z, const mf16 *dcm) {
		b_x->x = dcm->data[0][0];
		b_x->y = dcm->data[0][1];
		b_x->z = dcm->data[0][2];

		b_y->x = dcm->data[1][0];
		b_y->y = dcm->data[1][1];
		b_y->z = dcm->data[1][2];

		b_z->x = dcm->data[2][0];
		b_z->y = dcm->data[2][1];
		b_z->z = dcm->data[2][2];
}

static inline void dcm_from_basis(mf16 *dcm, const v3d *b_x, const v3d *b_y, const v3d *b_z) {
		dcm->data[0][0] = b_x->x;
		dcm->data[0][1] = b_x->y;
		dcm->data[0][2] = b_x->z;

		dcm->data[1][0] = b_y->x;
		dcm->data[1][1] = b_y->y;
		dcm->data[1][2] = b_y->z;

		dcm->data[2][0] = b_z->x;
		dcm->data[2][1] = b_z->y;
		dcm->data[2][2] = b_z->z;
}

static inline void qf16_align_to_axis(qf16 *dest, const qf16 *input, const qf16 *reference, const qf16_axis_lock_t axis) {
	qf16 q_temp;
	qf16 dq;

	qf16_inverse(&q_temp, input);
	qf16_mul(&dq, reference, &q_temp);	//Difference between reference and input
	qf16_normalize_to_unit(&dq, &dq);

	v3d fv;	//Flat vector

	fv.x = _fc_1;	//Set to directly forward, no rotation
	fv.y = 0;
	fv.z = 0;

	qf16_rotate(&fv, &dq, &fv);	//Rotate the flat vector by the difference in rotation

	v3d rot_axis;	//Axis of rotation
	fix16_t rot_a = 0;	//Rotation angle

	switch(axis) {
		case AXIS_LOCK_X: {
			fv.x = 0;
			v3d_normalize(&fv, &fv);	//Re-flatten, and normalize

			rot_a = fix16_atan2(fv.z, fv.y);	//Get the angular difference

			rot_axis.x = _fc_1;
			rot_axis.y = 0;
			rot_axis.z = 0;

			break;
		}
		case AXIS_LOCK_Y: {
			fv.y = 0;
			v3d_normalize(&fv, &fv);	//Re-flatten, and normalize

			rot_a = fix16_atan2(fv.x, fv.z);	//Get the angular difference

			rot_axis.x = 0;
			rot_axis.y = _fc_1;
			rot_axis.z = 0;

			break;
		}
		case AXIS_LOCK_Z: {
			fv.z = 0;
			v3d_normalize(&fv, &fv);	//Re-flatten, and normalize

			rot_a = fix16_atan2(fv.y, fv.x);	//Get the angular difference

			rot_axis.x = 0;
			rot_axis.y = 0;
			rot_axis.z = _fc_1;

			break;
		}
		default: {
			rot_a = 0;

			rot_axis.x = _fc_1;
			rot_axis.y = 0;
			rot_axis.z = 0;
		}
	}

	qf16 q_rot;
	qf16_from_axis_angle(&q_rot, &rot_axis, rot_a);	//Create a rotation quaternion for the flat rotation the axis
	qf16_mul(dest, &q_rot, input);	//Rotate the control input

	//Normalize quaternion
	qf16_normalize_to_unit(dest, dest);
}

//static inline v3d v3d_imu_to_ned(const v3d *imu) {
//	v3d ned;
//	qf16_rotate(&ned, &NED_IMU_Q, imu);
//	return ned;
//}

static inline v3d v3d_enu_to_ned(const v3d *enu) {
	v3d ned;
	qf16_rotate(&ned, &NED_ENU_Q, enu);
	return ned;
}

static inline v3d v3d_ned_to_enu(const v3d *ned) {
	v3d enu;
	qf16_rotate(&enu, &NED_ENU_Q, ned);
	return enu;
}
