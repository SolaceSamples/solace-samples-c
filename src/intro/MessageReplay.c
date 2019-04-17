/** @example MessageReplay.c
*/

/*
 *  Copyright 2019 Solace Corporation. All rights reserved.
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
 *  MessageReplay
 *
 *  This sample demonstrates the message replay feature in the Solace Message
 *  Router.
 *
 *******************************************************************************
 *  SETUP
 *
 *  Before running this sample, be sure that message-replay is enabled in the VPN
 *  used.
 *  Also messages must have been published to the replay-log for the queue that is
 *  used. The QueuePublisher sample can be used to create and publish messages to
 *  the queue. The QueueSubscriber sample can be used to drain the queue, so that
 *  replay is performed on an empty queue and observed by this sample.
 ********************************************************************************
 *
 ********************************************************************************
 *  OPERATION
 *
 *  First, a client initiated replay is started when the flow connects. All
 *  messages are requested from the replay log. Comments also explain how to
 *  request the replay log from a specific start time.
 *
 *  Second, the application waits for a CLI/Message-Router initiated replay.
 *  The flow event handler monitors for a replay start event. When the Message-Router
 *  initiates a replay, the flow will see a DOWN_ERROR event with cause
 *  'Replay Started'. This means an operator has initiated a replay, and the
 *  application must destroy and re-create the flow to receive the replayed messages.
 */

#include "os.h"
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"

/* Message Count */
static int msgCount = 0;


/*****************************************************************************
 * sessionMessageReceiveCallback
 *
 * The message receive callback is mandatory for session creation. This
 * callback silently discards all received messages.
 *****************************************************************************/
    static          solClient_rxMsgCallback_returnCode_t
sessionMessageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    return SOLCLIENT_CALLBACK_OK;
}

/*****************************************************************************
 * sessionEventCallback
 *
 * The event callback function is mandatory for session creation. This callback
 * ignores all session events.
 *****************************************************************************/
    void
sessionEventCallback ( solClient_opaqueSession_pt opaqueSession_p,
        solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
}

/*****************************************************************************
 * flowEventCallback
 *
 * The event callback function is mandatory for flow creation.  Monitor for
 * SOLCLIENT_FLOW_EVENT_DOWN_ERROR. When seen, save the error information for
 * processing by the main thread.
 *****************************************************************************/
    static void
flowEventCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_flow_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
    /*
     * The flow can not be destroyed and re-created from this callback,
     *  so the errorInfo is only saved instead, and processed on the main loop.
     */
    solClient_errorInfo_pt errorInfo_p = solClient_getLastErrorInfo();
    printf ( "flowEventCallbackFunc() called - %s; subCode: %s, responseCode: %d, reason: \"%s\"\n",
            solClient_flow_eventToString ( eventInfo_p->flowEvent ),
            solClient_subCodeToString ( errorInfo_p->subCode ), errorInfo_p->responseCode, errorInfo_p->errorStr );

    if ( SOLCLIENT_FLOW_EVENT_DOWN_ERROR == eventInfo_p->flowEvent ) {
        solClient_errorInfo_pt flowErrorInfo_p = (solClient_errorInfo_pt) user_p;
        flowErrorInfo_p->responseCode = errorInfo_p->responseCode;
        flowErrorInfo_p->subCode = errorInfo_p->subCode;
        strncpy(errorInfo_p->errorStr, flowErrorInfo_p->errorStr, sizeof(flowErrorInfo_p->errorStr));
        switch (errorInfo_p->subCode) {
            case SOLCLIENT_SUBCODE_REPLAY_STARTED:
            case SOLCLIENT_SUBCODE_REPLAY_FAILED:
            case SOLCLIENT_SUBCODE_REPLAY_CANCELLED:
            case SOLCLIENT_SUBCODE_REPLAY_LOG_MODIFIED:
            case SOLCLIENT_SUBCODE_REPLAY_START_TIME_NOT_AVAILABLE:
            case SOLCLIENT_SUBCODE_REPLAY_MESSAGE_UNAVAILABLE:
            case SOLCLIENT_SUBCODE_REPLAY_MESSAGE_REJECTED:
                break;
            default:
                break;
        }

    }
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
    const char     *sessionProps[20] = {NULL};
    int             propIndex;
    int             replayStartLocationIndex;

    /* Flow Properties */
    const char     *flowProps[20] = {NULL};

    /* Provision Properties */
    const char     *provProps[20] = {NULL};
    int             provIndex;

    if ( argc < 6 ) {
        printf ( "Usage: MessageReplay <msg_backbone_ip:port> <vpn> <client-username> <password> <queue>\n" );
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

    /* Check whether endpoint provisioning is supported */
    if ( !solClient_session_isCapable ( session_p, SOLCLIENT_SESSION_CAPABILITY_ENDPOINT_MANAGEMENT ) ) {

        printf ( "Endpoint management not supported on this message broker.\n" );
        return -1;
    }

    /* Try to provision the Queue. Ignore if already exists */
    solClient_session_endpointProvision ( ( char ** ) provProps,
            session_p,
            SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM|
            SOLCLIENT_PROVISION_FLAGS_IGNORE_EXIST_ERRORS,
            NULL, NULL, 0 );

    /* Check whether replay is supported */
    if ( !solClient_session_isCapable ( session_p, SOLCLIENT_SESSION_CAPABILITY_MESSAGE_REPLAY ) ) {

        printf ( "Message replay not supported on this message broker.\n" );
        return -1;
    }

    /*************************************************************************
     * Create a Flow
     * On this flow request a replay of all messages that have ever been published
     * to this queue name.
     *************************************************************************/

    /* Configure the Flow function information */
    flowFuncInfo.rxMsgInfo.callback_p = flowMessageReceiveCallback;
    flowFuncInfo.eventInfo.callback_p = flowEventCallback;

    /* Configure the Flow properties */
    propIndex = 0;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_BLOCKING;
    flowProps[propIndex++] = SOLCLIENT_PROP_ENABLE_VAL;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_ENTITY_ID;
    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_ENTITY_QUEUE;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_ACKMODE;
    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_NAME;
    flowProps[propIndex++] = argv[5];

    /*
     * Remember the replayStartLocation index for later, as we
     * need to modify or remove this property.
     */
    replayStartLocationIndex = propIndex;

    flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION;
    flowProps[propIndex] = SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION_BEGINNING;

    /***************************************************************
     * Alternative replay start specifications to try instead of
     * SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION_BEGINNING.
     */
     /* Seconds since UNIX epoch: */
     // flowProps[propIndex] = "DATE:1554331492";

     /* RFC3339 UTC date with timezone offset 0: */
     // flowProps[propIndex] = "DATE:2019-04-03T18:48:00Z";

     /* RFC3339 date with timezone: */
     // flowProps[propIndex] = "DATE:2019-04-03T18:48:00Z-05:00";

     /**************************************************************/

    propIndex++;

    /**************************************************************
     * NULL terminating the flow properties array
     **************************************************************/
    flowProps[propIndex++] = NULL;
    flowProps[propIndex] = NULL;

    solClient_errorInfo_t flowErrorInfo = {SOLCLIENT_SUBCODE_OK, 0, ""};
    flowFuncInfo.eventInfo.user_p = &flowErrorInfo;



    int rc = solClient_session_createFlow ( ( char ** ) flowProps,
            session_p,
            &flow_p, &flowFuncInfo, sizeof ( flowFuncInfo ) );

    if (rc != SOLCLIENT_OK) {
        printf ( "Flow bind failed with rc %d.\n", rc );
        solClient_errorInfo_pt errorInfo_p = solClient_getLastErrorInfo();
        printf ( "ErrorInfo subCode: %s, responseCode: %d, reason: \"%s\"\n", solClient_subCodeToString ( errorInfo_p->subCode ), errorInfo_p->responseCode, errorInfo_p->errorStr );
        return -1;
    }


    /*************************************************************************
     * Wait for messages
     *************************************************************************/

    printf ( "Waiting for 10 messages......\n" );
    fflush ( stdout );
    while ( msgCount < 10 ) {
        if (flowErrorInfo.subCode  != SOLCLIENT_SUBCODE_OK) {
            if (flowErrorInfo.subCode == SOLCLIENT_SUBCODE_REPLAY_STARTED) {
                printf ( "Router initiating replay, reconnecting flow to recieve messages.\n" );
                flowErrorInfo.responseCode = 0;
                flowErrorInfo.subCode = 0;
                flowErrorInfo.errorStr[0] = '\0';

                /*
                 * Destroying and re-creating the flow
                 * so messages can show up as many times as the replay restarts,
                 * without the Assured Delivery subsystem filtering them out as duplicates
                 * the second and consecutive times.
                 */

                solClient_flow_destroy(&flow_p);
                /*
                 * Remove the REPLAY_START_LOCATION from the flow properties array
                 * as it would override the operator initiated replay request.
                 * NOTE: the REPLAY_START_LOCATION must be the last property
                 * for this to work.
                 */
                propIndex = replayStartLocationIndex;
                /**************************************************************
                 * NULL terminating the flow properties array
                 **************************************************************/
                flowProps[propIndex++] = NULL;
                flowProps[propIndex] = NULL;

                rc = solClient_session_createFlow ( ( char ** ) flowProps,
                        session_p,
                        &flow_p, &flowFuncInfo, sizeof ( flowFuncInfo ) );
                if (rc != SOLCLIENT_OK) {
                    printf ( "Flow bind failed with rc %d.\n", rc );
                    solClient_errorInfo_pt errorInfo_p = solClient_getLastErrorInfo();
                    printf ( "ErrorInfo subCode: %s, responseCode: %d, reason: \"%s\"\n", solClient_subCodeToString ( errorInfo_p->subCode ), errorInfo_p->responseCode, errorInfo_p->errorStr );
                    return -1;
                }
            } else if (flowErrorInfo.subCode == SOLCLIENT_SUBCODE_REPLAY_START_TIME_NOT_AVAILABLE) {
                /*
                 * This can only happen when the replay is requested from a specific start time,
                 * which is older than the replay log on the router.
                 */
                printf ( "Replay log does not cover requested time period, reconnecting flow for full log instead.\n" );
                flowErrorInfo.responseCode = 0;
                flowErrorInfo.subCode = 0;
                flowErrorInfo.errorStr[0] = '\0';

                flowProps[replayStartLocationIndex + 1] = SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION_BEGINNING;
                solClient_flow_destroy(&flow_p);
                rc = solClient_session_createFlow ( ( char ** ) flowProps,
                        session_p,
                        &flow_p, &flowFuncInfo, sizeof ( flowFuncInfo ) );
                if (rc != SOLCLIENT_OK) {
                    printf ( "Flow bind failed with rc %d.\n", rc );
                    solClient_errorInfo_pt errorInfo_p = solClient_getLastErrorInfo();
                    printf ( "ErrorInfo subCode: %s, responseCode: %d, reason: \"%s\"\n", solClient_subCodeToString ( errorInfo_p->subCode ), errorInfo_p->responseCode, errorInfo_p->errorStr );
                    return -1;
                }
            }
        }

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
