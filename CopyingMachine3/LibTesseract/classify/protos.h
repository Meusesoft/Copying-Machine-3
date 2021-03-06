/* -*-C-*-
 ********************************************************************************
 *
 * File:        protos.h  (Formerly protos.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri Jul 12 10:06:55 1991 (Dan Johnson) danj@hpgrlj
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
#ifndef PROTOS_H
#define PROTOS_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "bitvec.h"
#include "cutil.h"
#include "unichar.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef BIT_VECTOR *CONFIGS;

typedef struct
{
  FLOAT32 A;
  FLOAT32 B;
  FLOAT32 C;
  FLOAT32 X;
  FLOAT32 Y;
  FLOAT32 Angle;
  FLOAT32 Length;
} PROTO_STRUCT;
typedef PROTO_STRUCT *PROTO;

typedef struct
{
  INT16 NumProtos;
  INT16 MaxNumProtos;
  PROTO Prototypes;
  INT16 NumConfigs;
  INT16 MaxNumConfigs;
  CONFIGS Configurations;
} CLASS_STRUCT;
typedef CLASS_STRUCT *CLASS_TYPE;
typedef CLASS_STRUCT *CLASSES;

/*----------------------------------------------------------------------
              C o n s t a n t s
----------------------------------------------------------------------*/
#define NUMBER_OF_CLASSES  MAX_NUM_CLASSES
#define Y_OFFSET           -40.0
#define FEATURE_SCALE      100.0

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern CLASS_STRUCT TrainingData[];

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * AddProtoToConfig
 *
 * Set a single proto bit in the specified configuration.
 **********************************************************************/

#define AddProtoToConfig(Pid,Config)	\
(SET_BIT (Config, Pid))

/**********************************************************************
 * RemoveProtoFromConfig
 *
 * Clear a single proto bit in the specified configuration.
 **********************************************************************/

#define RemoveProtoFromConfig(Pid,Config)	\
(reset_bit (Config, Pid))

/**********************************************************************
 * ClassOfChar
 *
 * Return the class of a particular ASCII character value.
 **********************************************************************/

#define ClassOfChar(Char)            \
((TrainingData [Char].NumProtos) ? \
	(& TrainingData [Char])         : \
	NO_CLASS)

/**********************************************************************
 * ProtoIn
 *
 * Choose the selected prototype in this class record.  Return the
 * pointer to it (type PROTO).
 **********************************************************************/

#define ProtoIn(Class,Pid)  \
(& (Class)->Prototypes [Pid])

/**********************************************************************
 * ConfigIn
 *
 * Choose the selected prototype configuration in this class record.
 * Return it as type 'BIT_VECTOR'.
 **********************************************************************/

#define ConfigIn(Class,Cid)  \
((Class)->Configurations [Cid])

/**********************************************************************
 * NumProtosIn
 *
 * Return the number of prototypes in this class.  The 'Class' argument
 * is of type 'CLASS_TYPE'.
 **********************************************************************/

#define NumProtosIn(Class)  \
((Class)->NumProtos)

/**********************************************************************
 * NumConfigsIn
 *
 * Return the number of configurations in this class.  The 'Class' argument
 * is of type 'CLASS_TYPE'.
 **********************************************************************/

#define NumConfigsIn(Class)  \
((Class)->NumConfigs)

/**********************************************************************
 * CoefficientA
 *
 * Return the first parameter of the prototype structure.  This is the
 * A coefficient in the line representation of "Ax + By + C = 0".  The
 * 'Proto' argument is of type 'PROTO'.
 **********************************************************************/

#define CoefficientA(Proto)  \
((Proto)->A)

/**********************************************************************
 * CoefficientB
 *
 * Return the second parameter of the prototype structure.  This is the
 * B coefficient in the line representation of "Ax + By + C = 0".  The
 * 'Proto' argument is of type 'PROTO'.
 **********************************************************************/

#define CoefficientB(Proto)  \
((Proto)->B)

/**********************************************************************
 * CoefficientC
 *
 * Return the third parameter of the prototype structure.  This is the
 * C coefficient in the line representation of "Ax + By + C = 0".  The
 * 'Proto' argument is of type 'PROTO'.
 **********************************************************************/

#define CoefficientC(Proto)  \
((Proto)->C)

/**********************************************************************
 * ProtoAngle
 *
 * Return the angle parameter of the prototype structure.  The
 * 'Proto' argument is of type 'PROTO'.
 **********************************************************************/

#define ProtoAngle(Proto)  \
((Proto)->Angle)

/**********************************************************************
 * ProtoX
 *
 * Return the X parameter of the prototype structure.  The 'Proto'
 * argument is of type 'PROTO'.
 **********************************************************************/

#define ProtoX(Proto)  \
((Proto)->X)

/**********************************************************************
 * ProtoY
 *
 * Return the angle parameter of the prototype structure.  The 'Proto'
 * argument is of type 'PROTO'.
 **********************************************************************/

#define ProtoY(Proto)  \
((Proto)->Y)

/**********************************************************************
 * ProtoLength
 *
 * Return the length parameter of the prototype structure.  The
 * 'Proto' argument is of type 'PROTO'.
 **********************************************************************/

#define  ProtoLength(Proto)  \
((Proto)->Length)

/**********************************************************************
 * PrintProto
 *
 * Print out the contents of a prototype.   The 'Proto' argument is of
 * type 'PROTO'.
 **********************************************************************/

#define PrintProto(Proto)                      \
(cprintf ("X=%4.2f, Y=%4.2f, Angle=%4.2f",    \
			ProtoX      (Proto),                \
			ProtoY      (Proto),                \
			ProtoLength (Proto),                \
			ProtoAngle  (Proto)))                \


/**********************************************************************
 * PrintProtoLine
 *
 * Print out the contents of a prototype.   The 'Proto' argument is of
 * type 'PROTO'.
 **********************************************************************/

#define PrintProtoLine(Proto)             \
(cprintf ("A=%4.2f, B=%4.2f, C=%4.2f",   \
			CoefficientA (Proto),           \
			CoefficientB (Proto),           \
			CoefficientC (Proto)))           \

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
int AddConfigToClass(CLASS_TYPE Class);

int AddProtoToClass(CLASS_TYPE Class);

FLOAT32 ClassConfigLength(CLASS_TYPE Class, BIT_VECTOR Config);

FLOAT32 ClassProtoLength(CLASS_TYPE Class);

void CopyProto(PROTO Src, PROTO Dest);

void FillABC(PROTO Proto);

void FreeClass(CLASS_TYPE Class);

void FreeClassFields(CLASS_TYPE Class);

void InitPrototypes();

CLASS_TYPE NewClass(int NumProtos, int NumConfigs);

void PrintProtos(CLASS_TYPE Class);

void ReadClassFile();

void ReadClassFromFile(FILE *File, UNICHAR_ID unichar_id);

void ReadConfigs(register FILE *File, CLASS_TYPE Class);

void ReadProtos(register FILE *File, CLASS_TYPE Class);

int SplitProto(CLASS_TYPE Class, int OldPid);

void WriteOldConfigFile(FILE *File, CLASS_TYPE Class);

void WriteOldProtoFile(FILE *File, CLASS_TYPE Class);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif*/

/* protos.c *
int AddConfigToClass
  _ARGS((CLASS_TYPE Class));

int AddProtoToClass
  _ARGS((CLASS_TYPE Class));

FLOAT32 ClassConfigLength
  _ARGS((CLASS_TYPE Class,
  BIT_VECTOR Config));

FLOAT32 ClassProtoLength
  _ARGS((CLASS_TYPE Class));

void CopyProto
  _ARGS((PROTO Src,
  PROTO Dest));

void FillABC
  _ARGS((PROTO Proto));

void FreeClass
  _ARGS((CLASS_TYPE Class));

void FreeClassFields
  _ARGS((CLASS_TYPE Class));

void InitPrototypes
  _ARGS((void));

CLASS_TYPE NewClass
  _ARGS((int NumProtos,
  int NumConfigs));

void PrintProtos
  _ARGS((CLASS_TYPE Class));

void ReadClassFile
  _ARGS((void));

void ReadClassFromFile
  _ARGS((FILE *File,
  int ClassChar));

void ReadConfigs
  _ARGS((FILE *File,
  CLASS_TYPE Class));

void ReadProtos
  _ARGS((FILE *File,
  CLASS_TYPE Class));

int SplitProto
  _ARGS((CLASS_TYPE Class,
  int OldPid));

void WriteOldConfigFile
  _ARGS((FILE *File,
  CLASS_TYPE Class));

void WriteOldProtoFile
  _ARGS((FILE *File,
  CLASS_TYPE Class));

#undef _ARGS
*/
#endif
