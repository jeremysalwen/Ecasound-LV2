#ifndef INCLUDE_STAMP_CTRL_H
#define INCLUDE_STAMP_CTRL_H

#include <string>
#include "ctrl-source.h"
#include "audio-stamp.h"
#include "samplebuffer.h"

/**
 * Controller sources that analyze audio stamps
 * and produce control data.
 * @author Kai Vehmanen
 */
class AUDIO_STAMP_CONTROLLER : public CONTROLLER_SOURCE,
			       public AUDIO_STAMP_CLIENT {

 public:

};

/**
 * Controller that analyzes stamp volume level, and creates
 * control data based on the results.
 */
class VOLUME_ANALYZE_CONTROLLER : public AUDIO_STAMP_CONTROLLER {

 public:

  virtual string name(void) const { return("Volume analyze controller"); }
  virtual parameter_type value(void);

  virtual void init(parameter_type step);
  virtual string parameter_names(void) const { return("stamp-id,rms-toggle"); }
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  VOLUME_ANALYZE_CONTROLLER(void); 
  VOLUME_ANALYZE_CONTROLLER* clone(void)  { return new VOLUME_ANALYZE_CONTROLLER(*this); }
  VOLUME_ANALYZE_CONTROLLER* new_expr(void)  { return new VOLUME_ANALYZE_CONTROLLER(*this); }

 private:

  int rms_mode_rep;
  SAMPLE_BUFFER sbuf_rep;
};

#endif