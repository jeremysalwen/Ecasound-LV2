#ifndef INCLUDED_AUDIOIO_H
#define INCLUDED_AUDIOIO_H

#include <string>

#include "eca-audio-position.h"
#include "eca-audio-time.h"
#include "dynamic-object.h"
#include "samplebuffer.h"

/**
 * Virtual base for all audio I/O classes (files, audio devices,
 * sound producing program modules, etc.)
 * @author Kai Vehmanen
 */
class AUDIO_IO : public DYNAMIC_OBJECT<string>,
		 public ECA_AUDIO_POSITION {

 public:

  /**
   * Input/Output mode
   *
   * @see io_mode()
   *
   * io_read
   *
   * Device is opened for input. If opening a file, 
   * it must exist.
   *
   * io_write
   *
   * Device is opened for output. If opening a file and
   * and output exists, it is first truncated.
   * 
   * io_readwrite
   *
   * Device is opened for both reading and writing. If
   * opening a file, a new file is created if needed. 
   * When switching from read to write or vica versa,
   * position should be reset before using the device.
   **/
  enum Io_mode { io_read = 1, io_write = 2, io_readwrite = 4 };

  // ===================================================================
  // Buffering

  /**
   * Set buffersize. Instead of specifying buffer length in seconds,
   * parameters 'samples' and 'samples_rate' are used to determinate the 
   * exact temporal length. When reading, this means that 'buffersize' of 
   * sample data is read. When writing, 'buffersize' is only used for 
   * initializing devices and data structures. Device should be able 
   * to write all sample data independent from current buffersize value.
   * If audio object's sampling rate is changed, buffersize must also be adjusted.
   */
  virtual void buffersize(long int samples, long int sample_rate) = 0;

  /**
   * Returns buffersize in samples.
   */
  virtual long int buffersize(void) const = 0;

  // ===================================================================
  // Basic input/output

  /**
   * Read samples to buffer pointed by 'sbuf'. If necessary, the target 
   * buffer will be resized.
   *
   * @see read_samples
   *
   * require:
   *  io_mode() == si_read || io_mode() == si_readwrite
   *  readable() == true
   *  sbuf != 0
   *
   * ensure:
   *  sbuf->length_in_samples() <= buffersize()
   */
  virtual void read_buffer(SAMPLE_BUFFER* sbuf) = 0;

  /**
   * Write all data from sample buffer pointed by 'sbuf'.
   *
   * @see write samples
   *
   * require:
   *  io_mode() == si_write || io_mode() == si_readwrite
   *  writable() == true
   *  sbuf != 0
   */
  virtual void write_buffer(SAMPLE_BUFFER* sbuf) = 0;

  /**
   * If supports_buffer_queries() == true, this function can be 
   * used to check how many samples are available for reading, or 
   * alternatively, how many samples can be written without 
   * blocking. Effectively this is meant for implementing nonblocking 
   * input and output with devices supporting it. Note, you 
   * should use buffersize() call for setting how many bytes 
   * read_buffer() will ask from the device.
   *
   * require:
   *  supports_buffer_queries() == true
   */
  long int samples_available(void) const { return(0); }

  /**
   * Whether all data has been processed? If opened in mode 'si_read', 
   * this means that end of stream has been reached. If opened in 
   * 'si_write' or 'si_readwrite' modes, finished status usually
   * means that an error has occured (no space left, etc).
   */
  virtual bool finished(void) const = 0;

  // ===================================================================
  // Audio object control

 public: 
    
  /**
   * Open audio object (possibly in exclusive mode).
   * This routine is meant for opening files and devices,
   * loading libraries, etc. Object's audio format can be 
   * adjusted during this call.
   *
   * ensure:
   *  readable() == true || writable() == true
   */
  virtual void open(void) = 0;

  /**
   * Close audio object. After calling this routine, 
   * all resources (ie. soundcard) must be freed
   * (they can be used by other processes).
   *
   * ensure:
   *  readable() != true
   *  writable() != true
   */
  virtual void close(void) = 0;

  // ===================================================================
  // Parameter handling - see DYNAMIC_PARAMETERS documentation.
  
  virtual string parameter_names(void) const { return("label"); }
  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  // ===================================================================
  // Status and info

  /**
   * Optional status string
   */
  virtual string status(void) const;

  /**
   * Returns info about supported I/O modes (bitwise-OR)
   */
  virtual int supported_io_modes(void) const { return(io_read | io_readwrite | io_write); }

  /**
   * Whether device supports the samples_available() function call.
   */
  virtual bool supports_buffer_queries(void) const { return(false); }

  /**
   * Whether audio format is locked
   */
  virtual bool locked_audio_format(void) const { return(false); }

  /**
   * Returns info about the current I/O mode.
   */
  int io_mode(void) const { return(si_mode_rep); }

  /**
   * The device name (usually set to device/file name).
   */
  const string& label(void) const { return(id_label_rep); }

  /**
   * Get a string containing info about sample format parameters.
   */
  string format_info(void) const;

  /**
   * Has device been opened (with open())?
   */
  bool is_open(void) const { return(open_rep); }

  /**
   * If applicable, returns total length of the audio data stored
   * into current audio object.
   */
  ECA_AUDIO_TIME length(void) const;

  /**
   * Returns the current position.
   */
  ECA_AUDIO_TIME position(void) const;

  /**
   * Is the audio object reading for reading? 
   */
  virtual bool readable(void) const { return(is_open() && io_mode() != io_write); }

  /**
   * Is the audio object reading for writing? 
   */
  virtual bool writable(void) const { return(is_open() && io_mode() != io_read); }

  // --
  // Format setup
  // --

  /**
   * Set object input/output-mode. If the requested mode isn't
   * supported, the nearest supported mode is used. Because 
   * of this, it's wise to afterwards check whether the requested
   * mode was accepted.
   *
   * require:
   *  is_open() != true
   */
  void io_mode(int mode) { si_mode_rep = mode; }

  /**
   * Set object label
   *
   * require:
   *  is_open() != true
   */
  void label(const string& id_label) { id_label_rep = id_label; }

 protected:

  /**
   * Sets the current position.
   */
  void position(const ECA_AUDIO_TIME& v);

  /**
   * Sets the total length of audio object data.
   */
  void length(const ECA_AUDIO_TIME& v);

  /**
   * Sets device's state to enabled or disabled.
   */
  void toggle_open_state(bool value) { open_rep = value; }

  // ===================================================================
  // Constructors and destructors

 public:

  virtual AUDIO_IO* clone(void) = 0;
  virtual AUDIO_IO* new_expr(void) = 0;
  virtual ~AUDIO_IO(void) { }
  AUDIO_IO(const string& name = "unknown", 
	   int mode = io_read, 
	   const ECA_AUDIO_FORMAT& fmt = ECA_AUDIO_FORMAT())
    : ECA_AUDIO_POSITION(fmt)
    {
      label(name);
      io_mode(mode);
      
      position_in_samples(0);

      readable_rep = writable_rep = open_rep = false;
    }

 private:
  
  int si_mode_rep;
  string id_label_rep;

  bool readable_rep;
  bool writable_rep;
  bool open_rep;
};

#endif
