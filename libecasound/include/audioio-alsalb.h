#ifndef _AUDIOIO_ALSALB_H
#define _AUDIOIO_ALSALB_H

#include <config.h>
#ifdef COMPILE_ALSA

#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/asoundlib.h>

#include "samplebuffer.h"

#ifdef ALSALIB_050
void loopback_callback_data(void *private_data, char *buffer, size_t count);
void loopback_callback_position_change(void *private_data, unsigned int pos);
void loopback_callback_format_change(void *private_data, snd_pcm_format_t *format);
#endif

/**
 * Class for handling ALSA loopback-devices (Advanced Linux Sound Architecture).
 * @author Kai Vehmanen
 */
class ALSA_LOOPBACK_DEVICE : public AUDIO_IO_DEVICE {

  snd_pcm_loopback_t *audio_fd;

  int card_number, device_number;
  bool pb_mode;
  long int bytes_read;

  bool is_triggered;
  
 public:

  void open(void) throw(ECA_ERROR*);
  void close(void);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples) { }

  void stop(void) { }
  void start(void) { }

  ALSA_LOOPBACK_DEVICE (int card, int device, const SIMODE mode, const
		ECA_AUDIO_FORMAT& form, long int buffersize,
		bool playback_mode = true);
  ALSA_LOOPBACK_DEVICE::~ALSA_LOOPBACK_DEVICE(void);
  ALSA_LOOPBACK_DEVICE* clone(void) { return new ALSA_LOOPBACK_DEVICE(*this); }
  
 private:
  
  //  ALSA_LOOPBACK_DEVICE(const ALSA_LOOPBACK_DEVICE& x) { }
  ALSA_LOOPBACK_DEVICE& operator=(const ALSA_LOOPBACK_DEVICE& x) { return *this; }
};

#endif // COMPILE_ALSA
#endif
