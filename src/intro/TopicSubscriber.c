/** @example ex/Intro/TopicSubscriber.c
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
 *  TopicSubscriber
 *
 *  This sample shows the basics of creating session, connecting a session,
 *  and receiving a direct message from a topic. This is meant to be a very
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
 * The message callback is invoked for each Direct message received by
 * the Session. In this sample, the message is printed to the screen.
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
sessionMessageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    printf ( "Received message:\n" );
    solClient_msg_dump ( msg_p, NULL, 0 );
    printf ( "\n" );

    msgCount++;

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

    /* Session Properties */
    const char     *sessionProps[20] = {0, };
    int             propIndex = 0;

    if ( argc < 6 ) {
        printf ( "Usage: TopicSubscriber <msg_backbone_ip:port> <vpn> <client-username> <password> <topic>\n" );
        return -1;
    }

    /*************************************************************************
     * Initialize the API (and setup logging level)
     *************************************************************************/

    /* solClient needs to be initialized before any other API calls. */
    solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL );
    printf ( "TopicSubscriber initializing...\n" );

    /*************************************************************************
     * Create a Context
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
    sessionFuncInfo.rxMsgInfo.user_p = NULL;
    sessionFuncInfo.eventInfo.callback_p = sessionEventCallback;
    sessionFuncInfo.eventInfo.user_p = NULL;

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
     * Subscribe
     *************************************************************************/

    solClient_session_topicSubscribeExt ( session_p,
                                          SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                          argv[5] );

    /*************************************************************************
     * Wait for message
     *************************************************************************/

    printf ( "Waiting for message......\n" );
    fflush ( stdout );
    while ( msgCount < 1 ) {
        SLEEP ( 1 );
    }

    printf ( "Exiting.\n" );

    /*************************************************************************
     * Unsubscribe
     *************************************************************************/

    solClient_session_topicUnsubscribeExt ( session_p,
                                            SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                            argv[4] );

    /*************************************************************************
     * Cleanup
     *************************************************************************/

    /* Cleanup solClient. */
    solClient_cleanup (  );

    return 0;
}
