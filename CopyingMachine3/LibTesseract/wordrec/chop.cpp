/* -*-C-*-
 ********************************************************************************
 *
 * File:        chop.c  (Formerly chop.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul 30 16:41:11 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 *********************************************************************************/

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "chop.h"
#include "debug.h"
#include "outlines.h"
#include "olutil.h"
#include "tordvars.h"
#include "callcpp.h"
#include "plotedges.h"
#include "const.h"

#include <math.h>

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
make_int_var (chop_debug, 0, make_chop_debug,
3, 1, set_chop_debug, "Chop debug");

make_int_var (chop_enable, 1, make_chop_enable,
3, 2, set_chop_enable, "Chop enable");

make_toggle_var (vertical_creep, 0, make_vertical_creep,
3, 4, set_vertical_creep, "Vertical creep");

make_int_var (split_length, 10000, make_split_length,
3, 5, set_split_length, "Split Length");

make_int_var (same_distance, 2, make_same_distance,
3, 6, set_same_distance, "Same distance");

make_int_var (min_outline_points, 6, make_min_points,
3, 9, set_min_points, "Min Number of Points on Outline");

make_int_var (inside_angle, -50, make_inside_angle,
3, 12, set_inside_angle, "Min Inside Angle Bend");

make_int_var (min_outline_area, 2000, make_outline_area,
3, 13, set_outline_area, "Min Outline Area");

/*----------------------------------------------------------------------
              V a r i a b l e s (moved from gradechop)
----------------------------------------------------------------------*/
make_float_var (split_dist_knob, 0.5, make_split_dist,
3, 17, set_split_dist, "Split length adjustment");

make_float_var (overlap_knob, 0.9, make_overlap_knob,
3, 18, set_overlap_knob, "Split overlap adjustment");

make_float_var (center_knob, 0.15, make_center_knob,
3, 19, set_center_knob, "Split center adjustment");

make_float_var (sharpness_knob, 0.06, make_sharpness_knob,
3, 20, set_sharpness_knob, "Split sharpness adjustment");

make_float_var (width_change_knob, 5.0, make_width_change,
3, 21, set_width_change_knob, "Width change adjustment");

make_float_var (ok_split, 100.0, make_ok_split,
3, 14, set_ok_split, "OK split limit");

make_float_var (good_split, 50.0, make_good_split,
3, 15, set_good_split, "Good split limit");

make_int_var (x_y_weight, 3, make_x_y_weight,
3, 16, set_x_y_weight, "X / Y  length weight");

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * length_product
 *
 * Compute the product of the length of two vectors.  The
 * vectors must be of type POINT.   This product is used in computing
 * angles.
 **********************************************************************/
#define length_product(p1,p2)                                      \
(sqrt ((((float) (p1).x * (p1).x + (float) (p1).y * (p1).y) *    \
			((float) (p2).x * (p2).x + (float) (p2).y * (p2).y))))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * point_priority
 *
 * Assign a priority to and edge point that might be used as part of a
 * split. The argument should be of type EDGEPT.
 **********************************************************************/
PRIORITY point_priority(EDGEPT *point) {
  return ((PRIORITY) point_bend_angle (point));
}


/**********************************************************************
 * add_point_to_list
 *
 * Add an edge point to a POINT_GROUP containg a list of other points.
 **********************************************************************/
void add_point_to_list(POINT_GROUP point_list, EDGEPT *point) {
  HEAPENTRY data;

  if (SizeOfHeap (point_list) < MAX_NUM_POINTS - 2) {
    data.Data = (char *) point;
    data.Key = point_priority (point);
    HeapStore(point_list, &data);
  }

#ifndef GRAPHICS_DISABLED
  if (chop_debug > 2)
    mark_outline(point);
#endif
}


/**********************************************************************
 * angle_change
 *
 * Return the change in angle (degrees) of the line segments between
 * points one and two, and two and three.
 **********************************************************************/
int angle_change(EDGEPT *point1, EDGEPT *point2, EDGEPT *point3) {
  VECTOR vector1;
  VECTOR vector2;

  int angle;
  float length;

  /* Compute angle */
  vector1.x = point2->pos.x - point1->pos.x;
  vector1.y = point2->pos.y - point1->pos.y;
  vector2.x = point3->pos.x - point2->pos.x;
  vector2.y = point3->pos.y - point2->pos.y;
  /* Use cross product */
  length = length_product (vector1, vector2);
  if ((int) length == 0)
    return (0);
  angle = static_cast<int>(floor(asin(CROSS (vector1, vector2) /
                                      length) / PI * 180.0 + 0.5));

  /* Use dot product */
  if (SCALAR (vector1, vector2) < 0)
    angle = 180 - angle;
  /* Adjust angle */
  if (angle > 180)
    angle -= 360;
  if (angle <= -180)
    angle += 360;
  return (angle);
}


/**********************************************************************
 * init_chop
 *
 * Create the required chopper variables.
 **********************************************************************/
void init_chop() {
  make_same_distance();
  make_vertical_creep();
  make_x_y_weight();
  make_chop_enable();
  make_chop_debug();
  make_split_dist();
  make_overlap_knob();
  make_sharpness_knob();
  make_width_change();
  make_good_split();
  make_ok_split();
  make_center_knob();
  make_split_length();
  make_min_points();
  make_inside_angle();
  make_outline_area();
}


/**********************************************************************
 * is_little_chunk
 *
 * Return TRUE if one of the pieces resulting from this split would
 * less than some number of edge points.
 **********************************************************************/
int is_little_chunk(EDGEPT *point1, EDGEPT *point2) {
  EDGEPT *p = point1;            /* Iterator */
  int counter = 0;

  do {
                                 /* Go from P1 to P2 */
    if (is_same_edgept (point2, p)) {
      if (is_small_area (point1, point2))
        return (TRUE);
      else
        break;
    }
    p = p->next;
  }
  while ((p != point1) && (counter++ < min_outline_points));
  /* Go from P2 to P1 */
  p = point2;
  counter = 0;
  do {
    if (is_same_edgept (point1, p)) {
      return (is_small_area (point2, point1));
    }
    p = p->next;
  }
  while ((p != point2) && (counter++ < min_outline_points));

  return (FALSE);
}


/**********************************************************************
 * is_small_area
 *
 * Test the area defined by a split accross this outline.
 **********************************************************************/
int is_small_area(EDGEPT *point1, EDGEPT *point2) {
  EDGEPT *p = point1->next;      /* Iterator */
  int area = 0;
  TPOINT origin;

  do {
                                 /* Go from P1 to P2 */
    origin.x = p->pos.x - point1->pos.x;
    origin.y = p->pos.y - point1->pos.y;
    area += CROSS (origin, p->vec);
    p = p->next;
  }
  while (!is_same_edgept (point2, p));

  return (area < min_outline_area);
}


/**********************************************************************
 * pick_close_point
 *
 * Choose the edge point that is closest to the critical point.  This
 * point may not be exactly vertical from the critical point.
 **********************************************************************/
EDGEPT *pick_close_point(EDGEPT *critical_point,
                         EDGEPT *vertical_point,
                         int *best_dist) {
  EDGEPT *best_point = NULL;
  int this_distance;
  int found_better;

  do {
    found_better = FALSE;

    this_distance = edgept_dist (critical_point, vertical_point);
    if (this_distance <= *best_dist) {

      if (!(same_point (critical_point->pos, vertical_point->pos) ||
        same_point (critical_point->pos, vertical_point->next->pos)
        || best_point != NULL
        && same_point (best_point->pos, vertical_point->pos) ||
      is_exterior_point (critical_point, vertical_point))) {
        *best_dist = this_distance;
        best_point = vertical_point;
        if (vertical_creep)
          found_better = TRUE;
      }
    }
    vertical_point = vertical_point->next;
  }
  while (found_better == TRUE);

  return (best_point);
}


/**********************************************************************
 * prioritize_points
 *
 * Find a list of edge points from the outer outline of this blob.  For
 * each of these points assign a priority.  Sort these points using a
 * heap structure so that they can be visited in order.
 **********************************************************************/
void prioritize_points(TESSLINE *outline, POINT_GROUP points) {
  EDGEPT *this_point;
  EDGEPT *local_min = NULL;
  EDGEPT *local_max = NULL;

  this_point = outline->loop;
  local_min = this_point;
  local_max = this_point;
  do {
    if (debug_5)
      cprintf ("(%3d,%3d)  min=%3d, max=%3d, dir=%2d, ang=%2.0f\n",
        this_point->pos.x, this_point->pos.y,
        (local_min ? local_min->pos.y : 999),
      (local_max ? local_max->pos.y : 999),
      direction (this_point), point_priority (this_point));

    if (this_point->vec.y < 0) {
                                 /* Look for minima */
      if (local_max != NULL)
        new_max_point(local_max, points);
      else if (is_inside_angle (this_point))
        add_point_to_list(points, this_point);
      local_max = NULL;
      local_min = this_point->next;
    }
    else if (this_point->vec.y > 0) {
                                 /* Look for maxima */
      if (local_min != NULL)
        new_min_point(local_min, points);
      else if (is_inside_angle (this_point))
        add_point_to_list(points, this_point);
      local_min = NULL;
      local_max = this_point->next;
    }
    else {
      /* Flat area */
      if (local_max != NULL) {
        if (local_max->prev->vec.y != 0) {
          new_max_point(local_max, points);
        }
        local_max = this_point->next;
        local_min = NULL;
      }
      else {
        if (local_min->prev->vec.y != 0) {
          new_min_point(local_min, points);
        }
        local_min = this_point->next;
        local_max = NULL;
      }
    }

                                 /* Next point */
    this_point = this_point->next;
  }
  while (this_point != outline->loop);
}


/**********************************************************************
 * new_min_point
 *
 * Found a new minimum point try to decide whether to save it or not.
 * Return the new value for the local minimum.  If a point is saved then
 * the local minimum is reset to NULL.
 **********************************************************************/
void new_min_point(EDGEPT *local_min, POINT_GROUP points) {
  INT16 dir;

  dir = direction (local_min);

  if (dir < 0) {
    add_point_to_list(points, local_min);
    return;
  }

  if (dir == 0 && point_priority (local_min) < 0) {
    add_point_to_list(points, local_min);
    return;
  }
}


/**********************************************************************
 * new_max_point
 *
 * Found a new minimum point try to decide whether to save it or not.
 * Return the new value for the local minimum.  If a point is saved then
 * the local minimum is reset to NULL.
 **********************************************************************/
void new_max_point(EDGEPT *local_max, POINT_GROUP points) {
  INT16 dir;

  dir = direction (local_max);

  if (dir > 0) {
    add_point_to_list(points, local_max);
    return;
  }

  if (dir == 0 && point_priority (local_max) < 0) {
    add_point_to_list(points, local_max);
    return;
  }
}


/**********************************************************************
 * vertical_projection_point
 *
 * For one point on the outline, find the corresponding point on the
 * other side of the outline that is a likely projection for a split
 * point.  This is done by iterating through the edge points until the
 * X value of the point being looked at is greater than the X value of
 * the split point.  Ensure that the point being returned is not right
 * next to the split point.  Return the edge point as a result.
 **********************************************************************/
void vertical_projection_point(EDGEPT *split_point, EDGEPT *target_point,
                               EDGEPT** best_point) {
  EDGEPT *p;                     /* Iterator */
  EDGEPT *this_edgept;           /* Iterator */
  int x = split_point->pos.x;    /* X value of vertical */
  int best_dist = LARGE_DISTANCE;/* Best point found */

  if (*best_point != NULL)
    best_dist = edgept_dist(split_point, *best_point);

  p = target_point;
  /* Look at each edge point */
  do {
    if ((((p->pos.x <= x) && (x <= p->next->pos.x)) ||
      ((p->next->pos.x <= x) && (x <= p->pos.x))) &&
      !same_point (split_point->pos, p->pos) &&
      !same_point (split_point->pos, p->next->pos)
    && (*best_point == NULL || !same_point ((*best_point)->pos, p->pos))) {

      this_edgept = near_point (split_point, p, p->next);

      if (*best_point == NULL)
        best_dist = edgept_dist (split_point, this_edgept);

      this_edgept =
        pick_close_point(split_point, this_edgept, &best_dist);
      if (this_edgept)
        *best_point = this_edgept;
    }

    p = p->next;
  }
  while (p != target_point);
}
