#ifndef INCLUDED_AUDIOIO_ALSA3_H
#define INCLUDED_AUDIOIO_ALSA3_H

#include <string>
#include <iostream>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio-types.h"
#include "eca-version.h"

#ifdef ALSALIB_060
#include <alsa/asoundlib.h>
#endif

using namespace std;

/**
 * Class for handling ALSA pcm-devices (Advanced Linux Sound Architecture).
 */
class ALSA_PCM_DEVICE_06X : public AUDIO_IO_DEVICE {

#ifdef ALSALIB_060
  snd_pcm_t *audio_fd_repp;
  snd_pcm_stream_t pcm_stream_rep;
  snd_pcm_format_t format_rep;
  snd_pcm_hw_params_t* pcm_hw_params_repp;
  snd_pcm_sw_params_t* pcm_sw_params_repp;
#endif

  long int fragment_size_rep;
  long int position_in_samples_rep;
 
  int card_number_rep, device_number_rep, subdevice_number_rep;
  int pcm_mode_rep;

  long int bytes_read_rep;
  long underruns_rep, overruns_rep;
  unsigned char **nbufs_repp;

  string pcm_device_name_rep;
  static const string default_pcm_device_rep;

  bool is_triggered_rep;
  bool is_prepared_rep;
  bool using_plugin_rep;
  bool trigger_request_rep;

  void open_device(void);

  void allocate_structs(void);
  void deallocate_structs(void);

  void set_audio_format_params(void);
  void fill_and_set_hw_params(void);
  void fill_and_set_sw_params(void);
  void print_pcm_info(void);
  void handle_xrun_capture(void);
  void handle_xrun_playback(void);

 protected:

  void set_pcm_device_name(const string& n);
  const string& pcm_device_name(void) const { return(pcm_device_name_rep); }

 public:

  virtual string name(void) const { return("ALSA PCM device"); }
  virtual string description(void) const { return("ALSA PCM devices. Library versions 0.6.x and newer."); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual string parameter_names(void) const { return("label,card,device,subdevice"); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);
  void prepare(void);

  virtual long position_in_samples(void) const;

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  ALSA_PCM_DEVICE_06X (int card = 0, int device = 0, int subdevice = -1);
  virtual ~ALSA_PCM_DEVICE_06X(void);
  ALSA_PCM_DEVICE_06X* clone(void) { cerr << "Not implemented!" << endl; return this; }
  ALSA_PCM_DEVICE_06X* new_expr(void) { return new ALSA_PCM_DEVICE_06X(); }
  
 private:

  ALSA_PCM_DEVICE_06X (const ALSA_PCM_DEVICE_06X& x) { }
  ALSA_PCM_DEVICE_06X& operator=(const ALSA_PCM_DEVICE_06X& x) { return *this; }
};

extern "C" {
AUDIO_IO* audio_io_descriptor(void);
int audio_io_interface_version(void);
};

#endif
