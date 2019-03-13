/** example Intro/RRcommon.h
 */

/**
 *
 * file RRcommon.h Include file for the Solace C API samples.
 *
 * Copyright 2013-2019 Solace Corporation. All rights reserved.
 *
 * This include file provides common utilities used throughout the Request-Reply
 * samples.
 */

#ifndef _RRCOMMON_H_
#define _RRCOMMON_H_

#include "solclient/solClient.h"

typedef enum RR_operation
{
    firstOperation = 1,
    plusOperation = 1,
    minusOperation = 2,
    timesOperation = 3,
    divideOperation = 4,
    lastOperation = 4
} RR_operation_t;



#define MY_SAMPLE_REQUEST_TE "my_sample_request_te"


const char *RR_operationToString ( RR_operation_t operation );

#endif
