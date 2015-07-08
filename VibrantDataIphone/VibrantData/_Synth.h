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

Synth*  synthNew                  (unsigned num_channels);
Synth*  synthDestroy              (Synth* self);
void    synthSetSpeed             (Synth* self, float speed);
float   synthGetSpeed             (Synth* self);
void    synthFeedNomNomNom        (Synth* self, void* grub, int numBytes, int bytesPerSample);

//you don't need to call this unless you are simulating mic data
//int micAudioCallback(void* SELF, auSample_t* buffer, int numFrames, int numChannels);

#if defined(__cplusplus)
}
#endif   //(__cplusplus)

#endif   // __SYNTH__
