/**********************************************************************
 * File:        applybox.h  (Formerly applybox.h)
 * Description: Re segment rows according to box file data
 * Author:		Phil Cheatle
 * Created:		Wed Nov 24 09:11:23 GMT 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef           APPLYBOX_H
#define           APPLYBOX_H

#include          "varable.h"
#include          "ocrblock.h"
#include          "ocrrow.h"
#include          "notdll.h"
#include          "unichar.h"

extern BOOL_VAR_H (applybox_rebalance, TRUE, "Drop dead");
extern INT_VAR_H (applybox_debug, 0, "Debug level");
extern STRING_VAR_H (applybox_test_exclusions, "|",
"Chars ignored for testing");
extern double_VAR_H (applybox_error_band, 0.15, "Err band as fract of xht");
void apply_boxes(BLOCK_LIST *block_list    //real blocks
                );
void clear_any_old_text(                        //remove correct text
                        BLOCK_LIST *block_list  //real blocks
                       );
BOOL8 read_next_box(FILE* box_file,  //
                    BOX *box,
                    UNICHAR_ID *uch_id);
ROW *find_row_of_box(                         //
                     BLOCK_LIST *block_list,  //real blocks
                     BOX box,                 //from boxfile
                     INT16 &block_id,
                     INT16 &row_id_to_process);
INT16 resegment_box(  //
                    ROW *row,
                    BOX box,
                    UNICHAR_ID uch_id,
                    INT16 block_id,
                    INT16 row_id,
                    INT16 boxfile_lineno,
                    INT16 boxfile_charno);
void tidy_up(                         //
             BLOCK_LIST *block_list,  //real blocks
             INT16 &ok_char_count,
             INT16 &ok_row_count,
             INT16 &unlabelled_words,
             INT16 *tgt_char_counts,
             INT16 &rebalance_count,
             UNICHAR_ID *min_uch_id,
             INT16 &min_samples,
             INT16 &final_labelled_blob_count);
void report_failed_box(INT16 boxfile_lineno,
                       INT16 boxfile_charno,
                       BOX box,
                       const char *box_ch,
                       const char *err_msg);
void apply_box_training(BLOCK_LIST *block_list);
void apply_box_testing(BLOCK_LIST *block_list);
#endif
