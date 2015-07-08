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
#define  SYNTH_MAX_NUM_PENDING_SAMPLES 16384

typedef struct synth_ring_buffer_struct
{
  auSample_t* buffer;
  int         head_index;
  double      count; //will be read out interpolated so count won't be integer
}synth_ring_buffer_t;

/*OpaqueSynthStruct---------------------------------------*/
struct OpaqueSynthStruct
{
  AUDIO_GUTS                           ;
  synth_ring_buffer_t** pending_samples;
  int                   current_channel; 
  double                playback_speed;
  pthread_mutex_t       mutex;
};

int synthAudioCallback         (void* SELF, auSample_t* buffer, int numFrames, int numChannels);
Synth* synthDestroy            (Synth* self);
synth_ring_buffer_t* synth_ring_buffer_new(int numSamples);
synth_ring_buffer_t* synth_ring_buffer_destroy(synth_ring_buffer_t* self);


/*---------------------------------------------------------*/
synth_ring_buffer_t* synth_ring_buffer_new(int numSamples)
{
  synth_ring_buffer_t* self = calloc(1, sizeof(*self));
  if(self != NULL)
    {
      self->buffer = calloc(numSamples, sizeof(*self->buffer));
      if(self->buffer == NULL)
        return synth_ring_buffer_destroy(self);
    }
  return self;
}

/*---------------------------------------------------------*/
synth_ring_buffer_t* synth_ring_buffer_destroy(synth_ring_buffer_t* self)
{
  if(self != NULL)
    {
      if(self->buffer != NULL)
        free(self->buffer);
        
      free(self);
    }
  return (synth_ring_buffer_t*) NULL;
}

/*syNew---------------------------------------------------*/
Synth* synthNew(unsigned num_channels)
{
  int i;
  Synth* self = (Synth*) auAlloc(sizeof(*self), synthAudioCallback, YES, num_channels);
  
  if(self != NULL)
    {
    
      if(pthread_mutex_init(&self->mutex, NULL) != 0)
        return synthDestroy(self);
            
      self->pending_samples = calloc(num_channels, sizeof(*(self->pending_samples)));
      if(self->pending_samples == NULL)
        return synthDestroy(self);
      
      for(i=0; i<num_channels; i++)
        {
          self->pending_samples[i] = synth_ring_buffer_new(SYNTH_MAX_NUM_PENDING_SAMPLES);
          if(self->pending_samples[i] == NULL)
            return synthDestroy(self);
        }
      
      self->playback_speed = 1;
      self->destroy = (Audio *(*)(Audio *))synthDestroy;
    }
  return self;
}

/*syDestroy-----------------------------------------------*/
Synth* synthDestroy(Synth* self)
{
  if(self != NULL)
    {
      int i;
      pthread_mutex_destroy(&self->mutex);
      auPause((Audio*)self);
      for(i=0; i<self->numChannels; i++)
        synth_ring_buffer_destroy(self->pending_samples[i]);

      auDestroy((Audio*)self);
    }
    
  return (Synth*) NULL;
}

/*---------------------------------------------------------*/
void    synthFeedNomNomNom(Synth* self, void* grub, int numBytes, int bytesPerSample)
{
  int i;
  synth_ring_buffer_t* ring = self->pending_samples[self->current_channel++];
  self->current_channel %= self->numChannels;
  
  char* p = (char*)grub;
  
  pthread_mutex_lock(&self->mutex);
  for(i=0; i<numBytes * bytesPerSample; i++)
    {
      if(ceil(ring->count) < SYNTH_MAX_NUM_PENDING_SAMPLES)
        {
          ring->buffer[ring->head_index++] = *p++ / (float)0xFF;
          ring->count++;
          ring->head_index %= SYNTH_MAX_NUM_PENDING_SAMPLES;
        }
      else{ fprintf(stderr, "ring buffer full! \r\n"); break;}
    }
  pthread_mutex_unlock(&self->mutex);
}

/*--------------------------------------------------------*/
void    synthSetSpeed           (Synth* self, float speed)
{
  if(speed < 0)
    speed = 0;
  self->playback_speed = speed;
}

/*--------------------------------------------------------*/
float   synthGetSpeed           (Synth* self)
{
  return self->playback_speed;
}

/*---------------------------------------------------------*/
int synthAudioCallback(void* SELF, auSample_t* buffer, int numFrames, int numChannels)
{

  Synth* self = (Synth*)SELF;

  synth_ring_buffer_t* ring;
  int channel, frame, base1, base2;
  double mantissa;
  
  pthread_mutex_lock(&self->mutex);
   
  for(frame=0; frame<numFrames; frame++)
    {
      for(channel=0; channel<numChannels; channel++)
        {
          ring = self->pending_samples[channel];
          if(ring->count > 0)
            {
              base1 = ring->head_index + SYNTH_MAX_NUM_PENDING_SAMPLES - ring->count;
              base2 = base1 + 1;
              base1 %= SYNTH_MAX_NUM_PENDING_SAMPLES;
              base2 %= SYNTH_MAX_NUM_PENDING_SAMPLES;
              mantissa = ceil(ring->count) - ring->count;
              
              *buffer++ = ring->buffer[base1] + mantissa * (ring->buffer[base2] - ring->buffer[base1]);
              ring->count -= self->playback_speed;
            }
          else
            *buffer++ = 0;
        }
    }
      
  pthread_mutex_unlock(&self->mutex);
  
  return numFrames;
}
