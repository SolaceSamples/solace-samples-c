/** @example ex/Intro/QueueSubscriber.c
 */

/*
 *  Copyright 2012-2019 Solace Corporation. All rights reserved.
 *
 *  http://www.solace.com
 *
 *  This source is distributed under the terms and conditions
 *  of any contract or contracts between Solace and you or
 *  your company. If there are no contracts in place use of
 *  this source is not authorized. No support is provided and
 *  no distribution, sharing with others or re-use of this
 *  source is authorized unless specifically stated in the
 *  contracts referred to above.
 *
 *  QueueSubscriber
 *
 *  This sample shows the basics of creating session, connecting a session,
 *  and receiving a guaranteed message from a queue. This is meant to be a very
 *  basic example for demonstration purposes.
 */

#include "os.h"
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"

/* Message Count */
static int msgCount = 0;


/*****************************************************************************
 * sessionMessageReceiveCallback
 *
 * The message receive callback is mandatory for session creation.
 *****************************************************************************/
static          solClient_rxMsgCallback_returnCode_t
sessionMessageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    return SOLCLIENT_CALLBACK_OK;
}

/*****************************************************************************
 * sessionEventCallback
 *
 * The event callback function is mandatory for session creation.
 *****************************************************************************/
void
sessionEventCallback ( solClient_opaqueSession_pt opaqueSession_p,
                solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
}

/*****************************************************************************
 * flowEventCallback
 *
 * The event callback function is mandatory for flow creation.
 *****************************************************************************/
static void
flowEventCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_flow_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
}

/*****************************************************************************
 * flowMessageReceiveCallback
 *
 * The message receive callback is mandatory for session creation.
 *****************************************************************************/
static          solClient_rxMsgCallback_returnCode_t
flowMessageReceiveCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_msgId_t msgId;

    /* Process the message. */
    printf ( "Received message:\n" );
    solClient_msg_dump ( msg_p, NULL, 0 );
    printf ( "\n" );
    msgCount++;
    
    /* Acknowledge the message after processing it. */
    if ( solClient_msg_getMsgId ( msg_p, &msgId )  == SOLCLIENT_OK ) {
        printf ( "Acknowledging message Id: %lld.\n", msgId );
        solClient_flow_sendAck ( opaqueFlow_p, msgId );
    }

    return SOLCLIENT_CALLBACK_OK;
}

/*****************************************************************************
 * main
 *
 * The entry point to the application.
 *****************************************************************************/
int
main ( int argc, char *argv[] )
{
    /* Context */
    solClient_opaqueContext_pt context_p;
    solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;

    /* Session */
    solClient_opaqueSession_pt session_p;
    solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

    /* Flow */
    solClient_opaqueFlow_pt flow_p;
    solClient_flow_createFuncInfo_t flowFuncInfo = SOLCLIENT_FLOW_CREATEFUNC_INITIALIZER;

    /* Session Properties */
    const char     *sessionProps[20] = {0, };
    int             propIndex;

    /* Flow Properties */
    const char     *flowProps[20] = {0, };

    /* Provision Properties */
    const char     *provProps[20] = {0, };
    int             provIndex;

    if ( argc < 6 ) {
        printf ( "Usage: QueueSubscriber <msg_backbone_ip:port> <vpn> <client-username> <password> <queue>\n" );
        return -1;
    }

    /*************************************************************************
     * Initialize the API and setup logging level
     *************************************************************************/

    /* solClient needs to be initialized before any other API calls. */
    solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL );

    /*************************************************************************
     * CREATE A CONTEXT
     *************************************************************************/

    /* 
     * Create a Context, and specify that the Context thread be created
     * automatically instead of having the application create its own
     * Context thread.
     */
    solClient_context_create ( SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                                           &context_p, &contextFuncInfo, sizeof ( contextFuncInfo ) );

    /*************************************************************************
     * Create and connect a Session
     *************************************************************************/

    /* Configure the Session function information. */
    sessionFuncInfo.rxMsgInfo.callback_p = sessionMessageReceiveCallback;
    sessionFuncInfo.eventInfo.callback_p = sessionEventCallback;

    /* Configure the Session properties. */
    propIndex = 0;

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
    sessionProps[propIndex++] = argv[1];

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
    sessionProps[propIndex++] = argv[2];

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
    sessionProps[propIndex++] = argv[3];

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_PASSWORD;
    sessionProps[propIndex++] = argv[4];

    /* Create the Session. */
    solClient_session_create ( ( char ** ) sessionProps,
                               context_p,
                               &session_p, &sessionFuncInfo, sizeof ( sessionFuncInfo ) );

    /* Connect the Session. */
    solClient_session_connect ( session_p );
    printf ( "Connected.\n" );

    /*************************************************************************
     * Provision a Queue
     *************************************************************************/

    /* Configure the Provision properties */
    provIndex = 0;

    provProps[provIndex++] = SOLCLIENT_ENDPOINT_PROP_ID;
    provProps[provIndex++] = SOLCLIENT_ENDPOINT_PROP_QUEUE;

    provProps[provIndex++] = SOLCLIENT_ENDPOINT_PROP_NAME;
    provProps[provIndex++] = argv[5];

    provProps[provIndex++] = SOLCLIENT_ENDPOINT_PROP_PERMISSION;
    provProps[provIndex++] = SOLCLIENT_ENDPOINT_PERM_DELETE;

    provProps[provIndex++] = SOLCLIENT_ENDPOINT_PROP_QUOTA_MB;
    provProps[provIndex++] = "100";

    /* Check if the endpoint provisioning is support */
    if ( !solClient_session_isCapable ( session_p, SOLCLIENT_SESSION_CAPABILITY_ENDPOINT_MANAGEMENT ) ) {

        printf ( "Endpoint management not supported on this appliance.\n" );
        return -1;
    }

    /* Try to provision the Queue. Ignore if already exists */
    solClient_session_endpointProvision ( ( char ** ) provProps,
                                          session_p,
                                          SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM|
                                          SOLCLIENT_PROVISION_FLAGS_IGNORE_EXIST_ERRORS,
                                          NULL, NULL, 0 );

    /*************************************************************************
     * Create a Flow
     *************************************************************************/

    /* Configure the Flow function information */
    flowFuncInfo.rxMsgInfo.callback_p = flowMessageReceiveCallback;
    flowFuncInfo.eventInfo.callback_p = flowEventCallback;

    /* Configure the Flow properties */
    propIndex = 0;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_BLOCKING;
    flowProps[propIndex++] = SOLCLIENT_PROP_DISABLE_VAL;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_ENTITY_ID;
    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_ENTITY_QUEUE;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_ACKMODE;
    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_NAME;
    flowProps[propIndex++] = argv[5];

    solClient_session_createFlow ( ( char ** ) flowProps,
                                   session_p,
                                   &flow_p, &flowFuncInfo, sizeof ( flowFuncInfo ) );

    /*************************************************************************
     * Wait for messages
     *************************************************************************/

    printf ( "Waiting for messages......\n" );
    fflush ( stdout );
    while ( msgCount < 1 ) {
        SLEEP ( 1 );
    }

    printf ( "Exiting.\n" );

    /*************************************************************************
     * Cleanup
     *************************************************************************/

    /* Destroy the Flow */
    solClient_flow_destroy ( &flow_p );

    /* Disconnect the Session */
    solClient_session_disconnect ( session_p );

    /* Cleanup solClient. */
    solClient_cleanup (  );

    return 0;
}
