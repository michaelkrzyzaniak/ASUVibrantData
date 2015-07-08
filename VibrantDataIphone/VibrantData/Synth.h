/*
 *  Synth.h
 *  Make weird noises
 *
 *  Made by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Summer of 2015
 *  mkrzyzan@asu.edu
 */

#ifndef __SYNTH__
#define __SYNTH__ 1

#if defined(__cplusplus)
extern "C"{
#endif   //(__cplusplus)

#include "AudioSuperclass.h"

typedef struct OpaqueSynthStruct Synth;

Synth*  synthNew                 (unsigned num_channels);
Synth*  synthDestroy             (Synth* self);
void    synth_set_carrier_freq   (Synth* self, double cps);
double  synth_get_carrier_freq   (Synth* self);
void    synth_set_gain           (Synth* self, double decibels);
double  synth_get_gain           (Synth* self);

void   synth_set_pulse_duration  (Synth* self, double seconds);
double synth_get_pulse_duration  (Synth* self);
void   synth_set_pulse_spacing   (Synth* self, double seconds);
double synth_get_pulse_spacing   (Synth* self);

//these are not orthogonal to the above getter / setter methods
void    synth_set_vibration_magnitude(Synth* self, double coefficient);
double  synth_get_vibration_magnitude(Synth* self);

#if defined(__cplusplus)
}
#endif   //(__cplusplus)

#endif   // __SYNTH__
