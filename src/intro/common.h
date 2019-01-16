
/** example ex/common.h
 */

/**
 *
 * file common.h Include file for the Solace C API samples.
 *
 * Copyright 2007-2019 Solace Corporation. All rights reserved.
 *
 * This include file provides common utilities used throughout the C API 
 * samples.
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"
#include "solclient/solCache.h"
#include "os.h"


/**
 * @anchor commonSampleValues
 * @name Common Sample values
 * Values that can be used by sample applications.
 */

/*@{*/

#define COMMON_MY_SAMPLE_TOPIC   "my/sample/topic"         /**< A sample topic. */

#define COMMON_SEMP_TOPIC_FORMAT "#SEMP/%s/SHOW"    /**< Format for SEMP topics. */

#define COMMON_TESTQ             "my_sample_queue"         /**< A durable Queue name. */

#define COMMON_TESTDTE           "my_sample_topicendpoint" /**< A durable Topic Endpoint name. */

#define COMMON_DMQ_NAME          "#DEAD_MSG_QUEUE"         /**< Name of a Dead Message Queue (DMQ). */

#define COMMON_ATTACHMENT_TEXT   "my attached data"        /**< Sample attachment. */
        
/*@}*/


/** 
 * @enum flowMode
 * Subscribe modes.
 */
enum flowMode
{

    SUBSCRIBER = 0,     /**< Regular subscribe mode. */

    QUEUE = 1,          /**< Queue mode. */

    TE = 2,             /**< Topic Endpoint mode. */

    DIRECT = 3          /**< Direct subscribe mode. */
};


/**
 * @anchor optionRequirementFlags
 * @name Option Requirements Flag
 * Flags used to identify which command line options are required or optional
 * when using common_initCommandOptions() to initialize the commonOptions 
 * structure
 */

/*@{*/

#define HOST_PARAM_MASK        0x0001      /**< Host option. */
#define USER_PARAM_MASK        0x0002      /**< User option. */
#define DEST_PARAM_MASK        0x0004      /**< Topic option. */
#define PASS_PARAM_MASK        0x0008      /**< Password option. */
#define CACHE_PARAM_MASK       0x0010      /**< Cache name option. */
#define DURABLE_MASK           0x0020      /**< Durable Endpoint option. */
#define NUM_MSGS_MASK          0x0040      /**< Number of Messages option. */
#define MSG_RATE_MASK          0x0080      /**< Message Rate option. */
#define WINDOW_SIZE_MASK       0x0100      /**< Window Size option. */
#define LOG_LEVEL_MASK         0x0200      /**< Log Level option. */
#define USE_GSS_MASK           0x0400      /**< Enable Kerberos option. */
#define ZIP_LEVEL_MASK         0x0800      /**< Zip Compression Level option. */
#define REPLAY_START_MASK      0x1000      /**< Replay Start Location option. */

/*@}*/

/**
 * @anchor optionDescriptions
 * @name Option Description Strings
 * Strings that will be displayed in Usage for each paramter
 * or option.
 */

/*@{*/
#define HOST_PARAM_STRING        "\t-c, --cip=[Protocol:]Host[:Port] Protocol, host and port of the messaging appliance (e.g. --cip=tcp:192.168.160.101).\n"
#define USER_PARAM_STRING        "\t-u, --cu=user[@vpn] Client username and Mesage VPN name. The VPN name is optional and\n"\
                                 "\t                      only used in a Solace messaging appliance running SolOS-TR. \n"

#define DEST_PARAM_STRING        "\t-t, --topic=Topic   Topic or Destination String. \n"
#define PASS_PARAM_STRING        "\t-p, --cp=password   Client password. \n"
#define CACHE_PARAM_STRING       "\t-a, --cache         Distributed Cache Name.\n"
#define DURABLE_STRING           "\t-d, --durable       Use durable endpoint (default: temporary)\n"
#define NUM_MSGS_STRING          "\t-n, --mn            Number of Messages.\n"
#define MSG_RATE_STRING          "\t-r, --mr            Message Rate.\n"
#define WINDOW_SIZE_STRING       "\t-w, --win           Window Size.\n"
#define LOG_LEVEL_STRING         "\t-l, --log=loglevel  API and application logging level (debug, info, notice, warn, error, critical).\n"
#define USE_GSS_STRING           "\t-g, --gss           Use GSS (Kerberos) authentication. When specified the '--cu' option is ignored.\n"
#define ZIP_LEVEL_STRING         "\t-z, --zip           Enable compression (set compress level=9 for SolOS-TR appliances only).\n"
#define REPLAY_START_STRING      "\t-R, --replay=replay Replay Start Location String (BEGINNING or RFC3339 time stamp).\n"

/*@}*/

/**
 * @struct commonOptions
 * The structure used to store common options. Most of these options are
 * parsed from the command line.
 */
struct commonOptions
{
    char            targetHost[256];
    char            username[SOLCLIENT_SESSION_PROP_MAX_USERNAME_LEN + 1];
    char            password[SOLCLIENT_SESSION_PROP_MAX_PASSWORD_LEN + 1];
    char            vpn[SOLCLIENT_SESSION_PROP_MAX_VPN_NAME_LEN + 1];
    char            destinationName[SOLCLIENT_BUFINFO_MAX_TOPIC_SIZE + 1];
    char            cacheName[SOLCLIENT_CACHESESSION_MAX_CACHE_NAME_SIZE + 1];
    char            replayStartLocation[250];
    int             usingTopic;
    int	            usingAD;
    int             numMsgsToSend;
    int             msgRate;
    int             gdWindow;
    int             requiredFields;
    int             optionalFields;
    solClient_log_level_t logLevel;
    int             usingDurable;
    int             enableCompression;
    int             useGSS;
};



/**
 * This function prints C API version to STDOUT.
 */
void
    common_printCCSMPversion (  );


/**
 * Given a solClient_returnCode_t and a call specific error string, this
 * function logs an error message including the solClient error code.
 * @param rc A return code to be interpretted and included in the logged
 * message.
 * @param errorStr An error message to be logged.
 */
void
    common_handleError ( solClient_returnCode_t rc, const char *errorStr );


/**
 * This function parses a string of the form 'username@vpn' into its sub
 * components.  When completed the outUsername and outVpn char pointers will
 * point to the username and vpn, respectively.  If there is no @vpn, the vpn
 * name string passed in is left untouched.
 * @param inName The username@vpn string to be parsed.
 * @param outUsername A pointer to the username.
 * @param usernameLen The (max) length of the username.
 * @param outVpn A pointer to the vpn.
 * @param vpnLen The (max) length of the vpn.
 */
void
    common_parseUsernameAndVpn ( const char *inName, char *outUsername, size_t usernameLen, char *outVpn, size_t vpnLen );


/**
 * This function parses the command line options and populates a commonOptions
 * struct with the information.
 * @param argc The number of command line arguments
 * @param argv The values of the command line arguments
 * @param copt The commonOptions struct to be filled.
 * @param positionalDesc A description of position parameters that may follow options. NULL if there are no 
 *             positional parameters.
 * @return 1 , 0
 */
int
    common_parseCommandOptions ( int argc, char **argv, struct commonOptions *copt, const char *positionalDesc );


/**
 * This function creates a SolClient session and connects the Session.
 * @param context_p A pointer to the Context in which the Session is to be
 * created.
 * @param session_p A pointer to a Session pointer. This Session pointer
 * points to the newly created session upon completion of this function.
 * @param msgCallback_p A message callback function be used while creating
 * the new Session.
 * @param user_p A pointer to the user-defined data to be associated with
 * the Session. If no user-defined data is required, a value of NULL should
 * be used.
 * @param commonOpts A pointer to the sample's commonOptions struct.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 */
solClient_returnCode_t
    common_createAndConnectSession ( solClient_opaqueContext_pt context_p,
                                 solClient_opaqueSession_pt * session_p,
                                 solClient_session_rxMsgCallbackFunc_t msgCallback_p,
                                 solClient_session_eventCallbackFunc_t eventCallback_p,
                                 void *user_p, struct commonOptions *commonOpts );


/*****************************************************************************
 * common_createQueue
 *****************************************************************************/

solClient_returnCode_t
    common_createQueue ( solClient_opaqueSession_pt session_p, const char *queueName_p );

/*****************************************************************************
 * common_deleteQueue
 *****************************************************************************/

solClient_returnCode_t
    common_deleteQueue ( solClient_opaqueSession_pt session_p, const char *queueName_p );

/**
 * This function publishes a message to the Topic provided as an argument using
 * the provided session. It uses Direct delivery mode. 
 * @param session_p A pointer to the Session.
 * @param topic_p   The Topic to publish on.
 * message.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 */
solClient_returnCode_t
    common_publishMessage ( solClient_opaqueSession_pt session_p, char *topic_p, solClient_uint32_t deliveryMode );


/**
 * A callback for cache events. The callback is given when making non-blocking
 * cache requests to perform actions when a cache event occurs.
 * @param opaqueSession_p A pointer to the session to which the event applies.
 * This pointer is never NULL.
 * @param evenInfo_p A pointer to information about the cache event, such as
 * the event type. This pointer is never NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 */
void
    common_cacheEventCallback ( solClient_opaqueSession_pt opaqueSession_p, solCache_eventCallbackInfo_pt eventInfo_p, void *user_p );


/**
 * A callback for Session events. The callback is registered for a Session
 * and is called whenever a Session event occurs. An event summary is output
 * to STDOUT.
 * @param opaqueSession_p A pointer to the session to which the event applies.
 * This pointer is never NULL.
 * @param evenInfo_p A pointer to information about the cache event, such as
 * the event type. This pointer is never NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 */
void
    common_eventCallback ( solClient_opaqueSession_pt opaqueSession_p,
                        solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p );


/**
 * A callback for Session events. The callback is registered for a Session
 * and is called whenever a Session event occurs. This performance event
 * callback does nothing.
 * @param opaqueSession_p A pointer to the Session to which the event applies.
 * This pointer is never NULL.
 * @param evenInfo_p A pointer to information about the cache event, such as
 * the event type. This pointer is never NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 */
void
    common_eventPerfCallback ( solClient_opaqueSession_pt opaqueSession_p,
                            solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p );


/**
 * A callback for flow events. The callback is registered for a Flow
 * and is called whenever a Flow event occurs.
 * @param opaqueFlow_p A pointer to the Flow to which the event applies.
 * This pointer is never NULL.
 * @param evenInfo_p A pointer to information about the cache event, such as
 * the event type. This pointer is never NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 */
void
    common_flowEventCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_flow_eventCallbackInfo_pt eventInfo_p, void *user_p );


/**
 * A callback for received messages on a Flow. The callback is registered for 
 * a Flow and is called whenever a message is received.
 * This callback prints a short message receipt acknowledgement to STDOUT.
 * @param opaqueFlow_p A pointer to the Flow receiving the message.
 * This pointer is never NULL.
 * @param msg_p A pointer to the received message. This pointer is never NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 * @return ::SOLCLIENT_CALLBACK_OK
 */
solClient_rxMsgCallback_returnCode_t
    common_flowMessageReceiveCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p );


/**
 * A callback for received messages on a Flow. The callback is registered for 
 * a Flow and is called whenever a message is received.
 * This callback calls solClient_flow_sendAck to acknowledge received messages at the application layer.
 * @param opaqueFlow_p A pointer to the Flow receiving the message.
 * This pointer is never NULL.
 * @param msg_p A pointer to the received message. This pointer is never NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 * @return ::SOLCLIENT_CALLBACK_OK
 */
solClient_rxMsgCallback_returnCode_t
    common_flowMessageReceiveAckCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p );


/**
 * Initialze struct commonOptions, used by all examples before parsing 
 * the command line.
 * @param commonOpt_p  A pointer to the commonOptions structure to be initialized.
 */
void
    common_initCommandOptions ( struct commonOptions *commonOpt, 
                            int requiredParams,
                            int optionals);

/**
 * A callback for received messages on a Flow. The callback is registered for 
 * a Flow and is called whenever a message is received.
 * This callback dumps the contents of the message to STDOUT.
 * @param opaqueFlow_p A pointer to the flow receiving the message.
 * This pointer is never NULL.
 * @param msg_p A pointer to the received message. This pointer is never
 * NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 * @return ::SOLCLIENT_CALLBACK_OK
 */
solClient_rxMsgCallback_returnCode_t
    common_flowMessageReceivePrintMsgCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p );

/**
 * A callback for received messages on a Flow. The callback is registered for
 * a Flow and is called whenever a message is received.
 * This callback dumps the contents of the message to STDOUT and calls
 * solClient_flow_sendAck() after printing the message contents.
 * @param opaqueFlow_p A pointer to the flow receiving the message.
 * This pointer is never NULL.
 * @param msg_p A pointer to the received message. This pointer is never
 * NULL.
 * @param user_p A pointer to opaque user data provided when the callback is
 * registered.
 * @return ::SOLCLIENT_CALLBACK_OK
 */
solClient_rxMsgCallback_returnCode_t
    common_flowMessageReceivePrintMsgAndAckCallback ( solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p );

/**
 * A callback for received messages by a Session. The callback is registered
 * for a Session and is called whenever a message is received.
 * This callback prints a short message receipt acknowledgement to STDOUT.
 * @param opaqueSession_p A pointer to the Session receiving the message.
 * This pointer is never NULL.
 * @param msg_p A pointer to the received message. This pointer is never 
 * NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 * @return ::SOLCLIENT_CALLBACK_OK
 */
solClient_rxMsgCallback_returnCode_t
    common_messageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p );


/**
 * A callback for received messages by a Session. The callback is registered
 * for a Session and is called whenever a message is received.
 * This callback dumps the contents of the message to STDOUT.
 * The user_p parameter is assumed to point to a NULL terminated char array
 * containing a user specified name for the Session.
 * @param opaqueSession_p A pointer to the Session receiving the message.
 * This pointer is never NULL.
 * @param msg_p A pointer to the received message. This pointer is never
 * NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 * @return ::SOLCLIENT_CALLBACK_OK
 */
solClient_rxMsgCallback_returnCode_t
    common_messageReceivePrintMsgCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p );


/**
 * A callback for received messages by a Session. The callback is registered
 * for a Session and is called whenever a message is received.
 * This callback does nothing.
 * @param opaqueSession_p A pointer to the session receiving the message.
 * This pointer is never NULL.
 * @param msg_p A pointer to the received message. This pointer is never
 * NULL.
 * @param user_p A pointer to opaque user data provided when the callback is 
 * registered.
 * @return ::SOLCLIENT_CALLBACK_OK
 */
solClient_rxMsgCallback_returnCode_t
    common_messageReceivePerfCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p );

#endif /* COMMON_H_ */
