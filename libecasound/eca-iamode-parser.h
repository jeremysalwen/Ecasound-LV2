#ifndef INCLUDED_ECA_IAMODE_PARSER_H
#define INCLUDED_ECA_IAMODE_PARSER_H

#include <map>
#include <string>
#include <vector>

#include "eca-error.h"

/**
 * Class that handles registering and querying interactive mode commands.
 * @author Kai Vehmanen
 */
class ECA_IAMODE_PARSER {

 protected:

  static map<string,int> cmd_map_rep;
  static void register_commands(void);

 public:
  
  enum Commands {
    ec_direct_option,
    // --
    ec_help,
    ec_exit,
    ec_start,
    ec_stop,
    ec_run,
    ec_debug,
    // --
    ec_st_general,
    // --
    ec_rewind,
    ec_forward,
    ec_setpos,
    // --
    ec_cs_add,
    ec_cs_remove,
    ec_cs_select,
    ec_cs_index_select,
    ec_cs_load,
    ec_cs_save,
    ec_cs_save_as,
    ec_cs_edit,
    ec_cs_connect,
    ec_cs_disconnect,
    ec_cs_set,
    ec_cs_format,
    ec_cs_status,
    ec_cs_length,
    ec_cs_loop,
    // --
    ec_c_add,
    ec_c_select,
    ec_c_select_all,
    ec_c_select_add,
    ec_c_deselect,
    ec_c_remove,
    ec_c_clear,
    ec_c_name,
    ec_c_mute,
    ec_c_bypass,
    ec_c_forward,
    ec_c_rewind,
    ec_c_setpos,
    ec_c_status,
    // --
    ec_aio_add_input,
    ec_aio_add_output,
    ec_aio_remove,
    ec_aio_select,
    ec_aio_select_input,
    ec_aio_select_output,
    ec_aio_index_select,
    ec_aio_attach,
    ec_aio_status,
    ec_aio_forward,
    ec_aio_rewind,
    ec_aio_setpos,
    ec_aio_wave_edit,
    ec_aio_register,
    // --
    ec_cop_add,
    ec_cop_remove,
    ec_cop_select,  // not implemented
    ec_cop_set,
    ec_cop_status,
    ec_cop_register,
    // --
    ec_ladspa_register,
    ec_preset_register,
    // --
    ec_ctrl_add,
    ec_ctrl_remove, // not implemented
    ec_ctrl_status,
    ec_ctrl_register,
    // --
    ec_dump_target,
    ec_dump_status,
    ec_dump_position,
    ec_dump_length,
    ec_dump_cs_status,
    ec_dump_c_selected,
    ec_dump_aio_selected,
    ec_dump_aio_position,
    ec_dump_aio_length,
    ec_dump_aio_open_state,
    ec_dump_cop_value
  };

  /**
   * Parse string mode command and act accordingly.
   */
  static const map<string,int>& registered_commands(void) { return(cmd_map_rep); }

  bool action_requires_params(int id);
  bool action_requires_connected(int id);
  bool action_requires_selected_not_connected(int id);
  bool action_requires_selected(int id);
  bool action_requires_selected_audio_object(int id);

  ECA_IAMODE_PARSER(void) { register_commands(); }
  virtual ~ECA_IAMODE_PARSER(void) { }
};

void show_controller_help(void);
void show_controller_help_more(void);

#endif

