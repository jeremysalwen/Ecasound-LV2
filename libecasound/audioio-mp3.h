#ifndef INCLUDED_AUDIOIO_MP3_H
#define INCLUDED_AUDIOIO_MP3_H

#include <string>
#include "audioio-types.h"
#include "samplebuffer.h"

/**
 *Interface for mp3 decoders and encoders that support 
 * input/output using standard streams. Defaults to
 * mpg123 and lame.
 * @author Kai Vehmanen
 */
class MP3FILE : public AUDIO_IO_BUFFERED {

 private:
  
  static string default_mp3_input_cmd;
  static string default_mp3_output_cmd;

 public:

  static void set_mp3_input_cmd(const string& value);
  static void set_mp3_output_cmd(const string& value);

 private:

  bool finished_rep;
  int pid_of_child_rep;
  long pcm_rep;
  long int bytes_rep;
  int fd_rep;
  
  void get_mp3_params(const string& fname) throw(ECA_ERROR*);
  
  //  MP3FILE(const MP3FILE& x) { }
  MP3FILE& operator=(const MP3FILE& x) { return *this; }

  void fork_mpg123(void) throw(ECA_ERROR*);
  void kill_mpg123(void);
  void fork_lame(void) throw(ECA_ERROR*);
  void kill_lame(void);
  
 public:

  virtual string name(void) const { return("MP3 file"); }
  virtual string description(void) const { return("Wrapper object that reads mp3s using mpg123 and writes them using lame."); }

  virtual void open(void);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual bool finished(void) const { return(finished_rep); }
  virtual void seek_position(void);

  // --
  // Realtime related functions
  // --
  
  MP3FILE (const string& name = "");
  ~MP3FILE(void);
    
  MP3FILE* clone(void) { return new MP3FILE(*this); }
  MP3FILE* new_expr(void) { return new MP3FILE(*this); }
};

#endif
