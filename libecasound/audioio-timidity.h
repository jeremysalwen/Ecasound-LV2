#ifndef INCLUDED_AUDIOIO_TIMIDITY_H
#define INCLUDED_AUDIOIO_TIMIDITY_H

#include <string>
#include <cstdio>
#include "audioio-buffered.h"
#include "audioio-forked-stream.h"

/**
 * Interface to MIDI-to-audio converters that support 
 * UNIX pipe i/o. By default, Timidity++ is used.
 *
 * @author Kai Vehmanen
 */
class TIMIDITY_INTERFACE : public AUDIO_IO_BUFFERED,
			   protected AUDIO_IO_FORKED_STREAM {

 private:
  
  static std::string default_timidity_cmd;

 public:

  static void set_timidity_cmd(const std::string& value);

 public:

  TIMIDITY_INTERFACE (const std::string& name = "");
  virtual ~TIMIDITY_INTERFACE(void);
    
  virtual TIMIDITY_INTERFACE* clone(void) const { return new TIMIDITY_INTERFACE(*this); }
  virtual TIMIDITY_INTERFACE* new_expr(void) const { return new TIMIDITY_INTERFACE(); }

  virtual std::string name(void) const { return("MIDI-to-audio stream"); }
  virtual std::string description(void) const { return("Interface to MIDI-to-audio converters that support UNIX pipe i/o."); }
  virtual int supported_io_modes(void) const { return(io_read); }
  virtual bool supports_seeking(void) const { return(false); }

  virtual void open(void) throw (AUDIO_IO::SETUP_ERROR &);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples) { }

  virtual bool finished(void) const { return(finished_rep); }
  virtual void seek_position(void);
 
 private:

  bool triggered_rep;
  bool finished_rep;
  long int bytes_read_rep;
  int fd_rep;
  FILE* f1_rep;
  
  void seek_position_in_samples(long pos);
  TIMIDITY_INTERFACE& operator=(const TIMIDITY_INTERFACE& x) { return *this; }

  void fork_timidity(void);
  void kill_timidity(void);
};

#endif
