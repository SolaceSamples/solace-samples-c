/** @example Intro/BasicRequestor.c
 */

/*
 * This sample shows how to implement a Requestor for direct Request-Reply messaging, where 
 *
 *    BasicRequestor: A message Endpoint that sends a request message and waits to receive
 *                       a reply message as a response.
 *    BasicReplier:   A message Endpoint that waits to receive a request message and responses
 *                       to it by sending a reply message.
 * 
 *  |----------------|  ---RequestTopic --> |---------------|
 *  | BasicRequestor |                      | BasicReplier  |
 *  |----------------|  <--ReplyToTopic---- |---------------|
 *
 * Copyright 2013-2019 Solace Corporation. All rights reserved.
 *
 */

/**************************************************************************
 *  For Windows builds, os.h should always be included first to ensure that
 *  _WIN32_WINNT is defined before winsock2.h or windows.h get included.
 **************************************************************************/
#include "os.h"
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"
#include "common.h"
#include "RRcommon.h"


/********************** Send Blocking  Requests********************************
 * Send a blocking request message for each of the four operation types, and
 * print out the results when the reply is received.
 ******************************************************************************/
static void
sendRequests ( solClient_opaqueSession_pt opaqueSession_p, const char *destinationName )
{
    solClient_returnCode_t rc;
    solClient_opaqueMsg_pt msg_p;
    solClient_opaqueMsg_pt replyMsg_p;
    solClient_destination_t destination;
    solClient_opaqueContainer_pt stream_p;
    solClient_opaqueContainer_pt replyStream_p;
    RR_operation_t     operation;
    solClient_uint32_t operand1 = 9;
    solClient_uint32_t operand2 = 5;
    solClient_bool_t resultOk;
    double result;


    /* Allocate a message for requests. */
    if ( ( rc = solClient_msg_alloc ( &msg_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_alloc()" );
        return;
    }
    /* Note: A bad operation is purposely sent in this example (lastOperation + 1). */
    for ( operation = firstOperation; operation <= lastOperation; operation++ ) {
        if ( operation <= lastOperation ) {
            printf ( "Sending request for %d %s %d\n", operand1, RR_operationToString ( operation ), operand2 );
        } else {
           printf ( "Sending request for a bad operation '%d %s %d', expect an APP error\n",
                    operand1, RR_operationToString ( operation ), operand2 );
        }
        /* Set the destination Topic for the request message. */
        destination.destType = SOLCLIENT_TOPIC_DESTINATION;
        destination.dest = destinationName;
        if ( ( rc = solClient_msg_setDestination ( msg_p, &destination, sizeof ( destination ) ) ) != SOLCLIENT_OK ) {
            common_handleError ( rc, "solClient_msg_setDestination()" );
            goto freeMsg;
        }
        /* Create a stream in the binary attachment part of the message. */
        if ( ( rc = solClient_msg_createBinaryAttachmentStream ( msg_p, &stream_p, 100 ) ) != SOLCLIENT_OK ) {
            common_handleError ( rc, "solClient_msg_createBinaryAttachmentStream()" );
            goto freeMsg;
        }
        /* Put the operation, operand1, operand2 into the stream. */
        if ( ( rc = solClient_container_addInt8 ( stream_p, ( solClient_int8_t ) operation, NULL ) ) != SOLCLIENT_OK ) {
            common_handleError ( rc, "solClient_container_addInt8()" );
            goto freeMsg;
        }
        if ( ( rc = solClient_container_addInt32 ( stream_p, operand1, NULL ) ) != SOLCLIENT_OK ) {
            common_handleError ( rc, "solClient_container_addInt32()" );
            goto freeMsg;
        }
        if ( ( rc = solClient_container_addInt32 ( stream_p, operand2, NULL ) ) != SOLCLIENT_OK ) {
            common_handleError ( rc, "solClient_container_addInt32()" );
            goto freeMsg;
        }
        /* Send a blocking request. */
        if ( ( rc = solClient_session_sendRequest ( opaqueSession_p,
                                                    msg_p, &replyMsg_p, 5000 /* timeout in milliseconds */  ) ) == SOLCLIENT_OK ) {

            /* Get the result status and result (if OK) from the reply message. */
            if ( ( rc = solClient_msg_getBinaryAttachmentStream ( replyMsg_p, &replyStream_p ) ) != SOLCLIENT_OK ) {
                common_handleError ( rc, "solClient_msg_getBinaryAttachmentStream()" );
                goto freeReplyMsg;
            }
            if ( ( rc = solClient_container_getBoolean ( replyStream_p, &resultOk, NULL ) ) != SOLCLIENT_OK ) {
                common_handleError ( rc, "solClient_container_getBool()" );
                goto freeReplyMsg;
            }
            if ( resultOk ) {
                if ( ( rc = solClient_container_getDouble ( replyStream_p, (double *)&result, NULL ) ) != SOLCLIENT_OK ) {
                    common_handleError ( rc, "solClient_container_getDouble()" );
                    goto freeReplyMsg;
                }
                printf ( "Received reply message, result = %f\n", result );
            } else {
                solClient_log ( SOLCLIENT_LOG_ERROR, "Received reply message with failed status." );
            }
          freeReplyMsg:
            /* Done with the reply message, so free it */
            if ( ( rc = solClient_msg_free ( &replyMsg_p ) ) != SOLCLIENT_OK ) {
                common_handleError ( rc, "solClient_msg_free()" );
            }
        } else {
            common_handleError ( rc, "solClient_session_sendRequest()" );
        }
        /* Reset the request message for the next operation. */
        if ( ( rc = solClient_msg_reset ( msg_p ) ) != SOLCLIENT_OK ) {
            common_handleError ( rc, "solClient_msg_reset()" );
            goto freeMsg;
        }
    }

  freeMsg:
    /* Finally, free the request message. */
    if ( ( rc = solClient_msg_free ( &msg_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_free()" );
    }
}


/*
 * fn main() 
 * param appliance_ip The message backbone IP address.
 * param appliance_username The client username* 
 * param request topic
 * The entry point to the application.
 */
int
main ( int argc, char *argv[] )
{

    struct commonOptions commandOpts;
    solClient_returnCode_t rc = SOLCLIENT_OK;

    /***********Context-related variable definitions*********/
    solClient_opaqueContext_pt context_p;
    solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;

    /***********Session-related variable definitions*********/
    solClient_opaqueSession_pt session_p;
    solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;
    const char     *sessionProps[50];
    int             propIndex = 0;

    /************ Basic initialization *********************/
    printf ( "\nBasicRequestor.c (Copyright 2013-2019 Solace Corporation. All rights reserved.)\n" );

    /*************************************************************************
     * Parse command options
     *************************************************************************/
    common_initCommandOptions(&commandOpts, 
                               ( USER_PARAM_MASK |
                                DEST_PARAM_MASK ),    /* required parameters */
                               ( HOST_PARAM_MASK |
                                PASS_PARAM_MASK |
                                LOG_LEVEL_MASK |
                                USE_GSS_MASK |
                                ZIP_LEVEL_MASK));                       /* optional parameters */
    if ( common_parseCommandOptions ( argc, argv, &commandOpts, NULL ) == 0 ) {
        exit (1);
    }


    /*************************************************************************
     * Initialize the API and setup logging level
     *************************************************************************/
    /* solClient needs to be initialized before any other API calls are made. */
    if ( ( rc = solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_initialize()" );
        goto cleanup;
    }

    common_printCCSMPversion (  );

    /* 
     * Standard logging levels can be set independently for the API and the
     * application. In this case, the ALL category is used to set the log level for 
     * both at the same time.
     */
    solClient_log_setFilterLevel ( SOLCLIENT_LOG_CATEGORY_ALL, commandOpts.logLevel );

    /*************************************************************************
     * CREATE A CONTEXT
     *************************************************************************/
    solClient_log ( SOLCLIENT_LOG_INFO, "Creating solClient context" );

    /* 
     * When creating the Context, specify that the Context thread should be 
     * created automatically instead of having the application create its own
     * Context thread.
     */
    if ( ( rc = solClient_context_create ( SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                                           &context_p, &contextFuncInfo, sizeof ( contextFuncInfo ) ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_context_create()" );
        goto cleanup;
    }

    /*************************************************************************
     * Create and connect a Session
     *************************************************************************/
    solClient_log ( SOLCLIENT_LOG_INFO, "Creating solClient sessions." );

    sessionFuncInfo.rxMsgInfo.callback_p = common_messageReceivePrintMsgCallback;
    sessionFuncInfo.rxMsgInfo.user_p = NULL;
    sessionFuncInfo.eventInfo.callback_p = common_eventCallback;
    sessionFuncInfo.eventInfo.user_p = NULL;;


    propIndex = 0;
    if ( commandOpts.targetHost[0] != (char) 0 ) {
        sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
        sessionProps[propIndex++] = commandOpts.targetHost;
    }

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_COMPRESSION_LEVEL;
    sessionProps[propIndex++] = ( commandOpts.enableCompression ) ? "9" : "0";

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_CONNECT_RETRIES;
    sessionProps[propIndex++] = "3";

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_RECONNECT_RETRIES;
    sessionProps[propIndex++] = "3";

    /*
     * Note: Reapplying subscriptions allows Sessions to reconnect after failure and
     * have all their subscriptions automatically restored. For Sessions with many
     * subscriptions, this can increase the amount of time required for a successful
     * reconnect.
     */
    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_REAPPLY_SUBSCRIPTIONS;
    sessionProps[propIndex++] = SOLCLIENT_PROP_ENABLE_VAL;

    /*
     * Note: Including meta data fields such as sender timestamp, sender ID, and sequence 
     * number can reduce the maximum attainable throughput as significant extra encoding/
     * decodingis required. This is true whether the fields are autogenerated or manually
     * added.
     */

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_GENERATE_SEND_TIMESTAMPS;
    sessionProps[propIndex++] = SOLCLIENT_PROP_ENABLE_VAL;

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_GENERATE_SENDER_ID;
    sessionProps[propIndex++] = SOLCLIENT_PROP_ENABLE_VAL;

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_GENERATE_SEQUENCE_NUMBER;
    sessionProps[propIndex++] = SOLCLIENT_PROP_ENABLE_VAL;

    if ( commandOpts.vpn[0] ) {
        sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
        sessionProps[propIndex++] = commandOpts.vpn;
    }
    
    /*
     * The certificate validation property is ignored on non-SSL sessions.
     * For simple demo applications, disable it on SSL sesssions (host
     * string begins with tcps:) so a local trusted root and certificate
     * store is not required. See the  API usres guide for documentation
     * on how to setup a trusted root so the servers certificate returned
     * on the secure connection can be verified if this is desired.
     */
    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_SSL_VALIDATE_CERTIFICATE;
    sessionProps[propIndex++] = SOLCLIENT_PROP_DISABLE_VAL;

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
    sessionProps[propIndex++] = commandOpts.username;
    
    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_PASSWORD;
    sessionProps[propIndex++] = commandOpts.password;

    if ( commandOpts.useGSS ) {
        sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME;
        sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME_GSS_KRB;
    }

    sessionProps[propIndex] = NULL;

    /*
     * Create a session.
     */
    if ( ( rc = solClient_session_create ( (char **) sessionProps,
                                           context_p,
                                           &session_p, &sessionFuncInfo, sizeof ( sessionFuncInfo ) ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_create()" );
         goto cleanup;
    }

    /*
     * Connect the session.
     */
    if ( ( rc = solClient_session_connect ( session_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_connect()" );
         goto cleanup;
    }


    /* Send the requests and wait for the responses. */
    sendRequests ( session_p,  commandOpts.destinationName);

    /*************************************************************************
     * CLEANUP
     *************************************************************************/
    /* Disconnect the Session. */
    if ( ( rc = solClient_session_disconnect ( session_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_disconnect()" );
    }

  cleanup:
    /* Cleanup solClient. */
    if ( ( rc = solClient_cleanup (  ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_cleanup()" );
    }
    goto notInitialized;

  notInitialized:
    return 0;
}
