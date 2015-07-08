/*
 *  Synth.h
 *  Make weird noises
 *
 *  Made by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Summer of 2015
 *  mkrzyzan@asu.edu
 */

#include "Synth.h"
#include <pthread.h>
#include <math.h>
#define  SYNTH_RESONANT_FREQUENCY 88 
#define  SYNTH_PULSE_DURATION     0.08
#define  SYNTH_MIN_GAIN           -25

/*OpaqueSynthStruct---------------------------------------*/
struct OpaqueSynthStruct
{
  AUDIO_GUTS                              ;
  double   amp                            ;
  double   carrier_freq                   ;
  uint32_t carrier_phase                  ;
  int      pulse_duration, pulse_spacing  ;
  int      pulse_counter                  ;
  
  double   vibration_magnitude            ;
};

int synthAudioCallback         (void* SELF, auSample_t* buffer, int numFrames, int numChannels);
Synth* synthDestroy            (Synth* self);

/*syNew---------------------------------------------------*/
Synth* synthNew(unsigned num_channels)
{
  Synth* self = (Synth*) auAlloc(sizeof(*self), synthAudioCallback, YES, num_channels);
  
  if(self != NULL)
    {
      synth_set_carrier_freq(self, SYNTH_RESONANT_FREQUENCY);
      synth_set_gain(self, SYNTH_MIN_GAIN);
      synth_set_pulse_duration(self, SYNTH_PULSE_DURATION);
      synth_set_pulse_spacing (self, 1);
      
      self->destroy = (Audio *(*)(Audio *))synthDestroy;
    }
  return self;
}

/*--------------------------------------------------------*/
void synth_set_carrier_freq(Synth* self, double cps)
{
  self->carrier_freq = cps * AU_TWO_PI_OVER_SAMPLE_RATE;
}

/*--------------------------------------------------------*/
double synth_get_carrier_freq(Synth* self)
{
  return self->carrier_freq / AU_TWO_PI_OVER_SAMPLE_RATE;
}

/*--------------------------------------------------------*/
void synth_set_pulse_duration(Synth* self, double seconds)
{
  self->pulse_duration = seconds * AU_SAMPLE_RATE + 0.5;
}

/*--------------------------------------------------------*/
double synth_get_pulse_duration(Synth* self)
{
  return self->pulse_duration / AU_SAMPLE_RATE;
}

/*--------------------------------------------------------*/
void synth_set_pulse_spacing(Synth* self, double seconds)
{
  self->pulse_spacing = seconds * AU_SAMPLE_RATE + 0.5;
}

/*--------------------------------------------------------*/
double synth_get_pulse_spacing(Synth* self)
{
  return self->pulse_spacing / AU_SAMPLE_RATE;
}

/*--------------------------------------------------------*/
void synth_set_gain(Synth* self, double decibels)
{
  AU_CONSTRAIN(decibels, SYNTH_MIN_GAIN, 0);
  if(decibels == SYNTH_MIN_GAIN)
    self->amp = 0;
  else
    self->amp = pow(2.0, decibels/6.0);
}

/*--------------------------------------------------------*/
double synth_get_gain(Synth* self)
{
  double result;
  if(self->amp == 0)
    result = SYNTH_MIN_GAIN;
  else
    result = 6 * log2(self->amp);
    
  return result;
}

/*--------------------------------------------------------*/
double synth_scalef(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*--------------------------------------------------------*/
void    synth_set_vibration_magnitude(Synth* self, double coefficient)
{
  AU_CONSTRAIN(coefficient, 0.0, 1.0);
  self->vibration_magnitude = coefficient;
  
  double decibels = synth_scalef(coefficient, 0, 1, SYNTH_MIN_GAIN, 0);
  double spacing     = synth_scalef(coefficient, 0, 1, 2, SYNTH_PULSE_DURATION);
  
  synth_set_gain(self, decibels);
  synth_set_pulse_spacing(self, spacing);
}

/*--------------------------------------------------------*/
double  synth_get_vibration_magnitude(Synth* self)
{
  return self->vibration_magnitude;
}

/*syDestroy-----------------------------------------------*/
Synth* synthDestroy(Synth* self)
{
  if(self != NULL)
    {
      auPause((Audio*)self);
      auDestroy((Audio*)self);
    }
    
  return (Synth*) NULL;
}

/*---------------------------------------------------------*/
int synthAudioCallback(void* SELF, auSample_t* buffer, int numFrames, int numChannels)
{

  Synth* self = (Synth*)SELF;
  auSample_t sample;
  int frame, channel;

  for(frame=0; frame<numFrames; frame++)
    {
      sample = fastsin(self->carrier_phase);
      
      
      sample *= (self->pulse_counter > self->pulse_duration) ? 0 : self->amp;
      
      self->pulse_counter++; 
      self->pulse_counter %= self->pulse_spacing;
      self->carrier_phase   += self->carrier_freq;
      
      for(channel=0; channel<numChannels; channel++)
        {
          *buffer++ = sample;
        }
    }
      
  
  return numFrames;
}
