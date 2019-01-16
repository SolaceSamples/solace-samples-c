/**
*
* @file solCache.h Include file for the Solace Corporation Messaging API for C
*
* Copyright 2008-2018 Solace Corporation. All rights reserved.
*
* This include file provides the public constants and API calls for 
* applications that interface to the Solace Corporation SolCache.
*/
/** @page solcachepage SolCache API
*
* @section solcache SolCache API
* The SolCache interface is an extension of the CCSMP interface (that is, the C API). 
* A SolCache object is created (solClient_session_createCacheSession()) in a Session to send messages to a
* Solace Corporation Cache Cluster.  A single Session may have many SolCache objects. Responses
* from the cache are received in the usual Session rxCallback function.
*
* @see \ref solcachecfg "Client Session Configuration Properties"
* \n
* \ref cacherequestflags  "CacheRequest Flag Types"
*/

#ifndef _SOLCACHE_H_
#define _SOLCACHE_H_

#include "solClient.h"

#if defined(__cplusplus)
extern "C"
{
#endif                          /* _cplusplus */

/** 
 *
 * @anchor solcachecfg
 * @name Client Session Configuration Properties
 * Items that can be configured in a client application that wants to 
 * retrieve messages from the cache.
 */
/*@{*/
#define SOLCLIENT_CACHESESSION_PROP_CACHE_NAME                 "CACHESESSION_CACHE_NAME"    /**< The identifier for the Distributed Cache to send cache requests to. */
#define SOLCLIENT_CACHESESSION_PROP_MAX_MSGS                   "CACHESESSION_MAX_MSGS"      /**< The maximum number of messages to retrieve from the cache for any one Topic. */
#define SOLCLIENT_CACHESESSION_PROP_MAX_AGE                    "CACHESESSION_MAX_AGE"       /**< The oldest message (in seconds) to retrieve from the cache. */
#define SOLCLIENT_CACHESESSION_PROP_REQUESTREPLY_TIMEOUT_MS    "CACHESESSION_RR_TIMEOUT_MS" /**< The timeout period (in milliseconds) to wait for a response from the cache. The minimum configurable value is 3000 (3 seconds). This is a protocol timer used internally by the API on each message exchange with solCache. A single call to solClient_cacheSession_sendCacheRequest() may lead to many request-reply exchanges with solCache and so is not bounded by this timer as long as each internal request is satisfied in time. */
#define SOLCLIENT_CACHESESSION_PROP_REPLY_TO                   "CACHESESSION_REPLY_TO"      /**< Deprecated: The reply-to topic for the cache request. It is preferred to leave this property unspecified and let the API use the Session P2P Reply To Topic: ::SOLCLIENT_SESSION_PROP_P2PINBOX_IN_USE. */
/*@} */

/** @name Default cacheSession Configuration Properties
* Default values for cacheSession configuration properties that are not explicitly set.
*/
/*@{*/
#define SOLCLIENT_CACHESESSION_PROP_DEFAULT_CACHE_NAME                ""      /**< There is no default cache name. */
#define SOLCLIENT_CACHESESSION_PROP_DEFAULT_MAX_MSGS                  "1"     /**< The default maximum number of message to retrieve from the cache for any one topic is 1. */
#define SOLCLIENT_CACHESESSION_PROP_DEFAULT_MAX_AGE                   "0"     /**< Default maximum age is no maximum age (set to zero). */
#define SOLCLIENT_CACHESESSION_PROP_DEFAULT_REQUESTREPLY_TIMEOUT_MS   "10000" /**< The default timeout (in milliseconds) to wait for a response from the cache is 10000 (10 seconds). */
#define SOLCLIENT_CACHESESSION_PROP_DEFAULT_REPLY_TO                  ""      /**< The default when no reply-to topic is specified is the ::SOLCLIENT_SESSION_PROP_P2PINBOX_IN_USE for the Session. */

#define SOLCLIENT_CACHESESSION_MAX_CACHE_NAME_SIZE                     200
/*@}*/
/**
 *
 *  SolCache events. The list of events that can be passed to the 
 *  event callback function.
 */

typedef enum solCache_event {
    SOLCACHE_EVENT_REQUEST_COMPLETED_NOTICE     /** Cache Request has finished. The event returnCode and subCode provide status information */
} solCache_event_t;

typedef void *solClient_opaqueCacheSession_pt;  /**< An opaque pointer to a cache session. */

/**
 *
 * SolCache event callback information. When the application registers
 * an event callback pointer with a cache request (for asynchronous cache 
 * requests), the callback function receives a pointer to the event
 * callback information structure.
 */

typedef struct solCache_eventCallbackInfo {
    solCache_event_t            cacheEvent;
    const char                 *topic;
    solClient_returnCode_t      rc;
    solClient_subCode_t         subCode;
    solClient_uint64_t          cacheRequestId;
} solCache_eventCallbackInfo_t, *solCache_eventCallbackInfo_pt;

/**
 * A prototype for the eventCallback function that can be registered by the 
 * application.
 */

typedef void (*solCache_eventCallbackFunc_t) (
                solClient_opaqueSession_pt      opaqueSession_p,
                solCache_eventCallbackInfo_pt   eventInfo_p,
                void *                          user_p);

typedef solClient_uint32_t solClient_cacheRequestFlags_t;       /**< A set of flags that can be provided to the solClient_cacheSession_sendCacheRequest() see \ref cacherequestflags "CacheRequest Flag Types" */
/**
 * @anchor cacherequestflags
 *
 * @name CacheRequest Flag Types
 *
 * Values that can be used as part of the \link ::solClient_cacheRequestFlags_t cacheRequestFlags\endlink field to 
 * solClient_session_sendCacheRequest().  
 * 
 * The following live data actions determine when a cache request completes and how live data is handled while the 
 * cache request is outstanding. The actions are mutually exclusive, and one must be specified. 
 * @li SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FULFILL
 * @li SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_QUEUE
 * @li SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FLOWTHRU
  *
 * The following flags can be added to the live data action to further modify the behavior of the cache request:
 * @li SOLCLIENT_CACHEREQUEST_FLAGS_NO_SUBSCRIBE
 * @li SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY 
 * 
 */
/*@{*/
#define SOLCLIENT_CACHEREQUEST_FLAGS_NO_SUBSCRIBE      (0x01)  /**< Do not send a subscription request to the appliance before sending the cache request. This flag is used if the client is already subscribed to the cache request topic.*/
#define SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FULFILL  (0x02)  /**< The cache request completes when the cache response is returned or when live data that matches the cache request topic arrives, whichever happens first. In the latter case, the cache request completes as soon as live data arrives. The status of the cache response is SOLCLIENT_OK, unless part of the response is flagged as suspect, in which case the response is SOLCLIENT_INCOMPLETE. 
This is a typical last value cache mode where only the most recent data is of interest and the application is not interested in all data available on the topic. */

#define SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_QUEUE    (0x04)  /**< The cache request completes when the cache response is returned. Live data matching the cache request topic is queued until the cache request completes.  Data from the cache-response is delivered before queued live data.
The queued live data is delivered to the application before the cache request completes. It is possible for queued live data to duplicate data in the cache-response. It is the responsibility of the application to handle duplication by some application specific method.  For example, by using an end-to-end message-id in the message meta-data.  */

#define SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FLOWTHRU (0x08)  /**< The cache request completes when the cache response is returned. Live data matching the cache request topic is delivered immediately to the application while the cache request is outstanding.
SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FLOWTHRU is the only live data action that supports wildcard topic cache requests.  */
#define SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY      (0x10)  /**< By default, solClient_cacheSession_sendCacheRequest blocks until the cache request completes and then returns the cache request status. If the cache request flag SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY is set, solClient_cacheSession_sendCacheRequest returns immediately with the status SOLCLIENT_IN_PROGRESS, and the cache request status is returned through a callback when it completes.  */

/*@}*/

/**
* Returns a string representation of the cache event passed in.
* @param  cacheEvent The Session event to convert to a string representation.
* @return A pointer to a constant character string. This pointer is never NULL.
*/
solClient_dllExport const char
*solClient_cacheSession_eventToString (solCache_event_t cacheEvent);
                                              
/**
 * Create a cache session object that is used in subsequent cacheRequests on the 
 * Session. Multiple cache session objects may be created on a Session. Each must be 
 * destroyed when the application no longer requires the object.
 *
 * @param props                 Array of name/value string pair pointers to configure 
 *                              cache session properties.
 * @param opaqueSession_p       Session in which the cache session is to be created.
 * @param opaqueCacheSession_p  Pointer to the location to contain the opaque cache session pointer
 *                              on return.
 * @returns                     ::SOLCLIENT_OK on success. ::SOLCLIENT_FAIL on failure.
 * @subcodes
 * @see ::solClient_subCode for a description of subcodes.
 */

solClient_dllExport solClient_returnCode_t 
solClient_session_createCacheSession(const char * const *         props,
                                     solClient_opaqueSession_pt   opaqueSession_p,
                                     solClient_opaqueCacheSession_pt *opaqueCacheSession_p);
/**
 * Destroy a cache session object.
 * This function is thread safe and can be called from any thread. When this function is invoked:
 * @li All blocked synchronous cache requests return immediately with SOLCLIENT_INCOMPLETE return code
 * and SOLCLIENT_SUBCODE_PARAM_NULL_PTR subcode.
 * @li Live messages that have been queued (if any) will be delivered.
 *
 * @param opaqueCacheSession_p  Pointer to opaque cache session pointer that was returned
 *                              when the cache session was created.
 * @returns ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_cacheSession_destroy(solClient_opaqueCacheSession_pt   *opaqueCacheSession_p);

/**
 * Send a cache request message. If the cache request flag
 * ::SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY is set, this function returns ::SOLCLIENT_IN_PROGRESS
 * immediately upon successful buffering of the message for transmission. 
 * Otherwise this function waits for the cache response to be fulfilled 
 * according to the LIVEDATA handling options. When the function waits for the cache response the
 * cache event callback is not invoked.
 *
 * Irrespective of the cache request flag, ::SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY, cache requests 
 * may be flow controlled if the underlying transport is flow controlled. The transport is considered flow controlled if 
 * the library is unable to write to the transport device (for example, the TCP socket is full), or if there are more than
 * 1000 session requests (solClient_session_sendRequest() + solClient_cacheSession_sendCacheRequest())
 * outstanding. This causes solClient_cacheSession_sendCacheRequest() to block if the session property,
 * ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING is enabled. If ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING is
 * disabled and it is not possible to write the cache request to the underlying transport,
 * SOLCLIENT_WOULD_BLOCK is returned.
 * 
 * Cached messages received in response to the cache request are delivered to the application 
 * through the usual receive message callback as the messages arrive. This function returns when all
 * cache responses have been received, or the request is either completed by live data (::SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FULFILL) or by a timeout. If, and only if, ::SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY
 * is set, the cache request callback is invoked when any of these terminating conditions occurs.
 * 
 * @param opaqueCacheSession_p An opaque cache session pointer returned when the cache session was
 *                        created.
 * @param topic_p         The string that contains the topic being requested from 
 *                        the cache.
 * @param cacheRequestId The 64-bit integer returned to the application in the cache request
 *                        response; it is available in every cached message returned.
 * @param callback_p      A callback pointer for an asynchronous reply to cache requests.
 * @param user_p          A user pointer to return with the callback.
 * @param cacheflags      \ref cacherequestflags "cacheRequest flags" to modify the cache request behavior.
 * @param subscribeFlags  Subscription flags (::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE)
 * @returns ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_INCOMPLETE, ::SOLCLIENT_IN_PROGRESS, ::SOLCLIENT_WOULD_BLOCK
 * @subcodes
 * This function can return ::SOLCLIENT_FAIL for any of the following reasons:
 * @li ::SOLCLIENT_SUBCODE_CACHE_INVALID_SESSION - the underlying session in which the cacheSession 
 *                        was created has been destroyed.
 * @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX  - the topic is invalid.
 * @li ::SOLCLIENT_SUBCODE_TOPIC_TOO_LARGE       - the topic exceeds the maximum length.
 * 
 * When the ::SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY is set in cacheFlags, the function returns 
 * ::SOLCLIENT_IN_PROGRESS and the subsequent callback indicates the 
 * final status of the cache request.
 *
 * Otherwise the return code indicates the status of the cache request. 
 * ::SOLCLIENT_OK is returned when the cache request completes successfully and valid data is delivered.  The ::solClient_subCode is never
 * set when ::SOLCLIENT_OK is returned.
 *
 * ::SOLCLIENT_INCOMPLETE may be returned if the cacheRequest or initial 
 * subscription request is sent but not completed successfully.
 * @li ::SOLCLIENT_SUBCODE_CACHE_TIMEOUT         - the timeout specified by 
 *     ::SOLCLIENT_CACHESESSION_PROP_REQUESTREPLY_TIMEOUT_MS expired.
 * @li ::SOLCLIENT_SUBCODE_PROTOCOL_ERROR        - the cache response is malformed.
 * @li ::SOLCLIENT_SUBCODE_CACHE_ERROR_RESPONSE  - the cache responded with an error response.
 * @li ::SOLCLIENT_SUBCODE_CACHE_SUSPECT_DATA    - at least one suspect message was received 
 *                                                 in response.
 * @li ::SOLCLIENT_SUBCODE_CACHE_NO_DATA         - the cache request completed successfully with no 
 *                                                 suspect responses, but no data matching the cache 
 *                                                 request was found.
 * @li ::SOLCLIENT_SUBCODE_CACHE_REQUEST_CANCELLED - the cache request has been cancelled.
 * @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR          - the cache session has been destroyed.
 * @li ::SOLCLIENT_SUBCODE_CACHE_INVALID_SESSION   - the underlying session in which the cacheSession 
 *                        was created has been destroyed.
 *
 * @see ::solClient_subCode for a description of all sub-codes.
 */

solClient_dllExport solClient_returnCode_t
solClient_cacheSession_sendCacheRequest (solClient_opaqueCacheSession_pt opaqueCacheSession_p,
                            const char *                     topic_p,
                            solClient_uint64_t               cacheRequestId,
                            solCache_eventCallbackFunc_t     callback_p,
                            void *                           user_p,
                            solClient_cacheRequestFlags_t    cacheflags,
                            solClient_subscribeFlags_t       subscribeFlags);


#define SOLCACHE_INVALID_TOPICSEQUENCE_NUMBER   0LL     /**< A sequence number that is never used by the appliance.  When passed as an argument to solClient_cacheSession_sendCacheRequestSequence(), it is taken to mean oldest or newest sequence number for the start sequence number and end sequence number, respectively.  */

/**
 * Send a Cache Request Message. This function is used for SolCache-RS only.
 *
 * If the cache request flag
 * ::SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY is set, this function returns ::SOLCLIENT_IN_PROGRESS
 * immediately upon successful buffering of the message for transmission. 
 * Otherwise this function waits for the cache response to be fulfilled 
 * according to the LIVEDATA handling options. When the function waits for the cache response, the
 * cache event callback is not invoked.
 *
 * Irrespective of the cache request flag, ::SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY, cache requests 
 * may be flow controlled if the underlying transport is flow controlled. This causes 
 * solClient_cacheSession_sendCacheRequest() to block if the session property, ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING
 * is enabled. If ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING is disabled and it is not possible to write
 * the cache request to the underlying transport, SOLCLIENT_WOULD_BLOCK is returned.
 * 
 * Cached messages received in response to the cache request are delivered to the application 
 * through the usual receive message callback as the messages arrive. This function returns when all cache responses have
 * been returned, and the request is either completed by live data (::SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FULFILL),
 * or by timeout. If, and only if, ::SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY
 * is set, the cache request callback is invoked when any of these terminating conditions occurs.
 * 
 * @param opaqueCacheSession_p An opaque cache session pointer returned when the cache session was
 *                        created.
 * @param topic_p         The string that contains the topic being requested from 
 *                        the cache.
 * @param cacheRequestId The 64-bit integer returned to the application in the cache request
 *                        response; it is available in every cached message returned.
 * @param callback_p      A callback pointer for an asynchronous reply to cache requests.
 * @param user_p          A user pointer to return with the callback.
 * @param cacheflags      \ref cacherequestflags "cacheRequest flags" to modify the cache request behaviour
 * @param subscribeFlags  Subscription flags (::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE)
 * @param startSeqId      Starting sequence number for retrieved messages. If set to 
 *                        ::SOLCACHE_INVALID_TOPICSEQUENCE_NUMBER, then start at the oldest available message.
 *                        \note If both startSeqId and endSeqId are set to ::SOLCACHE_INVALID_TOPICSEQUENCE_NUMBER, only the newest message is retrieved.
 * @param endSeqId        End sequence number for retrieved messages, if set to ::SOLCACHE_INVALID_TOPICSEQUENCE_NUMBER, then all available messages starting at startSeqId are retrieved.
 * @returns ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_INCOMPLETE, ::SOLCLIENT_IN_PROGRESS, ::SOLCLIENT_WOULD_BLOCK
 * @subcodes
 * This function can return ::SOLCLIENT_FAIL for any of the following reasons:
 * @li ::SOLCLIENT_SUBCODE_CACHE_INVALID_SESSION - the underlying session in which the cacheSession 
 *                        was created has been destroyed.
 * @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX  - the topic is invalid.
 * @li ::SOLCLIENT_SUBCODE_TOPIC_TOO_LARGE       - the topic exceeds the maximum length.
 * 
 * When the ::SOLCLIENT_CACHEREQUEST_FLAGS_NOWAIT_REPLY is set in cacheFlags, the function returns 
 * ::SOLCLIENT_IN_PROGRESS and the subsequent callback indicates the 
 * final status of the cache request.
 *
 * Otherwise the return code indicates the status of the cache request. 
 * ::SOLCLIENT_OK is returned when the cache request completes successfully and valid data is delivered.  The ::solClient_subCode is never
 * set when ::SOLCLIENT_OK is returned.
 *
 * ::SOLCLIENT_INCOMPLETE may be returned if the cacheRequest or initial 
 * subscription request is sent but not completed successfully.
 * @li ::SOLCLIENT_SUBCODE_CACHE_TIMEOUT         - the timeout specified by 
 *     ::SOLCLIENT_CACHESESSION_PROP_REQUESTREPLY_TIMEOUT_MS expired.
 * @li ::SOLCLIENT_SUBCODE_PROTOCOL_ERROR        - the cache response is malformed.
 * @li ::SOLCLIENT_SUBCODE_CACHE_ERROR_RESPONSE  - the cache responded with an error response.
 * @li ::SOLCLIENT_SUBCODE_CACHE_SUSPECT_DATA    - at least one suspect message was received 
 *                                                 in response.
 * @li ::SOLCLIENT_SUBCODE_CACHE_NO_DATA         - the cache request completed successfully with no 
 *                                                 suspect responses but no data matching the cache 
 *                                                 request was found.
 * @li ::SOLCLIENT_SUBCODE_CACHE_REQUEST_CANCELLED - the cache request has been cancelled.
 * @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR          - the cache session has been destroyed.
 * @li ::SOLCLIENT_SUBCODE_CACHE_INVALID_SESSION   - the underlying session in which the cacheSession 
 *                        was created has been destroyed.
 *
 * @see ::solClient_subCode for a description of all sub-codes.
 */
solClient_dllExport solClient_returnCode_t
solClient_cacheSession_sendCacheRequestSequence (solClient_opaqueCacheSession_pt opaqueCacheSession_p,
                            const char *                     topic_p,
                            solClient_uint64_t               cacheRequestId,
                            solCache_eventCallbackFunc_t     callback_p,
                            void *                           user_p,
                            solClient_cacheRequestFlags_t    cacheflags,
                            solClient_subscribeFlags_t       subscribeFlags,
                            solClient_int64_t                startSeqId,
                            solClient_int64_t                endSeqId);


/**
 * Cancel all in progress cache requests  for a given cache session.
 * This function is thread safe and can be called from any thread. When this function is invoked:
 * @li All blocked synchronous cache requests return immediately with SOLCLIENT_INCOMPLETE return code
 * and SOLCLIENT_SUBCODE_CACHE_REQUEST_CANCELLED subcode.
 * @li A cache event SOLCACHE_EVENT_REQUEST_COMPLETED_NOTICE with a subcode of SOLCLIENT_SUBCODE_CACHE_REQUEST_CANCELLED
 * is generated for each in progress asynchronous cache request.
 * @li Live messages that have been queued (if any) will be delivered.
 * @li The associated cache session is still valid to use.
 *
 * @param opaqueCacheSession_p  Opaque cache session pointer that was returned
 *                              when the cache session was created.
 * @returns ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_cacheSession_cancelCacheRequests(
       solClient_opaqueCacheSession_pt   opaqueCacheSession_p);

#if defined(__cplusplus)
}
#endif                          /* _cplusplus */

#endif                          /* _SOLCACHE_H_ */
