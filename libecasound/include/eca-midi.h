#ifndef _ECA_MIDI_H
#define _ECA_MIDI_H

#define MIDI_IN_QUEUE_SIZE 1024

#include <pthread.h>
#include <vector>

class ECA_ERROR;

void init_midi_queues(void) throw(ECA_ERROR*);
void *update_midi_queues(void *);

/**
 * Routines for accessing raw MIDI -devices (OSS or ALSA).
 */
class MIDI_IN_QUEUE {

private:

    vector<char> buffer;

    bool right;

    size_t current_put, current_get;
    size_t bufsize;

    double controller_value;

    bool is_status_byte(char byte);
    bool forth_get(void);

public:

    void put(char byte);  
    double last_controller_value(void);
    bool update_controller_value(double controller, double channel);
    MIDI_IN_QUEUE(void);
};

extern MIDI_IN_QUEUE midi_in_queue;
// extern MIDI_OUT_QUEUE midi_out_queue;
extern pthread_mutex_t midi_in_lock;     // mutex ensuring exclusive access to MIDI-buffer

#endif
