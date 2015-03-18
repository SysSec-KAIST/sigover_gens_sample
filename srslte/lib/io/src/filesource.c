/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2014 The libLTE Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution.
 *
 * \section LICENSE
 *
 * This file is part of the libLTE library.
 *
 * libLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "srslte/io/filesource.h"

int filesource_init(filesource_t *q, char *filename, data_type_t type) {
  bzero(q, sizeof(filesource_t));
  q->f = fopen(filename, "r");
  if (!q->f) {
    perror("fopen");
    return -1;
  }
  q->type = type;
  return 0;
}

void filesource_free(filesource_t *q) {
  if (q->f) {
    fclose(q->f);
  }
  bzero(q, sizeof(filesource_t));
}

void filesource_seek(filesource_t *q, int pos) {
  fseek(q->f, pos, SEEK_SET);
}

int read_complex_f(FILE *f, _Complex float *y) {
  char in_str[64];
  _Complex float x;
  if (NULL == fgets(in_str, 64, f)) {
    return -1;
  } else {
    if (index(in_str, 'i') || index(in_str, 'j')) {
      sscanf(in_str,"%f%fi",&(__real__ x),&(__imag__ x));
    } else {
      __imag__ x = 0;
      sscanf(in_str,"%f",&(__real__ x));
    }
    *y = x;
    return 0;
  }
}

int filesource_read(filesource_t *q, void *buffer, int nsamples) {
  int i;
  float *fbuf = (float*) buffer;
  _Complex float *cbuf = (_Complex float*) buffer;
  _Complex short *sbuf = (_Complex short*) buffer;
  int size;

  switch(q->type) {
  case FLOAT:
    for (i=0;i<nsamples;i++) {
      if (EOF == fscanf(q->f,"%g\n",&fbuf[i]))
        break;
    }
    break;
  case COMPLEX_FLOAT:
    for (i=0;i<nsamples;i++) {
      if (read_complex_f(q->f, &cbuf[i])) {
        break;
      }
    }
    break;
  case COMPLEX_SHORT:
    for (i=0;i<nsamples;i++) {
      if (EOF == fscanf(q->f,"%hd%hdi\n",&(__real__ sbuf[i]),&(__imag__ sbuf[i])))
        break;
    }
    break;
  case FLOAT_BIN:
  case COMPLEX_FLOAT_BIN:
  case COMPLEX_SHORT_BIN:
    if (q->type == FLOAT_BIN) {
      size = sizeof(float);
    } else if (q->type == COMPLEX_FLOAT_BIN) {
      size = sizeof(_Complex float);
    } else if (q->type == COMPLEX_SHORT_BIN) {
      size = sizeof(_Complex short);
    }
    return fread(buffer, size, nsamples, q->f);
    break;
  default:
    i = -1;
    break;
  }
  return i;
}


int filesource_initialize(filesource_hl* h) {
  return filesource_init(&h->obj, h->init.file_name, h->init.data_type);
}

int filesource_work(filesource_hl* h) {
  h->out_len = filesource_read(&h->obj, h->output, h->ctrl_in.nsamples);
  if (h->out_len < 0) {
    return -1;
  }
  return 0;
}

int filesource_stop(filesource_hl* h) {
  filesource_free(&h->obj);
  return 0;
}
