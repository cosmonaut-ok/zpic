# The ZPIC educational code suite

## Compiling the code

Each top level directory (em1d, em2d, etc.) has a self-contained version of the code. You can just open a terminal, navigate into into the required directory and execute `make`. No additional / external libraries are required.

You can find detailed instructions for Windows / Mac OS X / Linux in the [Compilation.md](Compilation.md) file, including instructions on how to get a C99 compiler for your system.

## Setting the simulation parameters
Simulation parameters are defined using a C file that is included in the `main.c` file of each code. For example, to launch the Two-Stream instability simulation on the 1D electrostatic code, you need to edit the `es1d/main.c` file and add:

```C
// Include Simulation parameters here
#include "input/twostream.c"
```

This file must define two functions:

* `sim_init()`	 - Initialization of the simulation
* `sim_report()` - Simulation diagnostics

Please check the examples available on the `input` directories for each code.

## Units

The current implementation uses the same normalized units as the OSIRIS code. This requires choosing a reference normalization frequency, $\omega_n$. Time is normalized to $1/\omega_n$. (Proper) velocities are normalized to the speed of light, $c$. Space is normalized to $c/\omega_n$. The fields are then normalized appropriately.

The density is normalized to $\omega_n^2$ (the normalization frequency squared). So if the density is 1 at a given location then the normalization frequency is the plasma frequency at that location. 

If the laser frequency is 1, then the normalization frequency is the laser frequency and the density is normalized to the critical densify (for that laser frequency)

|       Normalized units                   |
| ---------------------------------------- |
| $x' = \frac{\omega_n}{c} x$|
| $v' = \frac{v}{c}$ |
| $u' = \frac{u}{c} = \frac{\gamma v}{c}$ |
| $E' = e \frac{c / \omega_n }{m_e c^2} E$ |
| $B' = e \frac{c / \omega_n }{m_e c^2} B$ |

# Initializing Simulation Parameters

Initializing a ZPIC simulation requires defining a set of numerical parameters regarding the simulation spatial / temporal domain, the simulation particles. Additionally, the user may optionally specify additional parameters, such as laser pulses.

These parameters are defined in the `sim_init()` function in the input file. This function *must* call the `sim_new()` function to initialize the _sim_ (simulation) object.

The following example sets the minimal parameters for a 2D ZPIC simulation (with no particles):

```C
void sim_init( t_simulation* sim ){

	// Simulation box
	int   nx[2]  = { 100, 100 };
	float box[2] = { 10.0, 10.0 };

	// Time step
	float dt = 0.07;
	float tmax = 70.0;

	// Initialize Simulation data
	sim_new( sim, nx, box, dt, tmax, 0, NULL, 0 );
}
```

Please check the `input` directories under each code for several complete examples.

## Simulation box

The input parameters must define the number of grid points in each direction of the simulation grid, and the physical dimensions (in simulation units) of this box. These dimensions are measured from the lower boundary of the first cell to the upper boundary of the last cell.

```C
	// Simulation box
	int   nx[2]  = { 100, 100 };
	float box[2] = { 10.0, 10.0 };

	// Initialize Simulation data
	sim_new( sim, nx, box, ... );
```

## Time

The input parameters must define the simulation time step and the total simulation time (in simulation units)

```C
	// Time step
	float dt = 0.07;
	float tmax = 70.0;

	// Initialize Simulation data
	sim_new( sim, nx, box, dt, tmax, ... );
```

The code will check if the time step is compatible with the specified resolution.

## Diagnostic frequency

The input parameters must also define the frequency at which the `sim_report()` function is called from the main loop. For this purpose the user specifies the number of iterations between these function calls:

```C
	// Diagnostic frequency - call sim_report() every 50 iterations
	int ndump = 50;

	// Initialize Simulation data
	sim_new( sim, nx, box, dt, tmax, ndump );
```

Setting this parameter to 0 disables all diagnostics. For details on simulation diagnostics and the `sim_report()` function see the [Simulation Diagnostics](#simulation_diagnostics) section below.

## Particles

The input parameters may define an arbitrary number of particle species, defining the required plasma properties, such as density, temperature, fluid velocity, etc. See the [Particle injection.md](Particle_Injection.md) file.

## Adding laser pulses

The input parameters may optionally define an arbitrary number of laser pulses to be added to the simulation. Each laser pulse is added through a call to the `sim_add_laser()` routine:

```C
void sim_add_laser( t_simulation* sim, t_emf_laser* laser)
```

This routine should be called inside `sim_init()`, somewhere after the call to `sim_new()`. Laser parameters are defined in the supplied _t\_emf\_laser_ structure.

| Laser parameters| Description|
|---|---|
| type | PLANE for a plane wave or GAUSSIAN for a gaussian beam |
| start | Front edge of the laser pulse |
| fwhm  | FWHM of the laser pulse duration |
| rise, flat, fall  | Rise / flat / fall time of the laser pulse |
| a0  | Normalized peak vector potential of the pulse |
| omega0 | Laser frequency |
| polarization | Laser polarization angle, 0 aligns $E$ field along $x_2$ |
| W0 | Gaussian beam waist |
| focus | Focal plane position |
| axis | Position of the optical axis |

All parameters are in normalized simulation units except for the normalized peak vector potential, $a_0$, which is adimensional. In 1D only plane waves exist, so the _type_, _W0_, _focus_ and _axis_ parameters are not supported.

Using the _fwhm_ parameter will override the _rise_, _flat_ and _fall_ parameters. Specifically, it sets _rise = fwhm/2_, _flat = 0_, and _fall = fwhm/2_.

The following example launches a laser starting at position 17.0, with a (temporal) full width at half max of 2.0. The peak normalized vector potential is 2.0, and the laser frequency is 10.0. The polarization degree is $\pi/2$, which aligns the $E$ field along the $x_3$ direction. All values are in normalized simulation units.

```C
t_emf_laser laser = {
	.start = 17.0,
	.fwhm  = 2.0,
	.a0 = 2.0,
	.omega0 = 10.0,
	.polarization = M_PI_2
};
sim_add_laser( sim, &laser );
```

The following is a 2D example of a gaussian laser pulse. It uses the same parameters as the previous example, set the beam focus waist to 4.0, the focal plane position to x=20.0, and the propagation axis to y=12.8.

```C
t_emf_laser laser = {
	.type = GAUSSIAN,
	.start = 17.0,
	.fwhm  = 2.0,
	.a0 = 2.0,
	.omega0 = 10.0,
	.W0 = 4.0,
	.focus = 20.0,
	.axis = 12.8,
	.polarization = M_PI_2
};
sim_add_laser( sim, &laser );
```

See for example the Laser Wakefield input files (e.g. [lwfa.c](../em1d/input/lwfa.c)).

## Moving simulation window

The finite difference models can be run using a moving simulation window that moves at the speed of light. Please note that simulation is done in the lab reference frame; it is the simulation box that moves and follows relevant phenomena moving at, or close to, this speed, such as laser pulses or relativistic particle beams.

Using a moving window requires calling the `sim_set_moving_window()` routine:

```C
void sim_set_moving_window( t_simulation* sim )
```

This routine should be called inside `sim_init()`, somewhere after the call to `sim_new()`. Spectral models currently do not support this feature.

See for example the Laser Wakefield input files (e.g. [lwfa.c](../em1d/input/lwfa.c)).


## Neutralizing backgound

### Finite difference models

When using the finite difference models, the code will always behave as if the total charge density was initially zero, even when this is not the case. This effect is generally described as having the code add a neutralizing background; however, for finite difference models, this is simply a consequence of the way the field solver is implemented, and no actual addition takes place. So to model a plasma with a fixed ion background, it is enough to add the electron species, and no additional particle species are required.

### Spectral models

For the spectral models this is no longer the case, and if required a neutralizing background must be explicitly added. This is achieved through a call to the `sim_add_neutral_bkg()` routine:

```C
void sim_add_neutral_bkg( t_simulation* sim )
```

This routine should be called inside `sim_init()`, somewhere after the call to `sim_new()`.

The code will calculate the charge density resulting from the initial charge distribution of the multiple species and store that value on an auxiliary buffer. This buffer will then be subtracted from the simulation charge density calculated at each time step, before calling the field advance.

For an example, see the Laser Wakefield example for the 1D electromagnetic spectral code: [lwfa.c](../em1ds/input/lwfa.c))

## Electric Current Smoothing

For the finite difference models, the input parameters may also optionally define some smoothing (filtering) parameters to be applied to the electric current following the deposition of this quantity from particle motion. This is achieved through a call to the `sim_set_smooth()` routine:

```C
void sim_set_smooth( t_simulation* sim, t_smooth* smooth)
```

This routine should be called inside `sim_init()`, somewhere after the call to `sim_new()`. Smoothing parameters are defined in the supplied _t\_smooth_ structure.

| Smoothing parameters| Description|
|---|---|
| xtype | Smoothing type along x. Must be one of `none`, `binomial` or `compensated` |
| xlevel | Number of passes of the smoothing kernel along x |
| ytype  | (2D only) Same as xtype for the y direction |
| ylevel  | (2D only) Same as xlevel for the y direction |

For `binomial` smoothing, filtering is performed through a convolution of the grid data with a \[1,2,1\] kernel. This operation is repeated _n_ times, where _n_ is defined by the `xlevel | ylevel` parameters.

For `compensated` smoothing, the code will first apply `binomial` smoothing as described above, and will perform an additional pass with a specially calculated kernel that ensures that the filter transfer function near _k_=0 will be on the order of $1 + O(k^3)$, zeroing out $k^2$ dependencies. In pratice this leads to a filter that is flatter near _k_=0 and has a sharper drop near the cutoff frequency.

# Simulation Diagnostics

Simulation diagnostics are defined in the `sim_report()` function of the input file. This function will be called from the simulation main loop at a frequency defined in the `sim_init()` function described above. All input files must define this function.

Here's an example from a 2D Weibel instability simulation:

```C
void sim_report( t_simulation* sim ){
	// Bx, By
	emf_report( &sim -> emf, BFLD, 0 );
	emf_report( &sim -> emf, BFLD, 1 );

	// Jz
	current_report( &sim -> current, 2 );

	// electron and positron density
	spec_report( &sim -> species[0], CHARGE, NULL, NULL );
	spec_report( &sim -> species[1], CHARGE, NULL, NULL );
}
```

## Electromagnetic Field Diagnostics

Electromagnetic field diagnostics are done through a call to the `emf_report()` function:
```C
void emf_report( t_emf *emf, char field, char fc )
```

This function may be called multiple times inside `sim_report()` with the following parameters:

| EMF report parameters| Description|
|---|---|
| field | Field to save. Must be one of `BFLD` or `EFLD` |
| fc | Field component to save. Must be one of 0(x), 1(y), or 2(z).  |

## Electric Current Diagnostics

Electric Current diagnostics are done through a call to the `current_report()` function:
```C
void current_report( const t_current *emf, const char field, const char fc )
```

This function may be called multiple times inside `sim_report()` with the following parameters:

| Current report parameters| Description|
|---|---|
| fc | Electric current component to save. Must be one of 0(x), 1(y), or 2(z). |

## Electric Charge Diagnostics (spectral codes only)

Global electric Charge diagnostics are done through a call to the `charge_report()` function:
```C
void charge_report( t_current *charge )
```
This function is only available in spectral code (es1d, em1ds, em2ds) as this quantity is not required for the finit difference models.

This function should only be called once inside `sim_report()`.

## Particle diagnostics

Particle species diagnostics are done through a call to the `spec_report()` function:
```C
spec_report( t_species *spec, int rep_type, int pha_nx[], float pha_range[][2] )
```

This function may be called multiple times inside `sim_report()` with the following parameters:

| EMF report parameters| Description|
|---|---|
| rep_type | Type of diagnostic. Can be set to `CHARGE`, `PARTICLES`, or the `PHASESPACE()` macro |
| pha_nx | Dimensions (number of grid points) of the grid for phasespace density diagnostics  |
| pha_range | Physical grid size for phasespace density diagnostics |

This function is meant to be called per simulation species (set by the _spec_ parameter), and acting only on the specified species.

The following types of reports are available:
* **CHARGE** - Saves the charge density of the species. This density is calculated in the simulation grid. The parameters _pha\_nx_ and _pha\_range_ should be set to `NULL`.
* **PARTICLES** -  Saves the complete particle dataset: position, (generalized) velocity, charge for all particles in the species. The parameters _pha\_nx_ and _pha\_range_ should be set to `NULL`.
* **PHASESPACE(ax1,ax2)** - Saves the specified phasespace density for this plot. The phasespace parameters are controlled through the following parameters:
  * _ax1_, _ax2_ - Define the phasespace axis. Can be set to `X1`, `X2` (2D only), `U1`, `U2`, `U3` (EM codes) or `V1` (ES codes).
  * _pha_nx_ - Defines the number of grid points in each direction for the phasespace density grid.
  * _pha_range_ - Defines the physical limits (in simulation units) of the phasespace density grid in each direction.

In the following example, a simulation with two species saves diagnostic information for the charge density and $(x_1,u_1)$ phasespace density of species 0, and the complete particle dataset for species 1:

```C
void sim_report( t_simulation* sim ){

	spec_report( &sim -> species[0], CHARGE, NULL, NULL );
	spec_report( &sim -> species[0], PARTICLES, NULL, NULL );

	const int pha_nx[] = {1024,512};
	const float pha_range[][2] = {{0.0,20.0}, {-2.0,+2.0}};
	spec_report( &sim -> species[1], PHASESPACE(X1,U1), pha_nx, pha_range);
}
```


# Algorithm details

## Time centering of quantities

Time integration of quantities in these codes is done using a leap frog method. For this purpose particle positions are defined at integral time steps, $t_i$, $t_{i+1}$, etc., whereas velocities (or generalized velocities in the relativistic versions) are defined at half time steps $t_{i-1/2}$, $t_{i+1/2}$, etc.

As a consequence, the electromagnetic fields, $E$ and $B$, as well as charge density, $\rho$, are also defined at integral timesteps $t_i$. Energy will also be defined at integral time steps. For this purpose particle energy is calculated during the particle advance, using time centered values of the velocity.

Current density, $j$, is defined at half time steps $t_{i+1/2}$.
