// ------------------------------------------------------------------------
// midiio-raw.cpp: Input and output of raw MIDI streams using standard 
//                 UNIX file operations.
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include "midiio-raw.h"

#include "eca-debug.h"

MIDI_IO_RAW::MIDI_IO_RAW(const string& name) { label(name); }

MIDI_IO_RAW::~MIDI_IO_RAW(void) { if (is_open()) close(); }

void MIDI_IO_RAW::open(void) { 
  int flags;

  switch(io_mode()) {
  case io_read:
    {
      flags = O_RDONLY;
      break;
    }
  case io_write: 
    {
      flags = O_WRONLY;
      break;
    }
  case io_readwrite: 
    {
      flags = O_RDWR;
      break;
    }
  }
  if (nonblocking_mode() == true) flags |= O_NONBLOCK;
  
  fd_rep = ::open(label().c_str(), flags);
  if (fd_rep < 0) {
    toggle_open_state(false);
  }
  else {
    toggle_open_state(true);
  }

  finished_rep = false;
}

void MIDI_IO_RAW::close(void) {
  ::close(fd_rep);
  toggle_open_state(false);
}

bool MIDI_IO_RAW::finished(void) const { return(finished_rep); }

long int MIDI_IO_RAW::read_bytes(void* target_buffer, long int bytes) {
  long int res = ::read(fd_rep, target_buffer, bytes);
  if (res >= 0) return(res);
  finished_rep = true;
  return(0);
}

long int MIDI_IO_RAW::write_bytes(void* target_buffer, long int bytes) {
  long int res = ::write(fd_rep, target_buffer, bytes);
  if (res >= 0) return(res);
  finished_rep = true;
  return(0);
}