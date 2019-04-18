
/** example ex/common.c 
 */

/** 
 * Example file for the Solace Messaging API for C.
 * 
 * Common functions and utilities used by sample code.
 *
 * Copyright 2007-2019 Solace Corporation. All rights reserved.
 *
 */

/**************************************************************************
    For Windows builds, os.h should always be included first to ensure that
    _WIN32_WINNT is defined before winsock2.h or windows.h get included.
 **************************************************************************/
#include "os.h"
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"
#include "solclient/solCache.h"
#include "common.h"
#include "RRcommon.h"
#include "getopt.h"

/*****************************************************************************
 * common_printCCSMPversion
 *****************************************************************************/
void
common_printCCSMPversion ( void )
{
    solClient_version_info_pt version_p;

    if ( solClient_version_get ( &version_p ) != SOLCLIENT_OK ) {
        printf ( "Unknown library version, solClient_version_get returns FAIL\n\n" );
    } else {
        printf ( "CCSMP Version %s (%s)\tVariant: %s\n\n", version_p->version_p, version_p->dateTime_p, version_p->variant_p );
    }
}


/*****************************************************************************
 * common_handleError
 *****************************************************************************/
void
common_handleError ( solClient_returnCode_t rc, const char *errorStr )
{
    solClient_errorInfo_pt errorInfo = solClient_getLastErrorInfo (  );
    solClient_log ( SOLCLIENT_LOG_ERROR,
                    "%s - ReturnCode=\"%s\", SubCode=\"%s\", ResponseCode=%d, Info=\"%s\"",
                    errorStr, solClient_returnCodeToString ( rc ),
                    solClient_subCodeToString ( errorInfo->subCode ), errorInfo->responseCode, errorInfo->errorStr );
    solClient_resetLastErrorInfo (  );
}


/*****************************************************************************
 * common_parseUsernameAndVpn
 *****************************************************************************/
void
common_parseUsernameAndVpn ( const char *inName, char *outUsername, size_t usernameLen, char *outVpn, size_t vpnLen )
{
    const char     *c = inName;
    char           *dest = outUsername;
    size_t          remaining = usernameLen;

    while ( *c ) {
        if ( *c == '@' ) {
            *dest = '\0';
            dest = outVpn;
            remaining = vpnLen;
        } else {
            if ( remaining ) {
                *dest++ = *c;
                remaining--;
            } else {
                *( dest - 1 ) = '\0';
            }
        }
        c++;
    }
    *dest = '\0';
}


/*****************************************************************************
 * common_initCommandOptions
 *****************************************************************************/
void
common_initCommandOptions ( struct commonOptions *commonOpt, 
                            int requiredParams,
                            int optionals)
{
    if (commonOpt != NULL) {
        commonOpt->username[0] = ( char ) 0;
        commonOpt->password[0] = ( char ) 0;
        commonOpt->vpn[0] = ( char ) 0;
        commonOpt->targetHost[0] = ( char ) 0;
        commonOpt->cacheName[0] = ( char ) 0;
        commonOpt->replayStartLocation[0] = ( char ) 0;
        commonOpt->usingTopic = 1;       /* TRUE pub/sub to/from Topic */
        commonOpt->usingAD = 0; /* FALSE pub/sub direct */
        commonOpt->destinationName[0] = ( char ) 0;
        commonOpt->numMsgsToSend = 1;
        commonOpt->msgRate = 1;
        commonOpt->gdWindow = 0;
        commonOpt->logLevel = SOLCLIENT_LOG_DEFAULT_FILTER;
        commonOpt->usingDurable = 0; //FALSE
        commonOpt->enableCompression = 0; //FALSE
        commonOpt->useGSS = 0; //FALSE
        commonOpt->requiredFields = requiredParams;
        commonOpt->optionalFields = optionals;
    }
}

/*****************************************************************************
 * common_parseCommandOptions
 *****************************************************************************/
int
common_parseCommandOptions ( int argc, char **argv, struct commonOptions *commonOpt, const char *positionalDesc )
{
    static char    *optstring = "a:c:dgl:m:n:p:r:s:t:u:w:zR:";
    static struct option longopts[] = {
        {"cache", 1, NULL, 'a'},
        {"cip", 1, NULL, 'c'},
        {"durable", 0, NULL, 'd'},
        {"gss", 0, NULL, 'g'},
        {"log", 1, NULL, 'l'},
        {"cu", 1, NULL, 'u'},
        {"mn", 1, NULL, 'n'},
        {"cp", 1, NULL, 'p'},
        {"mr", 1, NULL, 'r'},
        {"topic", 1, NULL, 't'},
        {"win", 1, NULL, 'w'},
        {"zip", 0, NULL, 'z'},
        {"replay", 1, NULL, 'R'},
        {0, 0, 0, 0}
    };
    int             c;
    int             rc = 1;
    char           *end_p;

    while ( ( c = getopt_long ( argc, argv, optstring, longopts, NULL ) ) != -1 ) {
        switch ( c ) {
            case 'a':
                strncpy ( commonOpt->cacheName, optarg, sizeof ( commonOpt->cacheName ) );
                break;
            case 'c':
                strncpy ( commonOpt->targetHost, optarg, sizeof ( commonOpt->targetHost ) );
                break;
            case 'd':
                commonOpt->usingDurable = 1; //TRUE
                break;
            case 'g':
                commonOpt->useGSS = 1; //TRUE
                break;
            case 'z':
                commonOpt->enableCompression = 1; //TRUE
                break;
            case 'R':
                strncpy ( commonOpt->replayStartLocation, optarg, sizeof ( commonOpt->replayStartLocation ) );
                break;
            case 'l':
                commonOpt->logLevel = ( solClient_log_level_t ) strtol ( optarg, &end_p, 0 );
                if ( ( commonOpt->logLevel > SOLCLIENT_LOG_DEBUG ) || ( *end_p != ( char ) 0 ) ) {
                    if ( strcasecmp ( optarg, "debug" ) == 0 ) {
                        commonOpt->logLevel = SOLCLIENT_LOG_DEBUG;
                    } else if ( strcasecmp ( optarg, "info" ) == 0 ) {
                        commonOpt->logLevel = SOLCLIENT_LOG_INFO;
                    } else if ( strcasecmp ( optarg, "notice" ) == 0 ) {
                        commonOpt->logLevel = SOLCLIENT_LOG_NOTICE;
                    } else if ( strcasecmp ( optarg, "warn" ) == 0 ) {
                        commonOpt->logLevel = SOLCLIENT_LOG_WARNING;
                    } else if ( strcasecmp ( optarg, "error" ) == 0 ) {
                        commonOpt->logLevel = SOLCLIENT_LOG_ERROR;
                    } else if ( strcasecmp ( optarg, "critical" ) == 0 ) {
                        commonOpt->logLevel = SOLCLIENT_LOG_CRITICAL;
                    } else {
                        rc = 0;
                    }
                }
                break;
            case 'n':
                commonOpt->numMsgsToSend = atoi ( optarg );
                if ( commonOpt->numMsgsToSend <= 0 )
                    rc = 0;
                break;
            case 'r':
                commonOpt->msgRate = atoi ( optarg );
                if ( commonOpt->msgRate <= 0 )
                    rc = 0;
                break;
            case 't':
                /* This is the name of the Topic or Queue to use, or the symbol number for content routing with simplePubSub. */
                strncpy ( commonOpt->destinationName, optarg, sizeof ( commonOpt->destinationName ) );
                break;
            case 'u':
                common_parseUsernameAndVpn ( optarg,
                                             commonOpt->username,
                                             sizeof ( commonOpt->username ), commonOpt->vpn, sizeof ( commonOpt->vpn ) );
                break;
            case 'p':
                strncpy ( commonOpt->password, optarg, sizeof ( commonOpt->password ) );
                break;
            case 'w':
                commonOpt->gdWindow = atoi ( optarg );
                if ( commonOpt->gdWindow <= 0 )
                    rc = 0;
                break;
            default:
                rc = 0;
                break;
        }
    }

    if ( ( commonOpt->requiredFields & HOST_PARAM_MASK ) && ( commonOpt->targetHost[0] == ( char ) 0 ) ) {
        printf ( "Missing required parameter '--cip'\n" );
        rc = 0;
    }
    if ( ( commonOpt->requiredFields & USER_PARAM_MASK ) && ( commonOpt->username[0] == ( char ) 0 ) && !commonOpt->useGSS) {
        printf ( "Missing required parameter '--cu'\n" );
        rc = 0;
    }
    if ( ( commonOpt->requiredFields & DEST_PARAM_MASK ) && ( commonOpt->destinationName[0] == ( char ) 0 ) ) {
        printf ( "Missing required parameter '--topic'\n" );
        rc = 0;
    }
    if ( ( commonOpt->requiredFields & PASS_PARAM_MASK ) && ( commonOpt->password[0] == ( char ) 0 ) ) {
        printf ( "Missing required parameter '--cp'\n" );
        rc = 0;
    }
    if ( ( commonOpt->requiredFields & CACHE_PARAM_MASK) && ( commonOpt->cacheName[0] == ( char ) 0) ) {
        printf ( "Missing required parameter '--cache'\n" );
        rc = 0;
    }
    if (rc == 0) {
        if (positionalDesc == NULL) {
            printf ("\nUsage: %s PARAMETERS [OPTIONS]\n\n",
                argv[0]
               );
        } else {
            printf ("\nUsage: %s PARAMETERS [OPTIONS] [ARGUMENTS]\n\n",
                argv[0]
               );
        }
        printf (
            "Where PARAMETERS are:\n%s%s%s%s%s"
            "Where OPTIONS are:\n%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
            ( commonOpt->requiredFields & HOST_PARAM_MASK ) ? HOST_PARAM_STRING : "",
            ( commonOpt->requiredFields & USER_PARAM_MASK ) ? USER_PARAM_STRING : "",
            ( commonOpt->requiredFields & DEST_PARAM_MASK ) ? DEST_PARAM_STRING : "",
            ( commonOpt->requiredFields & PASS_PARAM_MASK ) ? PASS_PARAM_STRING : "",
            ( commonOpt->requiredFields & CACHE_PARAM_MASK ) ? CACHE_PARAM_STRING : "",
            ( commonOpt->optionalFields & HOST_PARAM_MASK ) ? HOST_PARAM_STRING : "",
            ( commonOpt->optionalFields & USER_PARAM_MASK ) ? USER_PARAM_STRING : "",
            ( commonOpt->optionalFields & DEST_PARAM_MASK ) ? DEST_PARAM_STRING : "",
            ( commonOpt->optionalFields & PASS_PARAM_MASK ) ? PASS_PARAM_STRING : "",
            ( commonOpt->optionalFields & CACHE_PARAM_MASK ) ? CACHE_PARAM_STRING : "",
            ( commonOpt->optionalFields & DURABLE_MASK ) ? DURABLE_STRING : "",
            ( commonOpt->optionalFields & NUM_MSGS_MASK ) ? NUM_MSGS_STRING : "",
            ( commonOpt->optionalFields & MSG_RATE_MASK ) ? MSG_RATE_STRING : "",
            ( commonOpt->optionalFields & WINDOW_SIZE_MASK ) ? WINDOW_SIZE_STRING : "",
            ( commonOpt->optionalFields & LOG_LEVEL_MASK ) ? LOG_LEVEL_STRING : "",
            ( commonOpt->optionalFields & USE_GSS_MASK ) ? USE_GSS_STRING : "",
            ( commonOpt->optionalFields & ZIP_LEVEL_MASK ) ? ZIP_LEVEL_STRING : "",
            ( commonOpt->optionalFields & REPLAY_START_MASK ) ? REPLAY_START_STRING : ""
           );
        if (positionalDesc != NULL) {
            printf (
                "Where ARGUMENTS are:\n%s",
                positionalDesc
               );
        }
    }
    return ( rc );
}


/*****************************************************************************
 * common_createAndConnectSession
 *****************************************************************************/
solClient_returnCode_t
common_createAndConnectSession ( solClient_opaqueContext_pt context_p,
                                 solClient_opaqueSession_pt * session_p,
                                 solClient_session_rxMsgCallbackFunc_t msgCallback_p,
                                 solClient_session_eventCallbackFunc_t eventCallback_p,
                                 void *user_p, struct commonOptions * commonOpts )
{
    /* Return code */
    solClient_returnCode_t rc = SOLCLIENT_OK;

    /* Session Function Info */
    solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

    /* Session Properties */
    const char     *sessionProps[50] = {0, };
    int             propIndex = 0;


    /*************************************************************************
     * CONFIGURE THE SESSSION FUNCTION INFO
     *************************************************************************/

    sessionFuncInfo.rxMsgInfo.callback_p = msgCallback_p;
    sessionFuncInfo.rxMsgInfo.user_p = user_p;
    sessionFuncInfo.eventInfo.callback_p = eventCallback_p;
    sessionFuncInfo.eventInfo.user_p = user_p;

    /*************************************************************************
     * Configure the Session properties
     *************************************************************************/

    propIndex = 0;

    if ( commonOpts->targetHost[0] ) {
        sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
        sessionProps[propIndex++] = commonOpts->targetHost;
    }

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_COMPRESSION_LEVEL;
    sessionProps[propIndex++] = ( commonOpts->enableCompression ) ? "9" : "0";

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

    if ( commonOpts->vpn[0] ) {
        sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
        sessionProps[propIndex++] = commonOpts->vpn;
    }
    
    /*
     * The certificate validation property is ignored on non-SSL sessions.
     * For simple demo applications, disable it on SSL sesssions (host
     * string begins with tcps:) so a local trusted root and certificate
     * store is not required. See the  API user's guide for documentation
     * on how to setup a trusted root so the servers certificate returned
     * on the secure connection can be verified if this is desired.
     */
    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_SSL_VALIDATE_CERTIFICATE;
    sessionProps[propIndex++] = SOLCLIENT_PROP_DISABLE_VAL;

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
    sessionProps[propIndex++] = commonOpts->username;
    
    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_PASSWORD;
    sessionProps[propIndex++] = commonOpts->password;

    if ( commonOpts->useGSS ) {
        sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME;
        sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME_GSS_KRB;
    }

    /*************************************************************************
     * Create the Session
     *************************************************************************/
    if ( ( rc = solClient_session_create ( (char **) sessionProps,
                                           context_p,
                                           session_p, &sessionFuncInfo, sizeof ( sessionFuncInfo ) ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_create()" );
        return rc;
    }

    /*************************************************************************
     * Connect the Session
     *************************************************************************/
    if ( ( rc = solClient_session_connect ( *session_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_connect()" );
        return rc;
    }

    return SOLCLIENT_OK;
}

/*****************************************************************************
 * common_createQueue
 *****************************************************************************/

solClient_returnCode_t
common_createQueue ( solClient_opaqueSession_pt session_p, const char *queueName_p )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;
    const char     *props[40] = {0, };
    int             propIndex = 0;

    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_ID;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_QUEUE;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_NAME;
    props[propIndex++] = queueName_p;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_PERMISSION;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PERM_DELETE;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_QUOTA_MB;
    props[propIndex++] = "100";


    if ( ( rc = solClient_session_endpointProvision ( (char**) props, session_p, ( SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM | SOLCLIENT_PROVISION_FLAGS_IGNORE_EXIST_ERRORS ), NULL, /* correlationTag pointer */
                                                      NULL,
                                                      0 ) ) == SOLCLIENT_FAIL ) {
        common_handleError ( rc, "solClient_session_endpointProvision()" );
        return rc;
    }
    return SOLCLIENT_OK;
}

/*****************************************************************************
 * common_deleteQueue
 *****************************************************************************/

solClient_returnCode_t
common_deleteQueue ( solClient_opaqueSession_pt session_p, const char *queueName_p )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;
    const char     *props[40] = {0, };
    int             propIndex = 0;

    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_ID;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_QUEUE;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_NAME;
    props[propIndex++] = queueName_p;
    props[propIndex] = NULL;

    if ( ( rc = solClient_session_endpointDeprovision ( (char**)props, session_p, ( SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM | SOLCLIENT_PROVISION_FLAGS_IGNORE_EXIST_ERRORS ), NULL        /* correlationTag pointer */
            ) ) == SOLCLIENT_FAIL ) {
        common_handleError ( rc, "solClient_session_endpointDeprovision()" );
        return rc;
    }
    return SOLCLIENT_OK;
}

/*****************************************************************************
 * common_publishMessage
 *****************************************************************************/
solClient_returnCode_t
common_publishMessage ( solClient_opaqueSession_pt session_p, char *topic_p, solClient_uint32_t deliveryMode )
{
    /* Return code */
    solClient_returnCode_t rc = SOLCLIENT_OK;
    solClient_returnCode_t rcFreeMsg = SOLCLIENT_OK;
    solClient_opaqueMsg_pt msg_p = NULL;
    solClient_destination_t destination;
    const char *text_p = COMMON_ATTACHMENT_TEXT;


    solClient_log ( SOLCLIENT_LOG_DEBUG, "common_publishMessage() called.\n" );

    /* Allocate memory for the message to be sent. */
    if ( ( rc = solClient_msg_alloc ( &msg_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_alloc()" );
        return rc;
    }

    /* Set the message delivery mode. */
    if ( ( rc = solClient_msg_setDeliveryMode ( msg_p, deliveryMode ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_setDeliveryMode()" );
        goto freeMessage;
    }

    /* attach a payload */
    if ( ( rc = solClient_msg_setBinaryAttachment ( msg_p, text_p, ( solClient_uint32_t ) strlen ( (char *)text_p ) ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_setBinaryAttachment()" );
        goto freeMessage;
    }

    /* Set the destination. */
    destination.destType = SOLCLIENT_TOPIC_DESTINATION;
    destination.dest = topic_p;
    if ( ( rc = solClient_msg_setDestination ( msg_p, &destination, sizeof ( destination ) ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_setDestination()" );
        goto freeMessage;
    }

    /* Send the message. */
    if ( ( rc = solClient_session_sendMsg ( session_p, msg_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_session_sendMsg()" );
        goto freeMessage;
    }

  freeMessage:
    if ( ( rcFreeMsg = solClient_msg_free ( &msg_p ) ) != SOLCLIENT_OK ) {
        common_handleError ( rcFreeMsg, "solClient_msg_free()" );
    }

    return rc;
}


/*****************************************************************************
 * common_cacheEventCallback
 *****************************************************************************/
void
common_cacheEventCallback ( solClient_opaqueSession_pt opaqueSession_p, solCache_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
    printf ( "common_cacheEventCallback() called - %s\n"
             "topic: %s\n"
             "responseCode: (%d) %s\n"
             "subCode: (%d) %s\n"
             "cacheRequestId: %llu\n\n",
             solClient_cacheSession_eventToString ( eventInfo_p->cacheEvent ),
             eventInfo_p->topic,
             eventInfo_p->rc, solClient_returnCodeToString ( eventInfo_p->rc ),
             eventInfo_p->subCode, solClient_subCodeToString ( eventInfo_p->subCode ), eventInfo_p->cacheRequestId );
}


/*****************************************************************************
 * common_eventCallback
 *****************************************************************************/
void
common_eventCallback ( solClient_opaqueSession_pt opaqueSession_p,
                       solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
    solClient_errorInfo_pt errorInfo_p;

    switch ( eventInfo_p->sessionEvent ) {
        case SOLCLIENT_SESSION_EVENT_UP_NOTICE:
        case SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT:
        case SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_OK:
        case SOLCLIENT_SESSION_EVENT_CAN_SEND:
        case SOLCLIENT_SESSION_EVENT_RECONNECTING_NOTICE:
        case SOLCLIENT_SESSION_EVENT_RECONNECTED_NOTICE:
        case SOLCLIENT_SESSION_EVENT_PROVISION_OK:
        case SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_OK:

            /* Non-error events are logged at the INFO level. */
            solClient_log ( SOLCLIENT_LOG_INFO,
                            "common_eventCallback() called - %s\n",
                            solClient_session_eventToString ( eventInfo_p->sessionEvent ));
            break;

        case SOLCLIENT_SESSION_EVENT_DOWN_ERROR:
        case SOLCLIENT_SESSION_EVENT_CONNECT_FAILED_ERROR:
        case SOLCLIENT_SESSION_EVENT_REJECTED_MSG_ERROR:
        case SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR:
        case SOLCLIENT_SESSION_EVENT_RX_MSG_TOO_BIG_ERROR:
        case SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_ERROR:
        case SOLCLIENT_SESSION_EVENT_PROVISION_ERROR:

            /* Extra error information is available on error events */
            errorInfo_p = solClient_getLastErrorInfo (  );
            /* Error events are output to STDOUT. */
            printf ( "common_eventCallback() called - %s; subCode %s, responseCode %d, reason %s\n",
                     solClient_session_eventToString ( eventInfo_p->sessionEvent ),
                     solClient_subCodeToString ( errorInfo_p->subCode ), errorInfo_p->responseCode, errorInfo_p->errorStr );
            break;

        default:
            /* Unrecognized or deprecated events are output to STDOUT. */
            printf ( "common_eventCallback() called - %s.  Unrecognized or deprecated event.\n",
                     solClient_session_eventToString ( eventInfo_p->sessionEvent ) );
            break;
    }
}


/*****************************************************************************
 * common_eventPerfCallback
 *****************************************************************************/
void
common_eventPerfCallback ( solClient_opaqueSession_pt opaqueSession_p,
                           solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
}

/*****************************************************************************
 * common_flowEventCallback
 *****************************************************************************/
void
common_flowEventCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_flow_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
    solClient_errorInfo_pt errorInfo_p;

    switch ( eventInfo_p->flowEvent ) {
        case SOLCLIENT_FLOW_EVENT_UP_NOTICE:
        case SOLCLIENT_FLOW_EVENT_SESSION_DOWN:
        case SOLCLIENT_FLOW_EVENT_ACTIVE:
        case SOLCLIENT_FLOW_EVENT_INACTIVE:

            /* Non error events are logged at the INFO level. */
            solClient_log ( SOLCLIENT_LOG_INFO,
                            "common_flowEventCallback() called - %s\n",
                            solClient_flow_eventToString ( eventInfo_p->flowEvent ));
            break;

        case SOLCLIENT_FLOW_EVENT_DOWN_ERROR:
        case SOLCLIENT_FLOW_EVENT_BIND_FAILED_ERROR:
        case SOLCLIENT_FLOW_EVENT_REJECTED_MSG_ERROR:

            /* Extra error information is available on error events */
            errorInfo_p = solClient_getLastErrorInfo (  );
            /* Error events are output to STDOUT. */
            printf ( "common_flowEventCallback() called - %s; subCode %s, responseCode %d, reason %s\n",
                     solClient_flow_eventToString ( eventInfo_p->flowEvent ),
                     solClient_subCodeToString ( errorInfo_p->subCode ), errorInfo_p->responseCode, errorInfo_p->errorStr );
            break;

        default:

            /* Unrecognized or deprecated events are output to STDOUT. */
            printf ( "common_flowEventCallback() called - %s.  Unrecognized or deprecated event.\n",
                     solClient_flow_eventToString ( eventInfo_p->flowEvent ) );
            break;
    }
}


/*****************************************************************************
 * common_flowMessageReceiveCallback
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
common_flowMessageReceiveCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_msgId_t msgId;
    int            *counter_p;

    if ( user_p == NULL ) {
        /* Note: solClient_msg_getMsgId will fail on Direct messages, but 
         * it should not get as the callback is for a Flow. */
        if ( solClient_msg_getMsgId ( msg_p, &msgId ) == SOLCLIENT_OK ) {
            printf ( "Received message on flow. (Message ID: %lld).\n", msgId );
        } else {
            printf ( "Received message on flow.\n" );
        }
    } else {
        counter_p = ( int * ) user_p;
        ( *counter_p )++;
    }

    /* 
     * Returning SOLCLIENT_CALLBACK_OK causes the API to free the memory 
     * used by the message. This is important to avoid leaks.
     */
    return SOLCLIENT_CALLBACK_OK;
}

/*****************************************************************************
 * common_flowMessageReceiveAckCallback
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
common_flowMessageReceiveAckCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_msgId_t msgId;

    /* Note: solClient_msg_getMsgId will fail on Direct messages, but 
     * it should not get as the callback is for a Flow. */
    if ( solClient_msg_getMsgId ( msg_p, &msgId ) == SOLCLIENT_OK ) {
        printf ( "Acknowledging message Id: %lld.\n", msgId );
        solClient_flow_sendAck ( opaqueFlow_p, msgId );
      
    } else {
        printf ( "Received message on flow.\n" );
    }

    /* 
     * Returning SOLCLIENT_CALLBACK_OK causes the API to free the memory 
     * used by the message. This is important to avoid leaks.
     */
    return SOLCLIENT_CALLBACK_OK;
}

/*****************************************************************************
 * common_flowMessageReceivePrintMsgCallback
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
common_flowMessageReceivePrintMsgCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;

    printf ( "Received message:\n" );
    if ( ( rc = solClient_msg_dump ( msg_p, NULL, 0 ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_dump()" );
        return SOLCLIENT_CALLBACK_OK;
    }

    printf ( "\n" );

    /*
     * Returning SOLCLIENT_CALLBACK_OK causes the API to free the memory
     * used by the message. This is important to avoid leaks.
     */
    return SOLCLIENT_CALLBACK_OK;
}


/*****************************************************************************
 * common_flowMessageReceivePrintMsgAndAckCallback
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
common_flowMessageReceivePrintMsgAndAckCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;
    solClient_msgId_t msgId;

    printf ( "Received message:\n" );
    if ( ( rc = solClient_msg_dump ( msg_p, NULL, 0 ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_dump()" );
        return SOLCLIENT_CALLBACK_OK;
    }

    printf ( "\n" );

    /* Acknowledge the message after processing it. */
    if ( solClient_msg_getMsgId ( msg_p, &msgId )  == SOLCLIENT_OK ) {
        printf ( "Acknowledging message Id: %lld.\n", msgId );
        solClient_flow_sendAck ( opaqueFlow_p, msgId );
    }

    /* 
     * Returning SOLCLIENT_CALLBACK_OK causes the API to free the memory 
     * used by the message. This is important to avoid leaks.
     */
    return SOLCLIENT_CALLBACK_OK;
}


/*****************************************************************************
 * common_messageReceiveCallback
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
common_messageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;

    solClient_int64_t rxSeqNum;
    const char     *senderId_p;

    /* 
     * Get the message sequence number and sender ID. Check to see if the 
     * fields exist, and use a default value if the field is not found.
     */

    if ( ( rc = solClient_msg_getSequenceNumber ( msg_p, &rxSeqNum ) ) != SOLCLIENT_OK ) {
        if ( rc == SOLCLIENT_NOT_FOUND ) {
            rxSeqNum = 0;
        } else {
            common_handleError ( rc, "solClient_msg_getSequenceNumber()" );
            return SOLCLIENT_CALLBACK_OK;
        }
    }

    if ( ( rc = solClient_msg_getSenderId ( msg_p, &senderId_p ) ) != SOLCLIENT_OK ) {
        if ( rc == SOLCLIENT_NOT_FOUND ) {
            senderId_p = "";
        } else {
            common_handleError ( rc, "solClient_msg_getSenderId()" );
            return SOLCLIENT_CALLBACK_OK;
        }
    }

    if ( user_p != NULL ) {
        printf ( "%s received message from '%s' (seq# %llu)\n", ( char * ) user_p, senderId_p, rxSeqNum );
    } else {
        printf ( "Received message from '%s' (seq# %llu)\n", senderId_p, rxSeqNum );
    }

    /* 
     * Returning SOLCLIENT_CALLBACK_OK causes the API to free the memory 
     * used by the message. This is important to avoid leaks.
     */
    return SOLCLIENT_CALLBACK_OK;

}


/*****************************************************************************
 * common_messageReceivePrintMsgCallback
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
common_messageReceivePrintMsgCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;

    if ( user_p != NULL ) {
        printf ( "%s Received message:\n", (char *)user_p );
    } else {
        printf ( "Received message:\n" );
    }
    if ( ( rc = solClient_msg_dump ( msg_p, NULL, 0 ) ) != SOLCLIENT_OK ) {
        common_handleError ( rc, "solClient_msg_dump()" );
        return SOLCLIENT_CALLBACK_OK;
    }

    printf ( "\n" );

    /* 
     * Returning SOLCLIENT_CALLBACK_OK causes the API to free the memory 
     * used by the message. This is important to avoid leaks.
     */
    return SOLCLIENT_CALLBACK_OK;
}


/*****************************************************************************
 * common_messageReceivePerfCallback
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
common_messageReceivePerfCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    /* 
     * Returning SOLCLIENT_CALLBACK_OK causes the API to free the memory 
     * used by the message. This is important to avoid leaks.
     */
    return SOLCLIENT_CALLBACK_OK;
}

/*****************************************************************************
 * Request-Reply: Convert operator type to string
 ******************************************************************************/
const char *
RR_operationToString ( RR_operation_t operation )
{
    static char    *plusString = "PLUS";
    static char    *minusString = "MINUS";
    static char    *timesString = "TIMES";
    static char    *divideString = "DIVIDED_BY";
    static char    *unknownString = "UNKNOWN";

    switch ( operation ) {
        case plusOperation:
            return plusString;
        case minusOperation:
            return minusString;
        case timesOperation:
            return timesString;
        case divideOperation:
            return divideString;
        default:
            return unknownString;
    }
}

