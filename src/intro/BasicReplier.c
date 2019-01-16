/** @example Intro/BasicReplier.c
 */

/*
 * This sample shows how to implement a Requester for direct Request-Reply messaging, where 
 *
 *    BasicRequester: A message Endpoint that sends a request message and waits to receive a
 *                       reply message as a response.
 *    BasicReplier:   A message Endpoint that waits to receive a request message and responses
 *                       to it by sending a reply message.
 *  
 *  |----------------|  ---RequestTopic --> |---------------|
 *  | BasicRequester |                      | BasicReplier  |
 *  |----------------|  <--ReplyToTopic---- |---------------|
 *
 * Copyright 2013-2019 Solace Corporation. All rights reserved.
 *
 */
#include "os.h"
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"
#include "common.h"
#include "RRcommon.h"

int msgReplied = 0;

/*****************************************************************************
 * Received message handling code
 *****************************************************************************/
static          solClient_rxMsgCallback_returnCode_t
requestMsgReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;
    solClient_opaqueMsg_pt replyMsg_p;
    solClient_opaqueContainer_pt stream_p;
    solClient_opaqueContainer_pt replyStream_p;
    solClient_bool_t resultOk = 1;
    solClient_int8_t operation = -1;
    solClient_int32_t operand1 = -1;
    solClient_int32_t operand2 = -1;
    double result;

    /*
     * Get the operator, operand1 and operand2 from the stream in the binary
     * attachment.
     */
    if ( ( rc = solClient_msg_getBinaryAttachmentStream ( msg_p, &stream_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_getBinaryAttachmentStream()" );
        goto createReply;
    }
    /* Get the operation, operand1 and operand2 from the stream. */
    if ( ( rc = solClient_container_getInt8 ( stream_p, &operation, NULL ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_container_getInt8() for operation" );
        goto createReply;
    }
    if ( ( rc = solClient_container_getInt32 ( stream_p, &operand1, NULL ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_container_getInt32() for operand1" );
        goto createReply;
    }
    if ( ( rc = solClient_container_getInt32 ( stream_p, &operand2, NULL ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_container_getInt32() for operand2" );
        goto createReply;
    }
    /* Do the requested calculation. */
    switch ( operation ) {
        case plusOperation:
            result = (double)operand1 + (double)operand2;
            resultOk = 1;
            break;
        case minusOperation:
            result = (double)operand1 - (double)operand2;
            resultOk = 1;
            break;
        case timesOperation:
            result = (double)operand1 * (double)operand2;
            resultOk = 1;
            break;
        case divideOperation:
            if (operand2 != 0) {
                result = (double)operand1 / (double)operand2;
                resultOk = 1;
            }
            break;
        default:
            break;
    }

  createReply:
    if ( resultOk ) {
        printf( "  Received request for %d %s %d, sending reply with result %f. \n",
                operand1, RR_operationToString ( operation ), operand2, result );
    } else {
        printf( "  Received request for %d %s %d, sending reply with a failure status.\n",
                operand1, RR_operationToString ( operation ), operand2  );
    }
    /*
     * Allocate a message to construct the reply, and put in the status and result in a
     * stream.
     */
    if ( ( rc = solClient_msg_alloc ( &replyMsg_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_alloc()" );
        return SOLCLIENT_CALLBACK_OK;
    }
    if ( ( rc = solClient_msg_createBinaryAttachmentStream ( replyMsg_p, &replyStream_p, 32 ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_createBinaryAttachmentStream()" );
        goto freeMsg;
    }
    if ( ( rc = solClient_container_addBoolean ( replyStream_p, resultOk, NULL ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_container_addBoolean()" );
        goto freeMsg;
    }
    if ( resultOk ) {
        if ( ( rc = solClient_container_addDouble ( replyStream_p, result, NULL ) ) != SOLCLIENT_OK ) {
            common_handleError ( rc, "solClient_container_addDouble()" );
            goto freeMsg;
        }
    }
    if ( ( rc = solClient_session_sendReply ( opaqueSession_p, msg_p, replyMsg_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_sendReply()" );
        goto freeMsg;
    }

  freeMsg:
    if ( ( rc = solClient_msg_free ( &replyMsg_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_free()" );
    }
    msgReplied ++;
    return SOLCLIENT_CALLBACK_OK;
}


/*
 * fn main() 
 * param appliance_ip The message backbone IP address.
 * param appliance_username The client username.
 * param request topic
 *
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
    const char     *sessionProps[50] = {0, };
    int             propIndex = 0;

    printf ( "\nBasicReplier.c (Copyright 2013-2019 Solace Corporation. All rights reserved.)\n" );

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

    sessionFuncInfo.rxMsgInfo.callback_p = requestMsgReceiveCallback;
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

    /*
     * Create a session.
     */
    if ( ( rc = solClient_session_create ( (char**) sessionProps,
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

    /*************************************************************************
     * Subscribe to the request topic
     *************************************************************************/
    if ( ( rc = solClient_session_topicSubscribeExt ( session_p,
                                                      SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                                      commandOpts.destinationName) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_topicSubscribe()" );
        goto sessionConnected;
    }

    /*************************************************************************
     * Serve requests, CTRL-C to stop
     *************************************************************************/
    printf ( "Serving requests on topic '%s', Ctrl-C to stop.....\n", commandOpts.destinationName );
    while ( msgReplied < 1) {
        SLEEP(1);
    }

    /*************************************************************************
     * CLEANUP
     *************************************************************************/
  sessionConnected:
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
