/* Exclude everything from doxygen */
#ifndef DOXYGEN_SHOULD_SKIP_THIS
/**
*
* @file solClientDeprecated.h include file for the Solace Corporation Messaging API for C
*
* Copyright 2007-2018 Solace Corporation. All rights reserved.
*
* This include file provides the deprecated public constants and API calls for clients
* connecting to a Solace messaging appliance.  This file is provided for backwards 
* compatibility, allowing old applications to compile.
*
*/

#ifndef SOL_CLIENTDEPRECATED_H_
#define SOL_CLIENTDEPRECATED_H_


/* 
 * The following properties are deprecated. They can still be set, but have no effect and are silently
 * ignored. The definitions remain for backwards compatibility.
 */
#define SOLCLIENT_CONTEXT_PROP_MAX_APP_TIMERS "CONTEXT_MAX_APP_TIMERS" 
#define SOLCLIENT_CONTEXT_PROP_MAX_FLOWS      "CONTEXT_MAX_FLOWS"      
#define SOLCLIENT_CONTEXT_PROP_MAX_APP_FDS    "CONTEXT_MAX_APP_FDS"
#define SOLCLIENT_CONTEXT_PROP_MAX_SESSIONS   "CONTEXT_MAX_SESSIONS" 
#define SOLCLIENT_CONTEXT_PROP_MULTI_THREAD   "CONTEXT_MULTI_THREAD"


#define SOLCLIENT_CONTEXT_PROP_DEFAULT_MAX_APP_TIMERS "0"
#define SOLCLIENT_CONTEXT_PROP_DEFAULT_MAX_FLOWS      "100" 
#define SOLCLIENT_CONTEXT_PROP_DEFAULT_MAX_APP_FDS    "0"   
#define SOLCLIENT_CONTEXT_PROP_DEFAULT_MAX_SESSIONS   "1"   
#define SOLCLIENT_CONTEXT_PROP_DEFAULT_MULTI_THREAD   SOLCLIENT_PROP_ENABLE_VAL

#define SOLCLIENT_SESSION_PROP_SSL_PROTOCOL  "SESSION_SSL_PROTOCOL"  /**< This property specifies a comma separated list of the encryption protocol(s) to use. Allowed protocols are 'SSLv3', 'TLSv1', 'TLSv1.1', 'TLSv1.2'. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SSL_PROTOCOL. */
#define SOLCLIENT_SESSION_DEFAULT_PROP_SSL_PROTOCOL          "TLSv1.2,TLSv1.1,TLSv1,SSLv3"         /**< The default value for ::SOLCLIENT_SESSION_PROP_SSL_PROTOCOL*/ 
#define SOLCLIENT_SESSION_PROP_MAX_FLOWS                     "SESSION_MAX_FLOWS" /**< The maximum number of Flows supported on this Session. The valid range is 0..2000. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_MAX_FLOWS */

#define SOLCLIENT_SESSION_PROP_DEFAULT_MAX_FLOWS                     "100"       /**< The default maximum number of subscriber Flows that may be created in a Session. */

#define    SOLCLIENT_SUBCODE_SSL_LIBRARY_NOT_LOADED     SOLCLIENT_SUBCODE_LIBRARY_NOT_LOADED   /**< The client attempted to establish a session with a library not loaded.*/

/*
 * The following arbitrary constants have been eliminated and are 
 * no longer used.
 */
#define SOLCLIENT_SESSION_PROP_MAX_HOST_LEN        (128) /**< The maximum length of a single host entry in the Session host property, not including the NULL terminator. */
#define SOLCLIENT_MAX_SELECTOR_SIZE          (1023)        /**< The maximum length of a selector on solClient_session_createFlow(). */
/**
* @typedef solClient_bufInfo_ap
* An array of solClient_bufInfo_t is used to represent a message. The 
* array elements are enumerated with ::solClient_bufInfo_index and, when
* returned by the API, have a size of ::SOLCLIENT_BUFINFO_MAX_PARTS. 
* This array is used:
* @li By the message send function solClient_session_send().
* @li In the receive callback of prototype ::solClient_session_rxCallbackFunc_t,
* which is specified the call to solClient_session_create().
* @li In the receive callback of prototype ::solClient_flow_rxCallbackFunc_t,
* which is specified in solClient_session_createFlow().
* @li In the utility functions solClient_bufInfo_getConsumerIdCount() and
* solClient_bufInfo_getConsumerId().
*/
  typedef solClient_bufInfo_pt solClient_bufInfo_ap;

  typedef solClient_uint32_t solClient_sendFlags_t;            /**< A set of \ref sendflags "flags" that can be provided to a solClient_session_send() call. */
  typedef solClient_uint32_t solClient_receiveFlags_t;         /**< A set of \ref receiveflags "flags" received in a message receive callback. */

/**
* @enum solClient_bufInfo_index
* Index into array of solClient_bufInfo_t to access different message portions.
* A given message portion might or might not be present. A non-present portion is indicated
* by a NULL buffer pointer and a zero buffer size on receive. For send, setting the pointer
* for non-present parts to NULL is sufficient. In addition, for send, fewer parts can be
* provided (thus the buffer info array can be less than ::SOLCLIENT_BUFINFO_MAX_PARTS).
* @see ::solClient_bufInfo_t
*/
  typedef enum solClient_bufInfo_index
  {
    SOLCLIENT_BUFINFO_BINARY_ATTACHMENT_PART = 0, /**< The binary payload. */
    SOLCLIENT_BUFINFO_TOPIC_PART = 1,             /**< The Topic payload. */
    SOLCLIENT_BUFINFO_CONSUMER_ID_PART = 2,       /**< An array of consumer IDs ( not valid for send). */
    SOLCLIENT_BUFINFO_USER_DATA_PART = 3,         /**< The user-data payload. */
    SOLCLIENT_BUFINFO_XML_PART = 4,               /**< The XML payload.*/
    SOLCLIENT_BUFINFO_CORRELATION_TAG_PART = 5,   /**< For application using Guaranteed messages, this field is returned with acknowledgment. */
    SOLCLIENT_BUFINFO_QUEUENAME_PART = 6,         /**< A Queue to publish to. */
    SOLCLIENT_BUFINFO_USER_PROPERTY_PART = 7,     /**< Structured Binary 'meta' data to carry application specific data.*/
    SOLCLIENT_BUFINFO_MAX_PARTS = 8               /**< The maximum number of defined parts (maximum size of array). */
  } solClient_bufInfo_index_t;                    /**< Type for index into message buffer information array. */

/*
 * undocumented interface preserved for backwards compatibility 
 */
  typedef struct solClient_session_V1createFuncInfo
  {
    solClient_session_createRxCallbackFuncInfo_t rxInfo;
    solClient_session_createEventCallbackFuncInfo_t eventInfo;
  } solClient_session_V1createFuncInfo_t;

/*
 * undocumented interface preserved for backwards compatibility 
 */
  typedef struct solClient_flow_V1createFuncInfo
  {
    solClient_flow_createRxCallbackFuncInfo_t rxInfo;
    solClient_flow_createEventCallbackFuncInfo_t eventInfo;
  } solClient_flow_V1createFuncInfo_t;

/**
* Sends a message on the specified Session. The message is composed of a number of optional
* components that are specified by the bufInfo_p. If the buffer info size passed in is smaller than the maximum
* number of entries defined, then only the specified entries are consulted. If the buffer info size
* passed in is larger than the maximum number of defined entries, then the extra entries are ignored.
* \n 
* solClient_session_send() returns SOLCLIENT_OK when the message has been successfully 
* copied to the transmit buffer or underlying transport, this does not guarantee successful
* delivery to the Solace messaging appliance. When sending Guaranteed messages (persistent or non-persistent),
* the application will receive a subsequent ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT event for all
* messages successfully delivered to the Solace messaging appliance. For Guaranteed messages, notifications of
* quota, permission, or other delivery problems are indicated in a ::SOLCLIENT_SESSION_EVENT_REJECTED_MSG_ERROR
* event.
*
*
* @param opaqueSession_p The opaque Session returned when the Session was created.
* @param bufInfo_p       A pointer to an array of solClient_bufInfo_t entries, each of which specifies a
*                        possible message part.
* @param bufInfoSize     The number of entries in the supplied bufInfo array (this is the number of
*                        entries, not the number of bytes).
* @param flags           Various optional send flags, OR'ed together. Currently the only flags in use
*                        are for message Class of Service (COS). When COS is not in use, supply zero
*                        for flags. Other flag are for future compatibility and must be set to zero. 
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to send a Direct (that, is non-Guaranteed) message on a non-established
* Session. Guaranteed messages are queued for later transmission on non-established Sessions.
* @li ::SOLCLIENT_SUBCODE_TOPIC_TOO_LARGE
* @li ::SOLCLIENT_SUBCODE_TOPIC_MISSING
* @li ::SOLCLIENT_SUBCODE_USER_DATA_TOO_LARGE 
* @li ::SOLCLIENT_SUBCODE_QUEUENAME_TOO_LARGE
* @li ::SOLCLIENT_SUBCODE_QUEUENAME_INVALID_MODE
* @li ::SOLCLIENT_SUBCODE_QUEUENAME_TOPIC_CONFLICT
* @li ::SOLCLIENT_SUBCODE_MAX_TOTAL_MSGSIZE_EXCEEDED
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - An attempt was made to send a Guaranteed message to a Session that does not support Guaranteed Messaging.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - The operation timed-out trying to write message to socket, 
*     waiting for Session to be established (only for blocking sends; refer to ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING), 
*     or waiting for an open publisher window (only when using ::SOLCLIENT_SEND_FLAGS_PERSISTENT or 
*     ::SOLCLIENT_SEND_FLAGS_NONPERSISTENT with blocking send operation).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_send (solClient_opaqueSession_pt opaqueSession_p,
                            solClient_bufInfo_ap bufInfo_p,
                            solClient_uint32_t bufInfoSize,
                            solClient_sendFlags_t flags);

/**
* @struct solClient_sendMultiple
*
* A structure used to describe a single message. This is used as part of the solClient_session_sendMultiple()
* call. Note that this only allows a Topic message to be sent with a binary payload. Use
* multiple calls to solClient_session_send() to send other message types.
*
*/
  typedef struct solClient_sendMultiple
  {
    solClient_sendFlags_t flags;             /**< Various optional \ref sendflags "send flags", OR'ed together. Currently the only flags in use are for message class of service. Other flag fields must be set to zero. */
    solClient_bufInfo_t topic;               /**< The Topic on which to send the message. */
    solClient_bufInfo_t binaryAttachment;    /**< The binary attachment (payload) for the message. */
  } solClient_sendMultiple_t, *solClient_sendMultiple_pt; /**< A pointer to the ::solClient_sendMultiple structure that is used to describe a single message sent as part of solClient_session_sendMultiple(). */

/**
* Sends multiple messages on the specified Session. For direct messages, using this function is more efficient than multiple calls to
* solClient_session_send(). This routine is suitable when an application is able to construct multiple
* messages at once from a single stimulus. This routine limits the operation to sending Direct 
* messages with a binary payload
* on a topic. For other message types, use multiple calls to solclient_session_send(). Note that the number of messages
* which can be sent through a single call to solClient_session_sendMultiple() is limited to
* ::SOLCLIENT_SESSION_SEND_MULTIPLE_LIMIT.
* For Sessions in which solClient_session_sendMultiple() is used, it is recommended that the Session
* property ::SOLCLIENT_SESSION_PROP_TCP_NODELAY be enabled (it is enabled by default).
* With solClient_session_sendMultiple() multiple messages are sent at once onto a TCP connection, and therefore
* there is no need to have the operating system carry out the TCP delay algorithm to cause fuller IP packets.
* \n 
* solClient_session_sendMultiple() returns SOLCLIENT_OK when the messages have been successfully 
* copied to the transmit buffer or underlying transport, this does not guarantee successful
* delivery to the Solace messaging appliance. 
*
*
* @param opaqueSession_p        The opaque Session returned when the Session was created.
* @param msgArray_p             A pointer to an array of solClient_sendMultiple_t entries. Each entry describes one message to be sent.
* @param numberOfMessages       The number of messages provided in the message array.
* @param sendMultipleStructSize The size (in bytes) of each member of the message descriptor array to allow the structure to grow in the future.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TOPIC_TOO_LARGE
* @li ::SOLCLIENT_SUBCODE_TOPIC_MISSING
* @li ::SOLCLIENT_SUBCODE_MAX_TOTAL_MSGSIZE_EXCEEDED
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write message to socket or
*     waiting for Session to be established (only for blocking sends; refer to ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_sendMultiple (solClient_opaqueSession_pt opaqueSession_p,
                                    solClient_sendMultiple_pt msgArray_p,
                                    solClient_uint32_t numberOfMessages,
                                    size_t sendMultipleStructSize);

typedef solClient_uint32_t solClient_consumerId_t;           /**< A consumer ID that can optionally be applied to subscriptions and received on messages. */

/**  
* @anchor nullConsumerId
* @name Null Consumer ID
* This consumer ID value is reserved to indicate a non-present consumer ID.
* This value is never received with a message. When provided with a subscription, it
* indicates that the subscription has no associated consumer ID value.
* Consumer IDs are only used with XML content subscriptions and not with Topic subscriptions.
* When XML subscriptions are applied with consumer IDs, received messages contain a list of
* the consumer IDs of all XML subscriptions that were a match to that received message.
* This allows the application to know which of its XML subscriptions matched 
* the received message.
*/
/*@{*/
static const solClient_consumerId_t SOLCLIENT_NULL_CONSUMER_ID = 0xffffffff; /**< Indicates a \ref nullConsumerId "non-present consumer ID". */
/*@}*/
#define SOLCLIENT_BUFINFO_MAX_CONSUMER_ID_SIZE (65536)     /**< The maximum size allowed for the consumerId portion. */

/**
* Used to determine how many consumer IDs are present in a received message.
* If the message does not have any consumer IDs, a count of zero is returned for the consumer
* ID count. This is not considered an error.
* @param bufInfo_p          The buffer information that was given to the application in the receive callback.
* @param consumerIdCount_p  A pointer to an integer to receive the count of consumer IDs.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/

  solClient_dllExport solClient_returnCode_t
    solClient_bufInfo_getConsumerIdCount (solClient_bufInfo_ap bufInfo_p,
                                          solClient_uint32_t *
                                          consumerIdCount_p);

/**
* Returns the requested consumer ID based on the index 
* (an integer  0 <= index < solClient_bufInfo_getConsumerIdCount()) , 
* where 0 represents the first available consumer ID, 1 represents the next,
* and so on. If a consumer ID index is requested that does
* not exist for the buffer information passed in, the ::SOLCLIENT_NULL_CONSUMER_ID is returned for the
* consumer ID, and this is not considered an error. Thus, an application can iterate over the set of
* consumer IDs until ::SOLCLIENT_NULL_CONSUMER_ID is returned, without having to estimate in advance how many
* consumer IDs are present.
* @param bufInfo_p       The buffer information that was given to the application in the receive callback.
* @param consumerIdIndex The index of the consumer ID to return (starting at zero).
* @param consumerId_p    A pointer to storage to receive the next consumer ID (or ::SOLCLIENT_NULL_CONSUMER_ID).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_bufInfo_getConsumerId (solClient_bufInfo_ap bufInfo_p,
                                     solClient_uint32_t consumerIdIndex,
                                     solClient_consumerId_t * consumerId_p);


/**
* Adds an XPath expression (XPE) subscription to a Session.
* Namespaces are supported for XPE subscriptions.
* An example of an XPE subscription is: 
*   <i> /invoice/total[text()>123.45] </i>
*
* When namespaces are used, the namespaces parameter is an array of namespace prefix/URI pairs,
* terminated by a NULL pointer. For example:
* @li    namespaces[0] = "foo";
* @li    namespaces[1] = "www.foo.com";
* @li    namespaces[2] = "bar";
* @li    namespaces[3] = "www.bar.com";
* @li    namespaces[4] = NULL;
*
* The subscription should use the namespace prefixes, such as <i> "/foo:invoice/bar:total[text()>123.45]" </i>.
* Subscription strings and namespace strings may not contain the ' character.
*
* @param opaqueSession_p The opaque Session that was returned when Session was created.
* @param xmlSubscription_p  The subscription string (must be NULL-terminated).
* @param namespaces An array of namespace prefix and namespace Uniform Resource Identifiers (URIs) , terminated by NULL pointer.
*                   If namespaces are not used, pass in NULL for this parameter.
* @param flags Controls the behavior of this function. Valid \ref subscribeflags "flags" are:
* @li                  ::SOLCLIENT_SUBSCRIBE_FLAGS_ISFILTER - Reset for a normal subscription and set for a filter.
* @li                  ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM - Set to indicate function should not return until
*                        confirmation is received from the appliance.
* @li                  ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM - When set the function returns immediately and 
*                        a confirmation event is received later when the appliance has completed the subscription add
*                        request.
* @param consumerId The consumer ID associated with the Topic (can be used for de-multiplexing
*                   at the receiver). Specify ::SOLCLIENT_NULL_CONSUMER_ID if consumer IDs are not
*                   being used by the application.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write a subscribe request to the socket,
*     waiting for a Session to be established, or waiting for internal resources
*     (only for blocking subscription operations; refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING);
*     or timed-out waiting for a confirm (only when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
*
* The following subcodes can occur when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. Otherwise, these errors are reported
* when a ::SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR Session event is received.
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_INVALID
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ALREADY_PRESENT (see ::SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_TOO_MANY
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - The Session does not support XML subscriptions.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_xmlSubscribe (solClient_opaqueSession_pt opaqueSession_p,
                                    const char *xmlSubscription_p,
                                    char **namespaces,
                                    solClient_subscribeFlags_t flags,
                                    solClient_consumerId_t consumerId);

/**
* Removes an XPath expression (XPE) subscription from a Session.
* Namespaces are supported for XPE subscriptions.
* An example of a subscription is:
*   /invoice/total[text()>123.45]
* When namespaces are used, the namespaces parameter is an array of name space prefix/URI pairs,
* terminated by a NULL pointer. For example:
*    namespaces[0] = "foo";
*    namespaces[1] = "www.foo.com";
*    namespaces[2] = "bar";
*    namespaces[3] = "www.bar.com";
*    namespaces[4] = NULL;
* and a subcscription that uses the namespace prefixes, such as "/foo:invoice/bar:total[text()>123.45]"
* Subscription strings and namespace strings cannot contain the ' character.
*
* @param opaqueSession_p The opaque Session that was returned when Session was created.
* @param xmlSubscription_p  The subscription string (must be NULL-terminated).
* @param namespaces An array of namespace prefix and namespace URIs, terminated by NULL pointer.
*                   If namespaces are not used, pass in NULL for this parameter.
* @param flags Controls the behavior of this function. The valid \ref subscribeflags "flags" are:
*                  ::SOLCLIENT_SUBSCRIBE_FLAGS_ISFILTER - Reset for a normal subscription and set for a filter.
*                  ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM - Set to indicate function should not return until
*                  confirmation is received from the appliance.
* @param consumerId The consumer ID associated with the Topic (can be used for de-multiplexing
*                   at the receiver). Specify ::SOLCLIENT_NULL_CONSUMER_ID if consumer IDs are not
*                   being used by the application.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write a subscribe request to socket, 
*     waiting for a Session to be established, or waiting for internal resources
*     (only for blocking subscription operations; refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING);
*     or timed-out waiting for confirm (only when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
*
* The following subcodes can occur when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. Otherwise, such errors are reported
* when a ::SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR Session event is received.
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_INVALID
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_NOT_FOUND (see ::SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - The Session does not support XML subscriptions.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_xmlUnsubscribe (solClient_opaqueSession_pt
                                      opaqueSession_p,
                                      const char *xmlSubscription_p,
                                      char **namespaces,
                                      solClient_subscribeFlags_t flags,
                                      solClient_consumerId_t consumerId);

/**
* Allows the application to take control over the
* file descriptor required for message receive functions, while the API maintains internal control
* of all other file descriptors. This function can only be called once the Session is fully established.
* The function returns callback information that must be invoked whenever the returned
* file descriptor is readable. The callback function must be invoked with the returned file descriptor, an
* event of ::SOLCLIENT_FD_EVENT_READ, and the specified opaque user data pointer.
* NOTE: This function should only be used in particular circumstances. For more information,
* contact your Solace Corporation representative.
* \n 
*
* @param opaqueSession_p The opaque Session pointer that is returned when the Session was created.
* @param fd_p            A pointer to the variable to receive the returned file descriptor.
* @param callback_p      A pointer to the variable to receive the returned callback function pointer.
* @param user_p          A pointer to the variable to receive the returned opaque user data pointer.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL
* @subcodes
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_controlMessageReceiveFd (solClient_opaqueSession_pt
                                               opaqueSession_p,
                                               solClient_fd_t * fd_p,
                                               solClient_context_fdCallbackFunc_t
                                               * callback_p, void **user_p);

  /**
   * Old documentation for solClient_session_createFuncInfo
   *
   * If the application uses the <i>rxInfo</i> callback interface, then the
   * application callback will have the interface (see ::solClient_session_rxCallbackFunc_t):
   *
   * <code style="font-size:90%">
     void
     applicationRxDataCallback ( solClient_opaqueSession_pt                opaqueSession_p,
                                 solClient_bufInfo_pt                      bufInfo_p,
                                 solClient_session_rxCallbackInfo_pt       rxInfo_p,
                                 void                                     *user_p);
   * </code>
   *
   *
   *
   */
/**
* @struct solClient_session_rxCallbackInfo
*
* A structure that is returned (as a pointer) with each received message in
* the message receive callback for a Session. A structure is used
* so that new fields can be added in the future without affecting existing 
* applications.
*/
  typedef struct solClient_session_rxCallbackInfo
  {
    solClient_receiveFlags_t flags;
    solClient_msgId_t        msgId;
    solClient_uint64_t       rcvTimestamp;
  } solClient_session_rxCallbackInfo_t,  *solClient_session_rxCallbackInfo_pt; /**< Pointer to :: solClient_session_rxCallbackInfo structure of information returned with a message. */

/**
* A structure that is returned (as a pointer) with each received message in
* the message receive callback for a Flow. A structure is used
* so that new fields can be added in the future without affecting existing 
* applications. Note that this structure is the same as ::solClient_session_rxCallbackInfo.
*/
  typedef solClient_session_rxCallbackInfo_t solClient_flow_rxCallbackInfo_t, *solClient_flow_rxCallbackInfo_pt; /**< Pointer to :: solClient_session_rxCallbackInfo structure of information returned with a message. */

/**
* A callback prototype for received messages on a Flow. A callback with this prototype 
* can be registered for a Flow. This interface is provided for backwards 
* compatibility with previous versions of the C API. If an application provides 
* this callback, it is called for each received message.
* When this callback is invoked for a received message, the application must 
* finish its use of the received message (as expressed by the various message 
* parts described by the buffer information array). If the application 
* requires a message (or some of its parts) after this callback returns, then 
* the application must copy the required message parts. For example, if an 
* application wants to Queue a received message for processing by a different 
* application thread, then the message receive callback
* copies the required message parts into the queue.
* @param opaqueFlow_p A pointer to the Flow for which the message has been received. This pointer is never NULL.
* @param bufInfo_p An array of buffer information for the received message. This pointer is never NULL.
* There is one bufInfo_t element for each message part as defined by 
* ::solClient_bufInfo_index ::SOLCLIENT_BUFINFO_MAX_PARTS. By only passing back a pointer to 
* the first element of the array, an application can refer to as many, or as few,
* elements of the array as needed. This allows for future backwards compatibility.
* @param rxInfo_p A pointer to a structure containing more information about the received message (for example, flags). This pointer is never NULL.
* @param user_p A pointer to opaque user data provided when callback registered.
* @see solClient_session_create()
*/
  typedef void (*solClient_flow_rxCallbackFunc_t) (solClient_opaqueFlow_pt opaqueFlow_p, solClient_bufInfo_pt bufInfo_p, solClient_flow_rxCallbackInfo_pt rxInfo_p, void *user_p);

  /**
  * <b>DEPRECATED</b>.
  * A callback prototype for received messages. A callback with this prototype
  * can be registered for a Session. This interface is provided <b>only</b> for backwards
  * compatibility with previous versions of the C API. If an application provides
  * this callback, it is called for each received message.
  * When the callback is invoked for a received message, the application must
  * finish its use of the received message (as expressed by the various message
  * parts described by the buffer information array). If the application
  * requires a message (or some of its parts) after this callback returns, then
  * the application must copy the required message parts. For example, if an
  * application wishes to Queue a received message for processing by a different
  * application thread, then the message receive callback
  * copies the required message parts into the queue.
  * @param opaqueSession_p A pointer to Session for which the message has been received. This pointer is never NULL.
  * @param bufInfo_p An array of buffer information for the received message. This pointer is never NULL.
  * There is one bufInfo_t element for each message part, as defined by
  * ::solClient_bufInfo_index ::SOLCLIENT_BUFINFO_MAX_PARTS. By only passing back a pointer to
  * the first element of the array, an application can refer to as many, or as few,
  * elements of the array as needed. This allows for future backwards compatibility.
  * @param rxInfo_p A pointer to a structure containing more information about the received message (for example, flags). This pointer is never NULL.
  * @param user_p A pointer to opaque user data provided when the callback is registered.
  * @see solClient_session_create()
  */
    typedef void (*solClient_session_rxCallbackFunc_t) (solClient_opaqueSession_pt opaqueSession_p, solClient_bufInfo_pt bufInfo_p, solClient_session_rxCallbackInfo_pt rxInfo_p, void *user_p);

    /**
    * @anchor sendflags
    * @name Send Flag Types
    * Values that can be used to modify message characteristics in
    * solClient_msg_setDeliveryMode() and solClient_msg_setClassOfService()
    */
    /*@{*/
    #define SOLCLIENT_SEND_FLAGS_COS_1          (0x00)  /**< The lowest class of service value. For guaranteed messaging, it means the low priority.*/
    #define SOLCLIENT_SEND_FLAGS_COS_2          (0x01)  /**< The middle class of service value. For guaranteed messaging, it is reserved for future use and is treated as the low priority.*/
    #define SOLCLIENT_SEND_FLAGS_COS_3          (0x02)  /**< The highest class of service value. For guaranteed messaging, it means the high priority.*/
    #define SOLCLIENT_SEND_FLAGS_COS_MASK       (0x03)  /**< A mask for class of service value. */
    #define SOLCLIENT_SEND_FLAGS_DIRECT         (0x00)  /**< Send a Direct message. */
    #define SOLCLIENT_SEND_FLAGS_PERSISTENT     (0x10)  /**< Send a Persistent message. */
    #define SOLCLIENT_SEND_FLAGS_NONPERSISTENT  (0x20)  /**< Send a Non-Persistent message. */

    #define SOLCLIENT_SEND_FLAGS_ASSURED        (SOLCLIENT_SEND_FLAGS_PERSISTENT|SOLCLIENT_SEND_FLAGS_NONPERSISTENT)
    #define SOLCLIENT_SEND_FLAGS_DELIVER_TO_ONE (0x40)  /**< This message property instructs the appliance to deliver the message to only one subscriber, even if multiple clients have subscriptions that match the topic. */
    #define SOLCLIENT_SEND_FLAGS_TMP_DESTINATION (0x80) /**< This message property indicates to the API that the destination carried in the ::SOLCLIENT_BUFINFO_QUEUENAME_PART or ::SOLCLIENT_BUFINFO_TOPIC_PART is a non-durable destination. */
    #define SOLCLIENT_SEND_FLAGS_DMQ_ELIGIBLE   (0x100) /**< Message is eligible for Dead Message Queue (DMQ). When set, messages that expire in the network, are saved on a DMQ on the appliance. Otherwise expired messages are discarded. */
    #define SOLCLIENT_SEND_FLAGS_ELIDING_ELIGIBLE (0x200) /**< Message is eligible for eliding. When set, messages are delayed on egress and only the latest message is delivered to clients that request eliding */
    #define SOLCLIENT_SEND_FLAGS_ACK_IMMEDIATELY  (0x400) /**< This message property indicates that the appliance should ACK the message immediately upon receipt. The property is only for Assured Delivery messages and is meaningless on messages received from appliances. */
    #define SOLCLIENT_SEND_FLAGS_VALID_MASK     (0x3FF)  /**< A mask to detect valid flags. */
    /*@}*/

    /**
    * @anchor receiveflags
    * @name Receive Callback Flags
    * Flags associated with received messages.
    * Represents bits that can be set in a bit vector.
    * These values are masks that can be ANDed with the flag's
    * value for a received message to see if the condition is set.
    */
    /*@{*/
    #define SOLCLIENT_RX_FLAGS_DISCARD_INDICATOR_MASK      (0x01) /**< Mask for discard indicator flag of a received message. */
    #define SOLCLIENT_RX_FLAGS_AD_REDELIVERED_MASK         (0x02) /**< Mask for Guaranteed Delivery Redelivered Flag in a received message.*/
    #define SOLCLIENT_RX_FLAGS_DELIVERY_MODE_PERSISTENT    (0x04) /**< Indicates the received message was a Persistent message. */
    #define SOLCLIENT_RX_FLAGS_DELIVERY_MODE_NONPERSISTENT (0x08) /**< Indicates the received message was a Non-Persistent message. */
    #define SOLCLIENT_RX_FLAGS_DELIVERY_MODE_DIRECT        (0x00) /**< Indicates the received message was a Direct message. */
    #define SOLCLIENT_RX_FLAGS_COS_MASK                    (0x30) /**< Mask for Class of Service (COS) value in received message. */
    #define SOLCLIENT_RX_FLAGS_COS_SHIFT                   (4)    /**< The number of bits to shift right after ::SOLCLIENT_RX_FLAGS_COS_MASK applied to get a Class of Service (COS) value in the form ::SOLCLIENT_SEND_FLAGS_COS_1, ::SOLCLIENT_SEND_FLAGS_COS_2 or ::SOLCLIENT_SEND_FLAGS_COS_3. */
    /*@}*/
	
	/* Deprecated constant used with solClient_session_getCapability() */
	#define SOLCLIENT_SESSION_CAPABILITY_SUPPORTS_XPE_SUBSCRIPTIONS     "SESSION_CAPABILITY_SUPPORTS_XPE_SUBSCRIPTIONS"    /**<  Boolean - The Session supports Xpath expression (XPE) subscriptions and routing based on subscription matches in XML content. */
	
	/* Deprecated constant used with solClient_subscribeFlags_t */
	#define SOLCLIENT_SUBSCRIBE_FLAGS_ISFILTER              (0x01) /**< The XML subscription is a filter. It is ignored for a Topic subscription. */

    /**
    * Used to determine how many consumer IDs are present in a received message.
    * If the message does not have any consumer IDs, zero is
    * returned for the consumer ID count. This is not considered an error.
    * @param msg_p    solClient_opaqueMsg_pt that is returned from a previous call
    *                       to solClient_msg_alloc() or received data callback.
    * @param consumerIdCount_p  A pointer to an integer to receive the count
    *                           of consumer IDs.
    * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
    * @subcodes
    * @see ::solClient_subCode for a description of all subcodes.
    */
    solClient_dllExport solClient_returnCode_t
    solClient_msg_getConsumerIdCount (solClient_opaqueMsg_pt  msg_p,
                                      solClient_uint32_t *consumerIdCount_p);

    /**
    * Returns the requested consumer ID based on the index
    * (an integer  0 <= index < solClient_msg_getConsumerIdCount()),
    * where 0 represents the first available consumer ID, 1 represents the next,
    * and so on. If a consumer ID index is requested that does
    * not exist for the buffer information passed in, the ::SOLCLIENT_NULL_CONSUMER_ID is
    * returned for the consumer ID, and this is not considered an error. Thus, an
    * application can iterate over the set of consumer IDs until ::SOLCLIENT_NULL_CONSUMER_ID
    * is returned, without having to estimate in advance how many consumer IDs are present.
    * @param msg_p    solClient_opaqueMsg_pt returned from a previous call
    *                       to solClient_msg_alloc() or received data callback.
    * @param consumerIdIndex The index of the consumer ID to return (starting at zero).
    * @param consumerId_p    A pointer to storage to receive the next consumer ID (or ::SOLCLIENT_NULL_CONSUMER_ID).
    * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
    * @subcodes
    * @see ::solClient_subCode for a description of all subcodes.
    */
    solClient_dllExport solClient_returnCode_t
    solClient_msg_getConsumerId (solClient_opaqueMsg_pt msg_p,
                                 solClient_uint32_t consumerIdIndex,
                                 solClient_consumerId_t * consumerId_p);


    /**
     * Extract a data block reference from a bufInfo part. If the bufInfo part
     * does not reference a data block, then ::SOLCLIENT_NOT_FOUND is returned, and the
     * data block pointer will be NULL. If this function returns success, the
     * message pointer bufInfo element will no longer reference the data block and
     * will no longer have that message part.
     *
     * @param msg_p         A pointer to a message block.
     * @param bufIndex      Index of bufInfo element to extract data block from.
     * @param datab_p       A pointer to a data block reference returns data block pointer.
     * @param bufInfo_p     A pointer to a bufInfo to return buffer information (ptr, size).
     *
     * @returns             ::SOLCLIENT_OK on success. ::SOLCLIENT_NOT_FOUND when there is
     *                      no data block to extract. ::SOLCLIENT_FAIL when there
     *                      is a parameter error.
     * @subcodes
     * @see ::solClient_subCode for a description of all subcodes.
     */
    solClient_dllExport solClient_returnCode_t
    solClient_msg_extractDatablock(solClient_opaqueMsg_pt msg_p,
                                   solClient_bufInfo_index_t bufIndex,
                                   solClient_opaqueDatablock_pt *datab_p,
                                   solClient_bufInfo_pt bufInfo_p);


    /**
     * Free a data block that has been previously acquired through a call to
     * ::solClient_msg_extractDatablock(). Decrement the reference count in a data block,
     * and return it to the appropriate free list if it goes to zero.
     *
     * @param datab_p       A pointer to datab_p previously returned from
     *                      ::solClient_msg_extractDatablock(). On return this
     *                      reference is NULL, and the memory previously referenced by
     *                      it is no longer valid.
     *
     * @returns             SOLCLIENT_OK on success. SOLCLIENT_FAIL when the
     *                      data block pointer is NULL.
     * @subcodes
     * @see ::solClient_subCode for a description of all subcodes.
     */
    solClient_dllExport solClient_returnCode_t
    solClient_datablock_free(solClient_opaqueDatablock_pt *datab_p);

  /**
   * Creates a Queue network name.  This function has been deprecated and exists for backwards
   * compatiblity.
   *
   * This string may be passed as the Queue name to
   * ::solClient_session_createFlow() when connecting with a Queue
   * endpoint. It may also be used as the destination name when publishing to a
   * queue, and used in a ::solClient_destination_t that is sent
   * to a peer application in the ReplyTo field of the Solace Header map or in a
   * structured data field.
   *
   * <b>WARNING:</b> This function must not be used with Solace messaging appliances running
   * SolOS-CR Versions lower than 5.0. Solace messaging appliances running SolOS-CR prior to Version 5.0 do not recognize the Queue network name, even for local queues.
   *
   * @param queueName_p     The local Queue name. This cannot be an empty string. To generate a
   *                        unique local Queue name, use solClient_generateUUIDString() or
   *                        solClient_appendUUIDString().
   * @param virtualName_p   The network virtual router name of the appliance configured with the queue. You must get
   *                        this string by querying the property ::SOLCLIENT_SESSION_PROP_VIRTUAL_ROUTER_NAME.
   * @param opaqueSession_p If virtualName_p is NULL, it is discovered from the opaqueSession_p, otherwise
   *                        opaqueSession_p should be NULL.
   * @param durability      A Boolean set to true to create a Queue network name for a durable queue.
   * @param queueNetName_p  A pointer to a string location where the string is returned.
   * @param length          The maximum string length to return.
   * @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
   * @subcodes
   * @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The generated Queue name would be longer than the
   * available length.
   * @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - The Session must be established before
   * temporary Queue names can be generated.
   * @see ::solClient_subCode for a description of all subcodes.
   */
  solClient_dllExport solClient_returnCode_t
  solClient_createQueueNetworkName (     char                      *queueName_p,
                                         char                      *virtualName_p,
                                         solClient_opaqueSession_pt opaqueSession_p,
                                         solClient_bool_t           durability,
                                         char                      *queueNetName_p,
                                         size_t                     length);
	
/**
 * Create a temporary Queue name. This function has been deprecated and exists for backwards
 * compatiblity.
 *
 * The string returned by this function may be passed as the Queue name to
 * ::solClient_session_createFlow() when connecting with a non-durable Queue
 * endpoint. It may also be used in a ::solClient_destination_t that is sent
 * to a peer application in the ReplyTo field of the Solace Header map or in a
 * structured data field.
 *
 * @param opaqueSession_p The opaque Session pointer that is returned when the Session was created.
 * @param queue_p         A pointer to a string location where the string is returned.
 * @param length          The maximum string length to return.
 * @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The generated Queue name would be longer than the
 * available length.
 * @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - The Session must be established before
 * temporary Queue names can be generated.
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_session_createTemporaryQueueName(solClient_opaqueSession_pt opaqueSession_p,
                                       char                      *queue_p,
                                       size_t                     length);

/* NEVER call directly */
  solClient_dllExport void
    _solClient_log_output (solClient_log_category_t category,
                           solClient_log_level_t level,
                           const char *format_p, ...);

/* NEVER call directly */
  solClient_dllExport void
    _solClient_log_output_va_list (solClient_log_category_t category,
                                   solClient_log_level_t level,
                                   const char *format_p, va_list ap);

#endif  /* SOL_CLIENTDEPRECATED_H_ */

#endif  /* DOXYGEN_SHOULD_SKIP_THIS */
