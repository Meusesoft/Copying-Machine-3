/**********************************************************************
 * File:        tstruct.cpp  (Formerly tstruct.c)
 * Description: Code to manipulate the structures of the C++/C interface.
 * Author:		Ray Smith
 * Created:		Thu Apr 23 15:49:29 BST 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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

#include "mfcpch.h"
#include          "tfacep.h"
#include          "tstruct.h"
//#include "structures.h"

static ERRCODE BADFRAGMENTS = "Couldn't find matching fragment ends";

ELISTIZE (FRAGMENT)
//extern /*"C"*/ oldoutline(TESSLINE*);
/**********************************************************************
 * FRAGMENT::FRAGMENT
 *
 * Constructor for fragments.
 **********************************************************************/
FRAGMENT::FRAGMENT (             //constructor
EDGEPT * head_pt,                //start point
EDGEPT * tail_pt                 //end point
):head (head_pt->pos.x, head_pt->pos.y), tail (tail_pt->pos.x,
tail_pt->pos.y) {
  headpt = head_pt;              //save ptrs
  tailpt = tail_pt;
}


/**********************************************************************
 * make_ed_word
 *
 * Make an editor format word from the tess style word.
 **********************************************************************/

WERD *make_ed_word(                  //construct word
                   TWERD *tessword,  //word to convert
                   WERD *clone       //clone this one
                  ) {
  WERD *word;                    //converted word
  TBLOB *tblob;                  //current blob
  PBLOB *blob;                   //new blob
  PBLOB_LIST blobs;              //list of blobs
  PBLOB_IT blob_it = &blobs;     //iterator

  for (tblob = tessword->blobs; tblob != NULL; tblob = tblob->next) {
    blob = make_ed_blob (tblob);
    if (blob != NULL) {
      blob_it.add_after_then_move (blob);
    }
  }
  if (!blobs.empty ())
    word = new WERD (&blobs, clone);
  else
    word = NULL;
  return word;
}


/**********************************************************************
 * make_ed_blob
 *
 * Make an editor format blob from the tess style blob.
 **********************************************************************/

PBLOB *make_ed_blob(                 //construct blob
                    TBLOB *tessblob  //blob to convert
                   ) {
  TESSLINE *tessol;              //tess outline
  FRAGMENT_LIST fragments;       //list of fragments
  OUTLINE *outline;              //current outline
  OUTLINE_LIST out_list;         //list of outlines
  OUTLINE_IT out_it = &out_list; //iterator

  for (tessol = tessblob->outlines; tessol != NULL; tessol = tessol->next) {
                                 //stick in list
    register_outline(tessol, &fragments);
  }
  while (!fragments.empty ()) {
    outline = make_ed_outline (&fragments);
    if (outline != NULL) {
      out_it.add_after_then_move (outline);
    }
  }
  if (out_it.empty())
    return NULL;                 //couldn't do it
  return new PBLOB (&out_list);  //turn to blob
}


/**********************************************************************
 * make_ed_outline
 *
 * Make an editor format outline from the list of fragments.
 **********************************************************************/

OUTLINE *make_ed_outline(                     //constructoutline
                         FRAGMENT_LIST *list  //list of fragments
                        ) {
  FRAGMENT *fragment;            //current fragment
  EDGEPT *edgept;                //current point
  ICOORD headpos;                //coords of head
  ICOORD tailpos;                //coords of tail
  FCOORD pos;                    //coords of edgept
  FCOORD vec;                    //empty
  POLYPT *polypt;                //current point
  POLYPT_LIST poly_list;         //list of point
  POLYPT_IT poly_it = &poly_list;//iterator
  FRAGMENT_IT fragment_it = list;//fragment

  headpos = fragment_it.data ()->head;
  do {
    fragment = fragment_it.data ();
    edgept = fragment->headpt;   //start of segment
    do {
      pos = FCOORD (edgept->pos.x, edgept->pos.y);
      vec = FCOORD (edgept->vec.x, edgept->vec.y);
      polypt = new POLYPT (pos, vec);
                                 //add to list
      poly_it.add_after_then_move (polypt);
      edgept = edgept->next;
    }
    while (edgept != fragment->tailpt);
    tailpos = ICOORD (edgept->pos.x, edgept->pos.y);
                                 //get rid of it
    delete fragment_it.extract ();
    if (tailpos != headpos) {
      if (fragment_it.empty ()) {
        tprintf("Bad tailpos (%d,%d), Head=(%d,%d), no fragments.\n",
                fragment->head.x(),fragment->head.y(),
                headpos.x(),headpos.y());
        return NULL;
      }
      fragment_it.forward ();
                                 //find next segment
      for (fragment_it.mark_cycle_pt (); !fragment_it.cycled_list () &&
               fragment_it.data ()->head != tailpos;
        fragment_it.forward ());
      if (fragment_it.data ()->head != tailpos) {
        tprintf("Bad tailpos (%d,%d), Fragments are:\n",
                tailpos.x(),tailpos.y());
        for (fragment_it.mark_cycle_pt ();
        !fragment_it.cycled_list (); fragment_it.forward ()) {
          fragment = fragment_it.extract ();
          tprintf("Head=(%d,%d), tail=(%d,%d)\n",
                  fragment->head.x(),fragment->head.y(),
                  fragment->tail.x(),fragment->tail.y());
          delete fragment;
        }
        return NULL;             //can't do it
//         BADFRAGMENTS.error("make_ed_blob",ABORT,NULL);
      }
    }
  }
  while (tailpos != headpos);
  return new OUTLINE (&poly_it); //turn to outline
}


/**********************************************************************
 * register_outline
 *
 * Add the fragments in the given outline to the list
 **********************************************************************/

void register_outline(                     //add fragments
                      TESSLINE *outline,   //tess format
                      FRAGMENT_LIST *list  //list to add to
                     ) {
  EDGEPT *startpt;               //start of outline
  EDGEPT *headpt;                //start of fragment
  EDGEPT *tailpt;                //end of fragment
  FRAGMENT *fragment;            //new fragment
  FRAGMENT_IT it = list;         //iterator

  startpt = outline->loop;
  do {
    startpt = startpt->next;
    if (startpt == NULL)
      return;                    //illegal!
  }
  while (startpt->flags[0] == 0 && startpt != outline->loop);
  headpt = startpt;
  do
  startpt = startpt->next;
  while (startpt->flags[0] != 0 && startpt != headpt);
  if (startpt->flags[0] != 0)
    return;                      //all hidden!

  headpt = startpt;
  do {
    tailpt = headpt;
    do
    tailpt = tailpt->next;
    while (tailpt->flags[0] == 0 && tailpt != startpt);
    fragment = new FRAGMENT (headpt, tailpt);
    it.add_after_then_move (fragment);
    while (tailpt->flags[0] != 0)
      tailpt = tailpt->next;
    headpt = tailpt;
  }
  while (tailpt != startpt);
}


/**********************************************************************
 * convert_choice_lists
 *
 * Convert the ARRAY of TESS_LIST of TESS_CHOICEs into a BLOB_CHOICE_LIST.
 **********************************************************************/

void convert_choice_lists(                                 //convert lists
                          ARRAY tessarray,                 //list from tess
                          BLOB_CHOICE_LIST_CLIST *ratings  //list of results
                         ) {
  INT32 length;                  //elements in array
  INT32 index;                   //index to array
  LIST result;                   //tess output
                                 //iterator
  BLOB_CHOICE_LIST_C_IT it = ratings;
  BLOB_CHOICE_LIST *choice;      //created choice

  if (tessarray != NULL) {
    length = array_count (tessarray);
    for (index = 0; index < length; index++) {
      result = (LIST) array_value (tessarray, index);
                                 //make one
      choice = new BLOB_CHOICE_LIST;
                                 //convert blob choices
      convert_choice_list(result, *choice);
                                 //add to super list
      it.add_after_then_move (choice);
    }
    free_mem(tessarray);  //lists already freed
  }
}


/**********************************************************************
 * convert_choice_list
 *
 * Convert the LIST of TESS_CHOICEs into a BLOB_CHOICE_LIST.
 **********************************************************************/

void convert_choice_list(                           //convert lists
                         LIST list,                 //list from tess
                         BLOB_CHOICE_LIST &ratings  //list of results
                        ) {
  LIST result;                   //tess output
  BLOB_CHOICE_IT it = &ratings;  //iterator
  BLOB_CHOICE *choice;           //created choice
  A_CHOICE *tesschoice;          //choice to convert

  for (result = list; result != NULL; result = result->next) {
  //traverse list
    tesschoice = (A_CHOICE *) result->node;
                                 //make one
    choice = new BLOB_CHOICE (tesschoice->string, tesschoice->rating,
                              tesschoice->certainty, tesschoice->config);
    it.add_after_then_move (choice);
  }
  destroy_nodes (list, (void (*)(void *)) free_choice);
  //get rid of it
}


/**********************************************************************
 * make_tess_row
 *
 * Make a fake row structure to pass to the tesseract matchers.
 **********************************************************************/

void make_tess_row(                  //make fake row
                   DENORM *denorm,   //row info
                   TEXTROW *tessrow  //output row
                  ) {
  tessrow->baseline.segments = 1;
  tessrow->baseline.xstarts[0] = -32767;
  tessrow->baseline.xstarts[1] = 32767;
  tessrow->baseline.quads[0].a = 0;
  tessrow->baseline.quads[0].b = 0;
  tessrow->baseline.quads[0].c = bln_baseline_offset;
  tessrow->xheight.segments = 1;
  tessrow->xheight.xstarts[0] = -32767;
  tessrow->xheight.xstarts[1] = 32767;
  tessrow->xheight.quads[0].a = 0;
  tessrow->xheight.quads[0].b = 0;
  tessrow->xheight.quads[0].c = bln_x_height + bln_baseline_offset;
  tessrow->lineheight = bln_x_height;
  tessrow->ascrise = denorm->row ()->ascenders () * denorm->scale ();
  tessrow->descdrop = denorm->row ()->descenders () * denorm->scale ();
}


/**********************************************************************
 * make_tess_word
 *
 * Convert the word to Tess format.
 **********************************************************************/

TWERD *make_tess_word(              //convert owrd
                      WERD *word,   //word to do
                      TEXTROW *row  //fake row
                     ) {
  TWERD *tessword;               //tess format

  tessword = newword ();         //use old allocator
  tessword->row = row;           //give them something
                                 //copy string
  tessword->correct = strsave (word->text ());
  tessword->guess = NULL;
  tessword->blobs = make_tess_blobs (word->blob_list ());
  tessword->blanks = 1;
  tessword->blobcount = word->blob_list ()->length ();
  tessword->next = NULL;
  return tessword;
}


/**********************************************************************
 * make_tess_blobs
 *
 * Make Tess style blobs from a list of BLOBs.
 **********************************************************************/

TBLOB *make_tess_blobs(                      //make tess blobs
                       PBLOB_LIST *bloblist  //list to convert
                      ) {
  PBLOB_IT it = bloblist;        //iterator
  PBLOB *blob;                   //current blob
  TBLOB *head;                   //output list
  TBLOB *tail;                   //end of list
  TBLOB *tessblob;

  head = NULL;
  tail = NULL;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    blob = it.data ();
    tessblob = make_tess_blob (blob, TRUE);
    if (head)
      tail->next = tessblob;
    else
      head = tessblob;
    tail = tessblob;
  }
  return head;
}


/**********************************************************************
 * make_tess_blob
 *
 * Make a single Tess style blob
 **********************************************************************/

TBLOB *make_tess_blob(               //make tess blob
                      PBLOB *blob,   //blob to convert
                      BOOL8 flatten  //flatten outline structure
                     ) {
  INT32 index;
  TBLOB *tessblob;

  tessblob = newblob ();
  tessblob->outlines = (struct olinestruct *)
    make_tess_outlines (blob->out_list (), flatten);
  for (index = 0; index < TBLOBFLAGS; index++)
    tessblob->flags[index] = 0;  //!!
  tessblob->correct = 0;
  tessblob->guess = 0;
  for (index = 0; index < MAX_WO_CLASSES; index++) {
    tessblob->classes[index] = 0;
    tessblob->values[index] = 0;
  }
  tessblob->next = NULL;
  return tessblob;
}


/**********************************************************************
 * make_tess_outlines
 *
 * Make Tess style outlines from a list of OUTLINEs.
 **********************************************************************/

TESSLINE *make_tess_outlines(                            //make tess outlines
                             OUTLINE_LIST *outlinelist,  //list to convert
                             BOOL8 flatten               //flatten outline structure
                            ) {
  OUTLINE_IT it = outlinelist;   //iterator
  OUTLINE *outline;              //current outline
  TESSLINE *head;                //output list
  TESSLINE *tail;                //end of list
  TESSLINE *tessoutline;

  head = NULL;
  tail = NULL;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    tessoutline = newoutline ();
    tessoutline->compactloop = NULL;
    tessoutline->loop = make_tess_edgepts (outline->polypts (),
      tessoutline->topleft,
      tessoutline->botright);
    if (tessoutline->loop == NULL) {
      oldoutline(tessoutline);
      continue;
    }
    tessoutline->start = tessoutline->loop->pos;
    tessoutline->node = NULL;
    tessoutline->next = NULL;
    tessoutline->child = NULL;
    if (!outline->child ()->empty ()) {
      if (flatten)
        tessoutline->next = (struct olinestruct *)
          make_tess_outlines (outline->child (), flatten);
      else {
        tessoutline->next = NULL;
        tessoutline->child = (struct olinestruct *)
          make_tess_outlines (outline->child (), flatten);
      }
    }
    else
      tessoutline->next = NULL;
    if (head)
      tail->next = tessoutline;
    else
      head = tessoutline;
    while (tessoutline->next != NULL)
      tessoutline = tessoutline->next;
    tail = tessoutline;
  }
  return head;
}


/**********************************************************************
 * make_tess_edgepts
 *
 * Make Tess style edgepts from a list of POLYPTs.
 **********************************************************************/

EDGEPT *make_tess_edgepts(                          //make tess edgepts
                          POLYPT_LIST *edgeptlist,  //list to convert
                          TPOINT &tl,               //bounding box
                          TPOINT &br) {
  INT32 index;
  POLYPT_IT it = edgeptlist;     //iterator
  POLYPT *edgept;                //current edgept
  EDGEPT *head;                  //output list
  EDGEPT *tail;                  //end of list
  EDGEPT *tessedgept;

  head = NULL;
  tail = NULL;
  tl.x = MAX_INT16;
  tl.y = -MAX_INT16;
  br.x = -MAX_INT16;
  br.y = MAX_INT16;
  for (it.mark_cycle_pt (); !it.cycled_list ();) {
    edgept = it.data ();
    tessedgept = newedgept ();
    tessedgept->pos.x = (INT16) edgept->pos.x ();
    tessedgept->pos.y = (INT16) edgept->pos.y ();
    if (tessedgept->pos.x < tl.x)
      tl.x = tessedgept->pos.x;
    if (tessedgept->pos.x > br.x)
      br.x = tessedgept->pos.x;
    if (tessedgept->pos.y > tl.y)
      tl.y = tessedgept->pos.y;
    if (tessedgept->pos.y < br.y)
      br.y = tessedgept->pos.y;
    if (head != NULL && tessedgept->pos.x == tail->pos.x
    && tessedgept->pos.y == tail->pos.y) {
      oldedgept(tessedgept);
    }
    else {
      for (index = 0; index < EDGEPTFLAGS; index++)
        tessedgept->flags[index] = 0;
      if (head != NULL) {
        tail->vec.x = tessedgept->pos.x - tail->pos.x;
        tail->vec.y = tessedgept->pos.y - tail->pos.y;
        tessedgept->prev = tail;
      }
      tessedgept->next = head;
      if (head)
        tail->next = tessedgept;
      else
        head = tessedgept;
      tail = tessedgept;
    }
    it.forward ();
  }
  head->prev = tail;
  tail->vec.x = head->pos.x - tail->pos.x;
  tail->vec.y = head->pos.y - tail->pos.y;
  if (head == tail) {
    oldedgept(head);
    return NULL;                 //empty
  }
  return head;
}
