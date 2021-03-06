#ifndef __SIMULATION__
#define __SIMULATION__

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "particles.h"
#include "field.h"
#include "charge.h"

typedef struct {
	
	// Time step
	float dt;
	float tmax;
	
	// Diagnostic frequency
	int ndump;

	// Simulation data
	int n_species;
	t_species* species;
	t_field field;
	t_charge charge;

	// FFT
	t_fftr_cfg fft_forward,fft_backward;

} t_simulation;


void sim_init( t_simulation* sim );
void sim_report( t_simulation* sim );

void sim_add_neutral_bkg( t_simulation* sim );

void sim_new( t_simulation* sim, int nx, float box, float dt, float tmax, int ndump, t_species* species, int n_species );
int report( int n, int ndump );
void sim_timings( t_simulation* sim, uint64_t t0, uint64_t t1 );
void sim_delete( t_simulation* sim );

#endif
