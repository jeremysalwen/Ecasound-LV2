// ------------------------------------------------------------------------
// samplebuffer.cpp: Routines and classes for handling sample buffers.
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sys/types.h>

#include <kvutils.h>

#define SAMPLEBUFFER_GUARD
#include "samplebuffer.h"
#include "eca-debug.h"
#include "eca-error.h"

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

SAMPLE_SPECS::sample_type SAMPLE_BUFFER_OBSOLETE::max_value(int channel) {
  return(*max_element(buffer[channel].begin(), buffer[channel].end()));
}

SAMPLE_SPECS::sample_type SAMPLE_BUFFER_OBSOLETE::min_value(int channel) {
  return(*min_element(buffer[channel].begin(), buffer[channel].end()));
}

void SAMPLE_BUFFER_OBSOLETE::copy_from_buffer(unsigned char* target,
				     ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
				     int ch,
				     long int srate) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(target != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) number_of_channels(ch);
  if (srate != sample_rate_rep) resample_to(srate);

  assert(ch == channel_count_rep);


  long int osize = 0;

  for(long int isize = 0; isize < buffersize_rep; isize++) {
    switch (fmt) {
    case ECA_AUDIO_FORMAT::sfmt_u8:
      {
	for(int c = 0; c < ch; c++) {
	  // --- for debugging signal flow
	  //	  printf("(c %u)(isize %u)(osize %u) converting %.2f, \n",
	  //		   c, isize, osize, buffer[c][isize]);
	  SAMPLE_SPECS::sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  target[osize++] = (unsigned
			     char)((SAMPLE_SPECS::sample_type)(stemp / SAMPLE_SPECS::u8_to_st_constant) + SAMPLE_SPECS::u8_to_st_delta);
	  // --- for debugging signal flow
	  //	  printf("converted to u8 %u (hex:%x)\n", target[osize-1], target[osize-1]);
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s16_le:
      {
	for(int c = 0; c < ch; c++) {
	  SAMPLE_SPECS::sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int16_t s16temp = (int16_t)(SAMPLE_SPECS::sample_type)(stemp * SAMPLE_SPECS::s16_to_st_constant);
	  // --- for debugging signal flow
	  // if (isize == 0) 
	  //  printf("converted to s16 %d (hex:%x)", s16temp, (unsigned short int)s16temp);
	  // ------------------------------
	  
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).

	  target[osize++] = (unsigned char)(s16temp & 0xff);
	  target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s16_be:
      {
	for(int c = 0; c < ch; c++) {
	  SAMPLE_SPECS::sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int16_t s16temp = (int16_t)(SAMPLE_SPECS::sample_type)(stemp * SAMPLE_SPECS::s16_to_st_constant);

	  // --- for debugging signal flow
	  // if (isize == 0) 
	  //  printf("converted to s16 %d (hex:%x)", s16temp, (unsigned short int)s16temp);
	  // ------------------------------
	  
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).
	  // ---
	  target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s16temp & 0xff);
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s24_le:
      {
	for(int c = 0; c < ch; c++) {
	  SAMPLE_SPECS::sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int32_t s32temp = (int32_t)(SAMPLE_SPECS::sample_type)(stemp * SAMPLE_SPECS::s24_to_st_constant);

	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = 0;

  	  if (s32temp < 0) target[osize - 2] |=  0x80;
//    	  if (osize == 4) printf("neg.target: %x:%x:%x:%x.\n",target[osize-4],
//    				 target[osize-3],
//    				 target[osize-2],
//    				 target[osize-1])
//	  if (isize == 0) cerr << "ulos:" << (*(int32_t*)(target+osize-4)) << "|\n";
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s24_be:
      {
	for(int c = 0; c < ch; c++) {
	  SAMPLE_SPECS::sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int32_t s32temp = (int32_t)(SAMPLE_SPECS::sample_type)(stemp * SAMPLE_SPECS::s24_to_st_constant);

	  target[osize++] = 0;
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  
	  if (s32temp < 0) target[osize - 3] |= 0x80;
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s32_le:
      {
	for(int c = 0; c < ch; c++) {
	  SAMPLE_SPECS::sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int32_t s32temp = (int32_t)(SAMPLE_SPECS::sample_type)(stemp * SAMPLE_SPECS::s32_to_st_constant);

	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 24) & 0xff);
	}
      }
      break;
	
    case ECA_AUDIO_FORMAT::sfmt_s32_be:
      {
	for(int c = 0; c < ch; c++) {
	  SAMPLE_SPECS::sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int32_t s32temp = (int32_t)(SAMPLE_SPECS::sample_type)(stemp * SAMPLE_SPECS::s32_to_st_constant);

	  target[osize++] = (unsigned char)((s32temp >> 24) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	}
      }
      break;      

    default: 
      { 
	throw(new ECA_ERROR("SAMPLEBUFFER", "Unknown sample format! [c_to_b]."));
   
      }
    }
  }
}

void SAMPLE_BUFFER_OBSOLETE::copy_to_buffer(unsigned char* source,
				   long int samples_read,
				   ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
				   int ch,
				   long int srate) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(samples_read >= 0);
  assert(source != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) number_of_channels(ch);
  if (buffersize_rep != samples_read) resize(samples_read);

  assert(ch == channel_count_rep);
  assert(buffersize_rep == samples_read);

  unsigned char a[2];
  unsigned char b[4];
  long int isize = 0;

  for(long int osize = 0; osize < buffersize_rep; osize++) {
    switch (fmt) {
    case ECA_AUDIO_FORMAT::sfmt_u8: 
      {
	for(int c = 0; c < ch; c++) {
	  // --- for debugging signal flow
	  //	  if (osize == 0) 
	  // printf("converting to u8 %u\n", source[isize]);

	  buffer[c][osize] = (unsigned char)source[isize++];
	  buffer[c][osize] -= SAMPLE_SPECS::u8_to_st_delta;
	  buffer[c][osize] *= SAMPLE_SPECS::u8_to_st_constant;
	  // --- for debugging signal flow
	  //	  if (osize == 0) 
	  //	    printf("converted to %.2f.\n", buffer[c][osize]);
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s16_le:
      {
	for(int c = 0; c < ch; c++) {
	  // little endian: (LSB, MSB) (Intel)
	  // big endian: (MSB, LSB) (Motorola)
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    a[0] = source[isize++];
	    a[1] = source[isize++];
	  }
	  else {
	    a[1] = source[isize++];
	    a[0] = source[isize++];
	  }
	  buffer[c][osize] = (SAMPLE_SPECS::sample_type)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
	  // --- for debugging signal flow
	  //  	  if (osize == 0) {
	  //  	    printf(" ... converted to %d (hex:%x)...", (*(int16_t*)a), (*(int16_t*)a)); 
	  //  	    printf(" (a0: %d, a1: %d) ", a[0], a[1]);
	  //  	    printf(" ... and scaled to %.2f.\n", buffer[c][osize]);
	  //  	  }
	  // ------------------------------
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s16_be:
      {
	for(int c = 0; c < ch; c++) {
	  if (!SAMPLE_SPECS::is_system_littleendian) {
	    a[0] = source[isize++];
	    a[1] = source[isize++];
	  }
	  else {
	    a[1] = source[isize++];
	    a[0] = source[isize++];
	  }
	  buffer[c][osize] = (SAMPLE_SPECS::sample_type)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s24_le:
      {
	for(int c = 0; c < ch; c++) {
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    //	    if (osize == 0) cerr << "sis��n:" << (*(int32_t*)(source+isize)) << "|\n";
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  else {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  buffer[c][osize] = (SAMPLE_SPECS::sample_type)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
	  //	  if (osize == 0) cerr << "sis��n3:" << buffer[c][osize] << "|\n";
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s24_be:
      {
	for(int c = 0; c < ch; c++) {
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  else {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  buffer[c][osize] = (SAMPLE_SPECS::sample_type)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s32_le:
      {
	for(int c = 0; c < ch; c++) {
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  else {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  buffer[c][osize] = (SAMPLE_SPECS::sample_type)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s32_be:
      {
	for(int c = 0; c < ch; c++) {
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  else {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  buffer[c][osize] = (SAMPLE_SPECS::sample_type)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
	}
      }
      break;

    default: 
      { 
	throw(new ECA_ERROR("SAMPLEBUFFER", "Unknown sample format! [c_to_b]."));
      }
    }
  }
  if (srate != sample_rate_rep) resample_from(srate);
}

void SAMPLE_BUFFER_OBSOLETE::make_silent(void) {
  buf_channel_iter_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    buf_sample_iter_t p = buf_iter->begin();
    while(p != buf_iter->end()) {
      *p = SAMPLE_SPECS::silent_value;
      ++p;
    }
    ++buf_iter;
  }
}

void SAMPLE_BUFFER_OBSOLETE::make_silent_range(long int start_pos,
				      long int end_pos) {
  assert(start_pos >= 0);
  assert(end_pos >= 0);

  buf_channel_iter_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    for(long int s = start_pos;
	s < end_pos && s < static_cast<long int>(buf_iter->size());
	s++) {
      (*buf_iter)[s] = SAMPLE_SPECS::silent_value;
      ++s;
    }
    ++buf_iter;
  }
}

void SAMPLE_BUFFER_OBSOLETE::limit_values(void) {
  buf_channel_iter_t c = buffer.begin();
  while(c != buffer.end()) {
    buf_sample_iter_t s = c->begin();
    while(s != c->end()) {
      if ((*s) > SAMPLE_SPECS::impl_max_value)
	(*s) = SAMPLE_SPECS::impl_max_value;
      else if ((*s) < SAMPLE_SPECS::is_system_littleendian) 
	(*s) = SAMPLE_SPECS::is_system_littleendian;
      ++s;
    }
    ++c;
  }
}

void SAMPLE_BUFFER_OBSOLETE::divide_by(SAMPLE_SPECS::sample_type dvalue) {
  buf_channel_iter_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    buf_sample_iter_t p = buf_iter->begin();
    while(p != buf_iter->end()) {
      *p /= dvalue;
      ++p;
    }
    ++buf_iter;
  }
}

void SAMPLE_BUFFER_OBSOLETE::add(const SAMPLE_BUFFER_OBSOLETE& x) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (number_of_channels() <= x.number_of_channels()) ? number_of_channels() : x.number_of_channels();
  for(int q = 0; q != c_count; q++) {
    //    long int s_count = (buffer[q].size() <= x.buffer[q].size()) ? buffer[q].size() : x.buffer[q].size();
    for(long int t = 0; t != static_cast<long int>(x.buffer[q].size()); t++) {
      buffer[q][t] += x.buffer[q][t];
    }
  }
}

void SAMPLE_BUFFER_OBSOLETE::add_with_weight(const SAMPLE_BUFFER_OBSOLETE& x, int weight) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (number_of_channels() <= x.number_of_channels()) ? number_of_channels() : x.number_of_channels();
  for(int q = 0; q != c_count; q++) {
    //    long int s_count = (buffer[q].size() <= x.buffer[q].size()) ? buffer[q].size() : x.buffer[q].size();
    for(long int t = 0; t != static_cast<long int>(x.buffer[q].size()); t++) {
      buffer[q][t] += x.buffer[q][t] / weight;
    }
  }
}

void SAMPLE_BUFFER_OBSOLETE::copy(const SAMPLE_BUFFER_OBSOLETE& x) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (number_of_channels() <= x.number_of_channels()) ? number_of_channels() : x.number_of_channels();
  for(int q = 0; q != c_count; q++) {
    for(long int t = 0; t != static_cast<long int>(x.buffer[q].size()); t++) {
      buffer[q][t] = x.buffer[q][t];
    }
  }
}

void SAMPLE_BUFFER_OBSOLETE::copy_range(const SAMPLE_BUFFER_OBSOLETE& x, 
			       long int start_pos,
			       long int end_pos,
			       long int to_pos) {
  int c_count = (number_of_channels() <= x.number_of_channels()) ? number_of_channels() : x.number_of_channels();
  long int t = to_pos;

  assert(start_pos < end_pos);
  assert(to_pos < length_in_samples());

  if (start_pos >= x.length_in_samples()) start_pos = x.length_in_samples();
  if (end_pos >= x.length_in_samples()) end_pos = x.length_in_samples();

  for(int q = 0; q != c_count; q++) {
    for(long int s = start_pos; s != end_pos && t < static_cast<long int>(buffer[q].size()); s++) {
      buffer[q][t] = x.buffer[q][s];
      ++t;
    }
  }
}

SAMPLE_SPECS::sample_type SAMPLE_BUFFER_OBSOLETE::average_volume(void) {
  SAMPLE_SPECS::sample_type temp_avg = 0.0;
  buf_channel_citer_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    buf_sample_citer_t p = buf_iter->begin();
    while(p != buf_iter->end()) {
      temp_avg += fabs((*p) - SAMPLE_SPECS::silent_value);
      ++p;
    }
    ++buf_iter;
  }

  return(temp_avg / (SAMPLE_SPECS::sample_type)number_of_channels());
}

SAMPLE_SPECS::sample_type SAMPLE_BUFFER_OBSOLETE::average_RMS_volume(void) {
  SAMPLE_SPECS::sample_type temp_avg = 0.0;
  buf_channel_citer_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    buf_sample_citer_t p = buf_iter->begin();
    while(p != buf_iter->end()) {
      temp_avg += (*p) * (*p);
      ++p;
    }
    ++buf_iter;
  }
  return(sqrt(temp_avg / (SAMPLE_SPECS::sample_type)number_of_channels()));
}

SAMPLE_SPECS::sample_type SAMPLE_BUFFER_OBSOLETE::average_volume(int channel,
							 int count_samples) 
{
  SAMPLE_SPECS::sample_type temp_avg = 0.0;
  if (count_samples == 0) count_samples = (int)number_of_channels();

  buf_sample_citer_t p = buffer[channel].begin();
  while(p != buffer[channel].end()) {
    temp_avg += fabs(*p - SAMPLE_SPECS::silent_value);
    ++p;
  }

  return(temp_avg / (SAMPLE_SPECS::sample_type)count_samples);
}

SAMPLE_SPECS::sample_type SAMPLE_BUFFER_OBSOLETE::average_RMS_volume(int channel,
							     int count_samples) 
{
  SAMPLE_SPECS::sample_type temp_avg = 0.0;
  if (count_samples == 0) count_samples = (int)number_of_channels();
  buf_sample_citer_t p = buffer[channel].begin();
  while(p != buffer[channel].end()) {
    temp_avg += *p + *p;
    ++p;
  }
  return(temp_avg);
}

void SAMPLE_BUFFER_OBSOLETE::resample_from(long int from_srate) {
  resample_nofilter(from_srate, sample_rate_rep);
}

void SAMPLE_BUFFER_OBSOLETE::resample_to(long int to_srate) {
  resample_nofilter(sample_rate_rep, to_srate);
}

void SAMPLE_BUFFER_OBSOLETE::resample_nofilter(long int from, 
				      long int to) {
  double step = (double)to / from;
  buffersize_rep = static_cast<long int>(step * buffersize_rep);

  for(int c = 0; c < channel_count_rep; c++) {
    old_buffer = buffer[c];
    
    buffer[c].resize(buffersize_rep);
    
    double counter = 0.0;
    size_t newbuf_sizet = 0;
    size_t last_sizet = 0;
    
    if (old_buffer.size() == 0) continue;
    
    buffer[c][0] = old_buffer[0];
    for(long int buf_sizet = 1; buf_sizet < static_cast<long int>(old_buffer.size()); buf_sizet++) {
      counter += step;
      if (step <= 1) {
	if (counter >= newbuf_sizet + 1) {
	  newbuf_sizet++;
	  if (newbuf_sizet >= buffer[c].size()) break;
	  buffer[c][newbuf_sizet] = old_buffer[buf_sizet];
	  //  	buffer[c][newbuf_sizet] *= 0.5;
	  //  	buffer[c][newbuf_sizet] += (buffer[c][newbuf_sizet-1] * 0.5);
	}
      }
      else {
	newbuf_sizet = (size_t)ceil(counter);
	if (newbuf_sizet >= buffer[c].size()) break;
	for(size_t t = last_sizet + 1; t < newbuf_sizet; t++) {
	  buffer[c][t] = old_buffer[buf_sizet - 1] + ((old_buffer[buf_sizet]
						      - old_buffer[buf_sizet-1])
						     * static_cast<SAMPLE_SPECS::sample_type>(t - last_sizet)
						     / (newbuf_sizet - last_sizet));
	  //  	buffer[c][t] *= 0.5;
	  //  	buffer[c][t] += (buffer[c][t-1] * 0.5);
	}
	buffer[c][newbuf_sizet] = old_buffer[buf_sizet];
      }
//        buffer[c][newbuf_sizet] *= 0.5;
//        buffer[c][newbuf_sizet] += (buffer[c][newbuf_sizet-1] * 0.5);

      last_sizet = newbuf_sizet;
    }
  }
}

void SAMPLE_BUFFER_OBSOLETE::resample_extfilter(long int from_srate,
					long int to_srate) {}
void SAMPLE_BUFFER_OBSOLETE::resample_simplefilter(long int from_srate,
					  long int to_srate) { }

void SAMPLE_BUFFER_OBSOLETE::length_in_samples(long int len) {
  if (buffersize_rep != len) resize(len); 
}

void SAMPLE_BUFFER_OBSOLETE::number_of_channels(int len) {
  if (len > static_cast<long int>(buffer.size())) {
    buffer.resize(len, vector<SAMPLE_SPECS::sample_type> (buffersize_rep,
					    SAMPLE_SPECS::sample_type(0.0)));
    ecadebug->msg(ECA_DEBUG::system_objects, "(samplebuffer) Increasing channel-count.");    
  }
  channel_count_rep = len;
}

void SAMPLE_BUFFER_OBSOLETE::resize(long int buffersize) {
  buf_channel_iter_t p = buffer.begin();
  while(p != buffer.end()) {
    p->resize(buffersize);
    ++p;
  }
  buffersize_rep = buffersize;
}

SAMPLE_BUFFER_OBSOLETE::SAMPLE_BUFFER_OBSOLETE (long int buffersize, 
			      int channels, 
			      long int srate) 
  : channel_count_rep(channels),
    buffersize_rep(buffersize),
    sample_rate_rep(srate),
    buffer(channels, 
	   vector<SAMPLE_SPECS::sample_type> (buffersize, SAMPLE_SPECS::sample_type(0.0))) {

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(samplebuffer) Buffer created, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ", sample rate: " +
		kvu_numtostr(sample_rate_rep) + ".");
}

SAMPLE_BUFFER_OBSOLETE::~SAMPLE_BUFFER_OBSOLETE (void) { }

SAMPLE_BUFFER_OBSOLETE& SAMPLE_BUFFER_OBSOLETE::operator=(const SAMPLE_BUFFER_OBSOLETE& x) {
  // ---
  // For better performance, doesn't copy IO-buffers nor
  // iterator state.
  // ---
  
  if (this != &x) {
    //    if (number_of_channels() != x.number_of_channels()) resize(x.number_of_channels());
    buffer = x.buffer;
    buffersize_rep = x.buffersize_rep;
    channel_count_rep = x.channel_count_rep;
    sample_rate_rep = x.sample_rate_rep;
  }
  return *this;
}

SAMPLE_BUFFER_OBSOLETE::SAMPLE_BUFFER_OBSOLETE (const SAMPLE_BUFFER_OBSOLETE& x)
  : channel_count_rep(x.channel_count_rep),
    buffersize_rep(x.buffersize_rep),
    sample_rate_rep(x.sample_rate_rep),
    buffer(x.buffer)
{
  // ---
  // For better performance, doesn't copy IO-buffers.
  // ---

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(samplebuffer) Buffer copy-constructed, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ", sample rate: " +
		kvu_numtostr(sample_rate_rep) + ".");
}
