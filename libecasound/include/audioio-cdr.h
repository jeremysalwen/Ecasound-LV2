#ifndef _AUDIOIO_CDR_H
#define _AUDIOIO_CDR_H

#include <string>
#include "samplebuffer.h"

typedef struct {
  public:
  int16_t sample[2]; // signed short int
} SAMPLE;

/**
 * Class for handling CDR-files
 * 
 * CDR -format is used on audio-CDs:
 *
 * - 16bit samples, 44100kHz, stereo
 *
 * - sample frame layout: (MSB-left, LSB-left, MSB-right, LSB-right)
 *
 * - files are padded sector size (2352 bytes)
 *
 * @author Kai Vehmanen
 */
class CDRFILE : public AUDIO_IO_BUFFERED {

  static const int sectorsize = 2352;

  long int samples_read;

  FILE* fobject;
  void pad_to_sectorsize(void);
  void set_length_in_bytes(void);

  //  CDRFILE(const CDRFILE& x) { }
  CDRFILE& operator=(const CDRFILE& x) { return *this; }

  void seek_position_in_samples(long pos);
  unsigned short swapw(unsigned short us) { return ((us >> 8) | (us << 8)) & 0xffff;  }

  void swap_bytes(SAMPLE* t) {
    t->sample[SAMPLE_SPECS::ch_left] = swapw(t->sample[SAMPLE_SPECS::ch_left]);
    t->sample[SAMPLE_SPECS::ch_right] = swapw(t->sample[SAMPLE_SPECS::ch_right]);
  }
  
  /**
   * Do a info query prior to actually opening the device.
   *
   * require:
   *  !is_open()
   *
   * ensure:
   *  !is_open()
   */
  void format_query(void);

 public:

  string name(void) const { return("CD-R audio file"); }
  virtual bool locked_audio_format(void) const { return(true); }

  void open(void) throw(ECA_ERROR*);
  void close(void);

  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  bool finished(void) const;
  void seek_position(void);
    
  CDRFILE (const string& name = "");
  ~CDRFILE(void);
    
  CDRFILE* clone(void) { return new CDRFILE(*this); }
  CDRFILE* new_expr(void) { return new CDRFILE(); }
};

#endif











