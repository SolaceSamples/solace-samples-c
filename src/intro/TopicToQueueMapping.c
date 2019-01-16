
/** @example ex/topicToQueueMapping.c 
 */

/*
 * This sample demonstrates:
 * - adding a Topic to a Queue provisioned on a appliance using the solClient_session_endpointTopicSubscribe() function.
 * - adding a Topic to a Queue provisioned on a appliance using the solClient_flow_topicSubscribeWithDispatch() function.
 *
 * Sample Requirements:
 *  - The appliance connection must support adding Topics to Queues.
 *
 *
 * In this sample, we create 1 session to a SolOS-TR appliance:
 * - create a durable Queue endpoint
 * - add Topic my/sample/topic/1 using session function
 * - add Topic my/sample/topic/2 using flow function
 * - publish message to my/sample/topic/1 and verify receipt
 * - publish message to my/sample/topic/2 and verify receipt
 *
 * Copyright 2010-2019 Solace Corporation. All rights reserved.
 */


/*****************************************************************************
 *  For Windows builds, os.h should always be included first to ensure that
 *  _WIN32_WINNT is defined before winsock2.h or windows.h get included.
 *****************************************************************************/
#include "os.h"
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"
#include "common.h"


/*****************************************************************************
 * main
 * 
 * The entry point to the application.
 *****************************************************************************/
int
main ( int argc, char *argv[] )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;

    /* Command Options */
    struct commonOptions commandOpts;

    /* Context */
    solClient_opaqueContext_pt context_p;
    solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;

    /* Session */
    solClient_opaqueSession_pt session_p;
    /* 
     * User pointers can be used when creating a session to allow applications
     * to have access to additional information from within its callback
     * functions.  The user pointer can refer to anything (void*), but in this
     * sample, we are using a simple char* to give the session a name that
     * will be used by the message receive callback to indicate which
     * actually received the message.
     *
     * Note: It is important that the data pointed to by the user pointer
     * remains valid for the duration of the context.  With the simplicity of
     * this sample, we are able to use the stack, but using the heap may be
     * required for more complex applications.
     */
    char           *user_p = "Session Callback";

    /* Flow   */
    solClient_opaqueFlow_pt flow_p = NULL;
    solClient_flow_createFuncInfo_t flowFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;
    /* Props - Properties used to create various objects */
    const char     *props[40] = {0, };
    int             propIndex;

    /* 
     * Use two different Topics for subscribing and publishing
     */
    char           *topic_p[2] = { COMMON_MY_SAMPLE_TOPIC "/1",
        COMMON_MY_SAMPLE_TOPIC "/2"
    };

    printf ( "\ntopicToQueueMapping.c (Copyright 2010-2019 Solace Corporation. All rights reserved.)\n" );

    /*************************************************************************
     * Parse command options
     *************************************************************************/
    common_initCommandOptions(&commandOpts, 
                               ( USER_PARAM_MASK ),    /* required parameters */
                               ( HOST_PARAM_MASK |
                                PASS_PARAM_MASK |
                                LOG_LEVEL_MASK |
                                USE_GSS_MASK |
                                ZIP_LEVEL_MASK));                       /* optional parameters */
    if ( common_parseCommandOptions ( argc, argv, &commandOpts, NULL ) == 0 ) {
        exit(1);
    }

    /*************************************************************************
     * Initialize the API and setup logging level
     *************************************************************************/

    /* solClient needs to be initialized before any other API calls are made. */
    if ( ( rc = solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_initialize()" );
        goto notInitialized;
    }

    common_printCCSMPversion (  );

    /* 
     * Standard logging levels can be set independently for the API and the
     * application. In this case, the ALL category is used to set the log level for 
     * both at the same time.
     */
    solClient_log_setFilterLevel ( SOLCLIENT_LOG_CATEGORY_ALL, commandOpts.logLevel );

    /*************************************************************************
     * Create a Context
     *************************************************************************/

    solClient_log ( SOLCLIENT_LOG_INFO, "Creating solClient context" );

    /* 
     * When creating the Context, specify that the Context thread be 
     * created automatically instead of having the application create its own
     * Context thread.
     */
    if ( ( rc = solClient_context_create ( SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                                           &context_p, &contextFuncInfo, sizeof ( contextFuncInfo ) ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_context_create()" );
        goto cleanup;
    }

    /*************************************************************************
     * Create a Session
     *************************************************************************/

    solClient_log ( SOLCLIENT_LOG_INFO, "Creating solClient sessions." );

    /* 
     * createAndConnectSession is a common function used in these samples.
     * It is a wrapper for solClient_session_create() that applies some 
     * common properties to the Session, some of which are based on the 
     * command options. The wrapper also connects the Session.
     */
    if ( ( rc = common_createAndConnectSession ( context_p,
                                                 &session_p,
                                                 common_messageReceiveCallback,
                                                 common_eventCallback, user_p, &commandOpts ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "common_createAndConnectSession()" );
        goto cleanup;
    }

    /*************************************************************************
     * Ensure Topic to Queue mapping is supported by this client connection
     *************************************************************************/

    /* The same appliance is used for all Sessions in this sample, so just check one. */
    if ( !solClient_session_isCapable ( session_p, SOLCLIENT_SESSION_CAPABILITY_QUEUE_SUBSCRIPTIONS ) ) {

        solClient_log ( SOLCLIENT_LOG_ERROR, "Topic To Queue Mapping is not supported on this client connection." );
        goto sessionConnected;
    }

    /************************************************************************
     * Provision a Queue on the appliance 
     ***********************************************************************/
    solClient_log ( SOLCLIENT_LOG_INFO, "Creating queue %s on appliance.", COMMON_TESTQ );
    if ( ( rc = common_createQueue ( session_p, COMMON_TESTQ ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "common_createQueue()" );
        goto sessionConnected;
    }

    /*************************************************************************
     * Subscribe through the Session
     *************************************************************************/

    solClient_log ( SOLCLIENT_LOG_INFO, "Adding subscription %s to queue %s via session.", topic_p[0], COMMON_TESTQ );
    propIndex = 0;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_ID;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_QUEUE;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_NAME;
    props[propIndex++] = COMMON_TESTQ;
    props[propIndex++] = NULL;
    if ( ( rc = solClient_session_endpointTopicSubscribe ( (char**)props,
                                                           session_p,
                                                           SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                                           topic_p[0], 0 ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_endpointTopicSubscribe()" );
        goto sessionConnected;
    }

    /*************************************************************************
     * Create a Flow to the Queue
     *************************************************************************/

    solClient_log ( SOLCLIENT_LOG_INFO, "Bind to queue %s.", COMMON_TESTQ );
    flowFuncInfo.rxMsgInfo.callback_p = common_flowMessageReceiveCallback;
    flowFuncInfo.rxMsgInfo.user_p = NULL;       /* NULL or counter pointer */
    flowFuncInfo.eventInfo.callback_p = common_flowEventCallback;
    flowFuncInfo.eventInfo.user_p = NULL;       /* unused in common_flowEventCallback */

    propIndex = 0;

    props[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_BLOCKING;
    props[propIndex++] = SOLCLIENT_PROP_ENABLE_VAL;

    props[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_ENTITY_ID;
    props[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_ENTITY_QUEUE;
    props[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_NAME;
    props[propIndex++] = COMMON_TESTQ;

    if ( ( rc = solClient_session_createFlow ( (char **)props,
                                               session_p, &flow_p, &flowFuncInfo, sizeof ( flowFuncInfo ) ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_createFlow() did not return SOLCLIENT_OK" );
        goto sessionConnected;
    }

    /*************************************************************************
     * Subscribe to the second Topic through the Flow. There is no dispatch
     * function here -- just add a subscription to Queue.
     *************************************************************************/

    solClient_log ( SOLCLIENT_LOG_INFO, "Adding subscription %s to queue %s via flow.", topic_p[1], COMMON_TESTQ );

    if ( ( rc = solClient_flow_topicSubscribeWithDispatch ( flow_p,
                                                            SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                                            topic_p[1],
                                                            NULL,       /* no dispatch functions */
                                                            0           /* correlation tag pointer */
                                                                ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_flow_topicSubscribeWithDispatch()" );
        goto sessionConnected;
    }

    /*************************************************************************
     * Publish a message on each Topic
     *************************************************************************/

    printf ( "Publishing two messages, expect two messages received on flow\n\n" );
    common_publishMessage ( session_p, topic_p[0], SOLCLIENT_DELIVERY_MODE_PERSISTENT );

    common_publishMessage ( session_p, topic_p[1], SOLCLIENT_DELIVERY_MODE_PERSISTENT );


    /* pause to let callback receive messages */
    SLEEP ( 1 );


    /*************************************************************************
     * Cleanup
     *************************************************************************/
  sessionConnected:
    if ( flow_p != NULL ) {
        /*
         * Destroy the Flow before deleting the Queue or else the API will log
         * at NOTICE level for the unsolicited unbind
         */
        if ( ( rc = solClient_flow_destroy ( &flow_p ) ) != SOLCLIENT_OK ) {
            common_handleError ( rc, "solClient_flow_destroy()" );
        }
    }


    /************************************************************************
     * Remove the Queue from the Appliance 
     ***********************************************************************/
    if ( ( rc = common_deleteQueue ( session_p, COMMON_TESTQ ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "common_deleteQueue()" );
    }

    /* Disconnect the Session. */
    if ( ( rc = solClient_session_disconnect ( session_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_disconnect()" );
    }

  cleanup:
    /* Cleanup solClient. */
    if ( ( rc = solClient_cleanup (  ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_cleanup()" );
    }

  notInitialized:
    return 0;

}
