/**********************************************************************
 * File:        mainblk.c  (Formerly main.c)
 * Description: Function to call from main() to setup.
 * Author:					Ray Smith
 * Created:					Tue Oct 22 11:09:40 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#define BLOB_MATCHING_ON

#include "mfcpch.h"
#include          "fileerr.h"
#ifdef __UNIX__
#include          <unistd.h>
#include          <signal.h>
#else
#include          <io.h>
#endif
#include          <stdlib.h>
#include          "basedir.h"
#include          "mainblk.h"

#define VARDIR        "configs/" /*variables files */
#define EXTERN

EXTERN DLLSYM STRING datadir;    //dir for data files
                                 //name of image
EXTERN DLLSYM STRING imagebasename;
EXTERN BOOL_VAR (m_print_variables, FALSE,
"Print initial values of all variables");
EXTERN STRING_VAR (m_data_sub_dir, "tessdata/", "Directory for data files");
EXTERN INT_VAR (memgrab_size, 13000000, "Preallocation size for batch use");
const ERRCODE NO_PATH =
"Warning:explicit path for executable will not be used for configs";
static const ERRCODE USAGE = "Usage";

/**********************************************************************
 * main_setup
 *
 * Main for mithras demo program. Read the arguments and set up globals.
 **********************************************************************/

void main_setup(                         /*main demo program */
                const char *argv0,       //program name
                const char *basename,    //name of image
                int argc,                /*argument count */
                const char *const *argv  /*arguments */
               ) {
  INT32 arg;                     /*argument */
  INT32 offset;                  //for flag
  FILE *fp;                      /*variables file */
  char flag[2];                  //+/-
  STRING varfile;                /*name of file */

  imagebasename = basename;      /*name of image */

  if(!getenv("TESSDATA_PREFIX")) {
  #ifdef TESSDATA_PREFIX
  #define _STR(a) #a
  #define _XSTR(a) _STR(a)
  datadir = _XSTR(TESSDATA_PREFIX);
  #undef _XSTR
  #undef _STR
  #else
  if (getpath (argv0, datadir) < 0)
  #ifdef __UNIX__
    CANTOPENFILE.error ("main", ABORT, "%s to get path", argv[0]);
  #else
  NO_PATH.error ("main", DBG, NULL);
  #endif
  #endif
  } else {
    datadir = getenv("TESSDATA_PREFIX");
  }

  for (arg = 0; arg < argc; arg++) {
    if (argv[arg][0] == '+' || argv[arg][0] == '-') {
      offset = 1;
      flag[0] = argv[arg][0];
    }
    else {
      offset = 0;
    }
    flag[offset] = '\0';
    varfile = flag;
                                 /*attempt open */
    fp = fopen (argv[arg] + offset, "r");
    if (fp != NULL) {
      fclose(fp);  /*was only to test */
    }
    else {
      varfile += datadir;
      varfile += m_data_sub_dir; /*data directory */
      varfile += VARDIR;         /*variables dir */
    }
                                 /*actual name */
    varfile += argv[arg] + offset;
    read_variables_file (varfile.string ());
  }

  if (m_print_variables)
    print_variables(stdout);  /*print them all */


  datadir += m_data_sub_dir;     /*data directory */

  #ifdef __UNIX__
  if (memgrab_size > 0) {
    void *membuf;                //test virtual mem
                                 //test memory
    membuf = malloc (memgrab_size);
    if (membuf == NULL) {
      raise(SIGTTOU);  //hangup for jobber
      sleep (10);
    }
    else
      free(membuf);
  }
  #endif
}
