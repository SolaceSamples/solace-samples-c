/**
*
* @file solClient.h include file for the Solace Corporation Messaging API for C
*
* Copyright 2007-2018 Solace Corporation. All rights reserved.
*
* This include file provides the public constants and API calls for clients
* connecting to a Solace messaging appliance.
*
*/

#ifndef SOL_CLIENT_H_
#define SOL_CLIENT_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <wchar.h>
#endif

#if defined(__cplusplus)


extern "C"
{
#endif                          /* _cplusplus */

/**
     @mainpage

     @section introduction Introduction

     The Messaging API for C provides an Application Programming
     Interface (API) for developing C or C++ applications for use with a Solace messaging appliance.

     @section overview Overview

     The Messaging API for C (also referred to as SolClient) is specifically designed to 
	 provide high message throughput and low latency with the lowest CPU utilization
     possible. It is a fully functional API, and it contains the following
     main features:
     @li Allows the application to connect to a Solace messaging appliance through the concept of a "Session".
     @li Allows messages to be constructed and sent on a Session and to be received from a Session.
     @li Allows subscriptions to be added to specify what messages are to be received. Subscriptions are
     topic-based and require the use of a Solace messaging appliance with a Topic Routing Blade (TRB).
     @li Support for Guaranteed Message delivery (Queue-based or Topic Endpoint-based).
     Guaranteed messages are received on "Flows" (see solClient_session_createFlow()), which are 
	 constructed within a Session. Guaranteed Messaging is only available to clients when a Solace messaging appliance
	 is used that has an Assured Delivery Blade (ADB) installed and Guaranteed Messaging and message 
	 spooling enabled.
     @li Allows fine-tuning of API behavior, such as whether operations should be blocking or non-blocking in nature.
     @li Allows for the option of application file descriptors to be monitored within the API, providing
	 the application with callbacks for readable and writable events.
     @li Allows for the option of the application taking over control of file descriptors created within the API to
     connect to the Solace messaging appliance, where the application must provide readable and writable events to the API.
     @li Support for timer services.
     @li Support for logging, including support for logging filter levels, and the ability for the application to
     be called back for generated logs so that they can be placed into the application's logging system.

     @section specifics Messaging API for C Specifics

     The Messaging API for C behavior is unique among Solace Corporation Messaging APIs in 
	 the following ways:

     @subsection threading-support Threading Support

     By default, the Messaging API for C does not provide any internal threads. Instead, the 
	 API allows the application to provide thread processing time, providing more 
	 flexibility to application designers on how to allocate messaging system resources. 
	 However, SolClient can optionally provide a Context thread for processing work that 
	 is suitable for the most common application models and architectures.

     @subsection blocking-non-blocking Blocking and Non-Blocking Modes

     These modes are handled differently for the Messaging API for C than with the Messaging API for
     Java. For the Messaging API for C, blocking mode means that the calling thread for each send()
     function call is blocked until the API can accept the message. As a result, the application
     automatically controls the flow of send() calls to a rate at which the appliance can accept them. The send()
     function call remains blocked until either it is accepted by the API, or the timer (specified by
     ::SOLCLIENT_SESSION_PROP_BLOCKING_WRITE_TIMEOUT_MS) expires.

     In non-blocking mode, send() function calls that cannot be accepted by the API immediately return a
     ::solClient_returnCode ::SOLCLIENT_WOULD_BLOCK error code to the application. 
     If the application receives a ::SOLCLIENT_WOULD_BLOCK error, then 
     it also receive a subsequent ::SOLCLIENT_SESSION_EVENT_CAN_SEND event.
     The application can then retry the send()
     function call. In the interim, it can continue to process other actions.

     @subsection blocking-context Threading Effects on Blocking Modes

     The thread that is responsible for calling solClient_context_processEvents() is commonly referred to as
     the Context thread. Whether this thread is provided by SolClient, or a separate application thread, there are
     blocking implications that the application developer must be aware of. All callbacks from the API to the application,
     message receive callbacks, event callback, and timer callbacks, run from the Context thread. Additionally the Context
     thread must run to detect relief from flow control and unblock waiting application threads. The Context thread 
     must run to complete the Session connection sequence and unblock applications waiting for connection complete. The
     Context thread must run to unblock applications waiting for confirmation on subscription requests.

     Consequently, applications must not block in callback routines. Waiting in callback routines can deadlock the application 
     or at a minimum severely degrade receive performance. Deferring SolClient processing by running for excessively long 
     periods of time in the callback routines will prevent SolClient from unblocking other application threads that might be 
     waiting for confirmation of sent messages or be blocked in flow control situations. 
     
     SolClient detects when an application re-enters the API from a callback and will never block. Consequently, applications
     can expect to see ::SOLCLIENT_FAIL return codes when making SolClient function calls from a callback routine,
     if the function call would be blocking in other threads. Generally, if a function could block because of TCP flow control, 
     ::SOLCLIENT_FAIL is returned when that function is called from a callback, if the Session property 
     ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING is enabled.  This will only occur if the session finds it is flow controlled at
     the socket (TCP).  If it is possible to send the message, the function call will succeed as usual.  If the Session property 
     ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING is disabled, the API will succeed or return SOLCLIENT_WOULD_BLOCK as expected.

     
     Further, if the application explicitly sets ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM
     on a subscribe or unsubscribe API call made from a callback routine, that function call will return ::SOLCLIENT_FAIL 
     and set the subcode to ::SOLCLIENT_SUBCODE_CANNOT_BLOCK_IN_CONTEXT. Similarly, a function call on modifyClientInfo API made from a callback 
     will return ::SOLCLIENT_FAIL (with the subcode ::SOLCLIENT_SUBCODE_CANNOT_BLOCK_IN_CONTEXT) when the flag 
     ::SOLCLIENT_MODIFYPROP_FLAGS_WAITFORCONFIRM is set explicitly by the application.

     @subsection message-buffer Message Buffer Size Configuration

     The Session buffer size configured using the ::SOLCLIENT_SESSION_PROP_BUFFER_SIZE
     Session property controls SolClient buffering of transmit messages. When sending 
     small messages, to improve the performance, the Session buffer size should be set 
	 to multiple times the typical message size. Regardless of
     the buffer size, SolClient always accepts at least one message to transmit. 
     So even if a single message exceeds ::SOLCLIENT_SESSION_PROP_BUFFER_SIZE, it 
     is accepted and transmitted as long as the current buffered data is
     zero. However no more messages are accepted until the amount
     of data buffered is reduced enough to allow room below 
     ::SOLCLIENT_SESSION_PROP_BUFFER_SIZE.

     SolClient will internally buffer up to ::SOLCLIENT_SESSION_PROP_BUFFER_SIZE 
     bytes on transmit. This buffering is dynamic and does not consume memory 
     when not in use.
     
     @section topic-syntax Supported Topic Syntax
     Topics are NULL-terminated UTF-8 strings in the form "level1/level2/level3".
     The total length of a Topic can be 250 bytes, not including the NULL terminator.
     
     @section subscription-syntax Supported Subscription Syntax
     Subscriptions are in the form "level1/level2/level3". \n
     The total length of a subscription can be 250 bytes. \n
     The '>' character, when alone at the final level of a subscription,
     means match one or more levels at the end of a received topic. For example, "level1/level2/level3/>"
     matches "level1/level2/level3/level4" and "level1/level2/level3/level4/level5", but not
     "level1/level2/level3". \n
     '>' elsewhere in the subscription has no special meaning. For example, "level1>/level2".
     
     @subsection trb-subscription Supported Subscription Syntax with a Topic Routing Blade (TRB) in the Appliance
     The '*' character is a wildcard. When alone at a level, it means match any string at that level
     in a received topic. For example, "level1/ * /level3" matches "/level1/level2/level3". \n
     '*' can also be used to match a level that starts with a specified string. \n
     For example "level1/lev* /level3" matches "/level1/level2/level3". \n
     A '*'cannot appear at the beginning or within a string. For example, "lev*1" and "*evel" are not valid.
     
     @section topic-dispatch Dispatching Messages Based on a Topic
     By default, when subscriptions are added, all messages are dispatched to the Session message
     receive callback that is defined when the Session is created (through the ::solClient_session_createFuncInfo_t 
     parameter of ::solClient_session_create). However, a Session can also be configured through the Session
     property ::SOLCLIENT_SESSION_PROP_TOPIC_DISPATCH to dispatch messages with specific 
     Topic destinations to a message receive callback that is specified when a Topic subscription is added
     ( ::solClient_session_topicSubscribeWithDispatch()). Dispatching/demultiplexing incoming messages 
     based on the Topic associated with the message allows different callbacks to be 
     invoked based on the received topic. It also allows different application data to be provided to 
     the callback based on the matching subscription. Full wildcard subscription syntax is supported
     (refer to @ref subscription-syntax).
     
     As an example, consider the following Topic dispatch descriptions: \n
         ::solClient_session_rxMsgDispatchFuncInfo_t dispatch1 = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallback1Func, &item1Data, NULL };\n
         ::solClient_session_rxMsgDispatchFuncInfo_t dispatch2 = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallback1Func, &item2Data, NULL };\n
         ::solClient_session_topicSubscribeWithDispatch(
             session_p, 0, "level1/item1", &dispatch1, NULL ); \n
         ::solClient_session_topicSubscribeWithDispatch(
             session_p, 0, "level1/item2", &dispatch2, NULL); \n
     
     When a message is received with Topic "level1/item1", msgCallback1Func is called with
     a pointer to the application's item1Data structure.
     When a message is received with Topic "level1/item2", msgCallback1Func is called with
     a pointer to the application's item2Data structure.
     A Topic dispatch subscription is uniquely identified by the tuple <subscription, callback_p, 
     user_p, NULL (reserved for future use)>, any number of message callback functions can be used.
     
     When Topic dispatching is enabled, the Session callback is still used for subscriptions that
     are created through ::solClient_session_topicSubscribe() or ::solClient_session_topicSubscribeExt, but
     only if no other callback has been invoked for a received message. So the Session callback acts as a "default" callback. For example, consider what occurs when the following subscription is added to the preceding example: \n
         ::solClient_session_topicSubscribeExt(
             session_p, 0, "level1/>");
     
     The application subscribes to all topics under level1, but "level1/item1" and "level1/item2" go
     to the msgCallback1Func callback with the specified user_p, while any other Topic starting with "level1/" 
     go to the default Session message callback. This allows an application to trap certain topics 
     to a specified callback and send others to the Session callback.
     
     If a message matches subscriptions that use the Topic dispatch Session callback, then each matching
     subscription invokes a callback, and the application can receive and process a message 
     multiple times. For example: \n
         ::solClient_session_rxMsgDispatchFuncInfo_t dispatchA = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallbackAFunc, &userDataA, NULL };\n
         ::solClient_session_rxMsgDispatchFuncInfo_t dispatchB = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallbackBFunc, &userDataB, NULL };\n
         ::solClient_session_rxMsgDispatchFuncInfo_t dispatchC = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallbackCFunc, &userDataC, NULL };\n
         ::solClient_session_topicSubscribeWithDispatch(
             session_p, 0, "part1/item1", &dispatchA, NULL); \n
         ::solClient_session_topicSubscribeWithDispatch(
             session_p, 0, "part1/item1", &dispatchB, NULL); \n
         ::solClient_session_topicSubscribeWithDispatch(
             session_p, 0, "part1/>", &dispatchC, NULL); \n
     
     In this example, when a message arrives with Topic "part1/item1", it is delivered to
     each of the msgCallbackAFunc, msgCallbackBFunc, and msgCallbackCFunc callback functions. When a message 
     arrives with the Topic "part1/item2", it is only delivered to the msgCallbackCFunc
     callback function. Note that when a message does not match a specific callback and is being delivered
     to the default Session message received callback, it is only delivered once to that callback, 
     independent of the number of subscriptions it matched.

     Each unique Topic subscription is also added to the Solace messaging appliance and removed when there are 
     no longer any dispatch functions associated with the subscription. Applications that want to have 
     fine-grained control over appliance resources may choose to add dispatch Session callback functions only without 
     adding a subscription to the appliance. When the @ref subscribeflags "subscription flag" 
     ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY is set, the subscription is not added to the appliance. For example:\n
         ::solClient_session_rxMsgDispatchFuncInfo_t dispatchA = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallbackAFunc, &userDataA, NULL };\n
         ::solClient_session_rxMsgDispatchFuncInfo_t dispatchB = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallbackBFunc, &userDataB, NULL };\n
         ::solClient_session_rxMsgDispatchFuncInfo_t dispatchC = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallbackCFunc, &userDataC, NULL };\n
         ::solClient_session_topicSubscribeWithDispatch(
             session_p, SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY, "part1/item1", &dispatchA, NULL); \n
         ::solClient_session_topicSubscribeWithDispatch(
             session_p, SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY, "part1/item2", &dispatchB, NULL); \n
         ::solClient_session_topicSubscribeWithDispatch( session_p, 0, "part1/>", &dispatchC, NULL); \n
    
    In this example, the application avoids adding the subscriptions for "part1/item1" and "part1/item2" to the appliance as
    these subscriptions overlap with the wildcard subscription "part1/>". All messages are forwarded to the application due
    to the last wildcard subscription, and properly dispatched by the more specific non-wildcard subscriptions.
     
     When unsubscribing, a specified callback can be removed through ::solClient_session_topicUnsubscribeWithDispatch().
     For example: \n
         ::solClient_session_rxMsgDispatchFuncInfo_t removedispatchA = {SOLCLIENT_DISPATCH_TYPE_CALLBACK,  msgCallbackAFunc, &userDataA, NULL };\n
         ::solClient_session_topicUnsubscribeWithDispatch( session_p, 0, "part1/item1", &removedispatchA, NULL);

     Note:  When unsubscribing the pointer to the dispatch information (&removedispatchA) does not have to exactly match the pointer used
     to create the Topic dispatch entry, but the contents must describe the entry to be removed.
     
     The interface ::solClient_session_topicUnsubscribe() or ::solClient_session_topicUnsubscribeExt()
     can only be used to remove subscriptions for the default Session callback. When calling
     ::solClient_session_topicUnsubscribeWithDispatch(), providing a NULL pointer for the callback and the 
     user pointer or providing a NULL pointer to the dispatch information is equivalent to calling solClient_session_topicUnsubscribeExt().

     <b>
     @subsection host-entry Configuring Host Entry for SOLCLIENT_SESSION_PROP_HOST property
     </b>
     
     The entry for the SOLCLIENT_SESSION_PROP_HOST property should provide a protocol, host, and port. The SOLCLIENT_SESSION_PROP_HOST 
     property may also include an optional Proxy Server configuration, separated from the message router configuration by a percent (\%) 
     sign.

    @li [Protocol:]Host[:Port][\%ProxyService] or [Protocol://]Host[:Port][\%ProxyService]

    Protocol is the protocol used for the transport channel. The valid values are: \n
     @li tcp - use a TCP channel for communications between the application and its peers. If no protocol is set, tcp is used as a default.\n
     @li tcps - use a SSL channel over TCP for communications between the application and its peers.\n
     @li http - use HTTP channels or a WebSocket channel over TCP for communications between the application and its peers. Web Messaging with compression is not supported. \n
     @li https - use HTTP channels or a WebSocket channel over SSL for communications between the application and its peers. Web Messaging with compression is not supported.
  
    Host is the IP address (or host name) to connect to for a connection. 

    Port is the port to connect to for a connection. A value is only required when using a port other than the automatically assigned 
    default port number. The default port for TCP is 55555 when compression is not in use, or 55003 when compression is in use. The default port 
    for SSL is 55443. The default port for HTTP or WebSocket over TCP is 80. The default port for HTTP or WebSocket over SSL is 443.

    ProxyService is a description of the non-transparent proxy.  If it is necessary to configure the proxy server that is used to connect to
    the message router, then the proxy server is configured in the ProxyService string. The ProxyService string format is specified as:

    @li [ProxyProtocol]://[username:password@]proxyHost[:proxyPort]

    ProxyProtocol is the protocol used to communication with the proxy server. The valid values are:\n
     @li socks5 - Connect to the server with the SOCKS Protocol Version 5, RFC 1928 (IETF Standards Track Document) 
     @li httpc -  Connect to the server with the HTTP Connect Protocol, RFC 2817 (IETF Standards Track Document)

    If authentication is required for the proxy server, the <b>username</b> and <b>password</b> may be optionally 
    specified before the proxy host.

    proxyHost is the IP address (or host name) of the proxy server. 

    proxyPort is the port to connect to for a connection. If the port number is not specified, the default for SOCKS5 is port 1080, and
    the default for Http-Connect is port 3128.

    The following examples show how to specify transport channel types. Unless it is otherwise specified, the default port 55555 is used. \n
     @li 192.168.160.28 - connect to IP address 192.168.160.28 and the default port 55555 over TCP. \n
     @li tcp:192.168.160.28 - connect to IP address 192.168.160.28 and the default port 55555 over TCP. \n
     @li tcps:192.168.160.28 - connect to IP address 192.168.160.28 and the default port 55443 over SSL over TCP.\n
     @li tcp:192.168.160.28:44444 - connect to IP address 192.168.160.28 and port 44444 over TCP. \n
     @li http&nbsp;://192.168.160.28 - connect to IP address 192.168.160.28 and the default port 80 over HTTP or WebSocket over TCP. \n
     @li https&nbsp;://192.168.160.28 - connect to IP address 192.168.160.28 and the default port 443 over HTTP or WebSocket over SSL over TCP.

    The following examples show how to connect to a message router through a proxy server.\n
     @li 192.168.160.28\%socks5://192.168.1.1 - connect to message router at 192.168.160.28 through a SOCKS5 proxy server at 192.168.1.1.\n
     @li 192.168.160.28\%httpc://192.168.1.1 - connect to message router at 192.168.160.28 through a HTTP-Connect proxy server at 192.168.1.1.\n
     @li tcps:solace.company.com\%socks5://User:PassWord\@proxy.company.com:13128 - connect to message router at solace.company.com using
         SSL over TCP through a SOCKS5 proxy server at proxy.company.com, port 13128. Authenticate with the proxy server using username <I>User</I>
         and password <I>PassWord</I>.
     @li http&nbsp;://192.168.160.27:44444\%httpc://proxy.company.com:11080 - connect to the message router at 192.168.160.28, port 44444, using HTTP.
         Connect through the proxy server at proxy.company.com, port 11080.

     The SOLCLIENT_SESSION_PROP_HOST property also supports multiple host entries separated by comma. 

     @subsection host-list Configuring Multiple Hosts for Redundancy and Failover

     You can provide up to sixteen potential hosts for a client application 
     to connect or reconnect to. Typically the listed appliances are in separate geographic 
     locations, and the use of a host list allows your client applications to fail over to the
     alternate connections should the first appliance be unavailable. The host
     list is configured in the ::SOLCLIENT_SESSION_PROP_HOST property as a comma-separated list 
     of hosts. Each host may optionally include a port number
     as well. For example, if there are two appliances at 192.168.160.128 and
     192.168.160.129, but the second is using the non-default port 50005 for the
     message bus, the ::SOLCLIENT_SESSION_PROP_HOST would be configured as: \n
        "192.168.160.128,192.168.160.129:50005".

    If a client application publishes Guaranteed messages  (send flags of 
    ::SOLCLIENT_DELIVERY_MODE_PERSISTENT or ::SOLCLIENT_DELIVERY_MODE_NONPERSISTENT) in a Session, and
    then a disconnect occurs, the API will automatically reconnect to other
    listed hosts. However, because another host will not know the state of the publisher flow
    to the original host, the API must reset publisher flow state.  Unacknowledged messages are 
    renumbered and resent by the API.  If the alternate router is configured as a replication site this
    may lead to duplicate messages in the system.  It is up to the application to resolve this duplication
    in what ever way is appropriate to the application.

    Applications may wish to configure the session so that auto-reconnect only occurs if no guaranteed 
    messages have been published. This is the legacy behaviour of the API. If this is desired then 
    set the session property ::SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION to the value 
    ::SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION_DISCONNECT.  This session property can also be
    set as environment variable which then allows legacy applications to run without modification or
    recompile.

    In the given example, with the ::SOLCLIENT_SESSION_PROP_HOST configured as: \n
        "192.168.160.128,192.168.160.129:50005"\n
    when solClient_session_connect() is called, SolClient will attempt to
     connect, first to 192.168.160.128. If that connection fails for any
     reason, it will attempt to connect to 192.168.160.129:50005. This process is
     repeated until all entries in the host list are attempted. After each entry
     has been attempted, and if all fail, the Session properties ::SOLCLIENT_SESSION_PROP_CONNECT_RETRIES
     and ::SOLCLIENT_SESSION_PROP_RECONNECT_RETRY_WAIT_MS determine the behavior of
     SolClient. If ::SOLCLIENT_SESSION_PROP_CONNECT_RETRIES is non-zero, SolClient
     waits for up to the number of milliseconds set for ::SOLCLIENT_SESSION_PROP_RECONNECT_RETRY_WAIT_MS
	 then starts a connection attempt again from the beginning of the list.

     If an established Session fails, to any host in the list, when 
     ::SOLCLIENT_SESSION_PROP_RECONNECT_RETRIES is non-zero, then SolClient automatically attempts
     to reconnect, starting at the beginning of the list.
     
     The Session connect timer, ::SOLCLIENT_SESSION_PROP_CONNECT_TIMEOUT_MS, runs separately for
     each connection attempt. So an application waiting for a connection established 
     (::SOLCLIENT_SESSION_EVENT_UP_NOTICE) or connection failure 
     (::SOLCLIENT_SESSION_EVENT_CONNECT_FAILED_ERROR) may have to wait up to 
     (<<i>number of hosts in list</i>> times ::SOLCLIENT_SESSION_PROP_CONNECT_TIMEOUT_MS) for the event.
     
     @subsection flow-topic-dispatch Dispatching Messages Based on a Topic for a Flow
     
     Topic dispatching is also available on Flows to Queues and Topic Endpoints.
     
     A Queue endpoint Flow may have many topics associated with it. Topics may be added to Queue 
     endpoint Flows with the Session function, solClient_session_endpointTopicSubscribe(), or with the
     Flow function, solClient_flow_topicSubscribeWithDispatch().  
     
     A Topic Endpoint Flow only has a single Topic which is defined when the Flow is created
     (see ::SOLCLIENT_FLOW_PROP_TOPIC). Topic dispatching for a Topic Endpoint Flow can be
     useful when the Flow's Topic contains wildcards because the Topic dispatching capability can
     then be used to separate out different topics covered by the Flow's wildcard topic.

     As with solClient_session_topicSubscribeWithDispatch(), the @ref subscribeflags "subscribe flag"
     ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY may be used on a 
     Queue endpoint Flow to add a dispatch callback entry only.  ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY is
     implied on a Topic Endpoint Flow as adding topics dynamically to a Topic Endpoint is not supported. 

     solClient_flow_topicSubscribeWithDispatch() can be used to add a Topic subscription only (no dispatch entry)  to the Queue 
     endpoint by setting the dispatch information to NULL.
     
     Similar to a Session, Topic dispatching on a Flow is controlled by the functions
     ::solClient_flow_topicSubscribeWithDispatch() and ::solClient_flow_topicUnsubscribeWithDispatch().
     Note that for a Topic Endpoint the ::solClient_flow_topicSubscribeWithDispatch() and ::solClient_flow_topicUnsubscribeWithDispatch() functions only control the dispatching of messages received on the Flow and do 
     not affect which messages are received on the Flow. The only property that controls the messages
     received on a Flow to a Topic Endpoint is ::SOLCLIENT_FLOW_PROP_TOPIC.  
     
     No check is made to ensure that any Topic subscribed to through ::solClient_flow_topicSubscribeWithDispatch()
     is actually received on the Topic Endpoint Flow. For example, if the Topic Endpoint Flow was created with Topic 'level1/>',
     if ::solClient_flow_topicSubscribeWithDispatch() is invoked with a Topic of 'level2/level3', that topic
     is accepted, but the callback for that Topic is never invoked as the Flow will not attract
     messages that match that topic.
     
     @section feature-limitations Feature Limitations

     The routing of messages according to their defined Topics can only be used with Solace messaging appliances running
     Version 4.3 or above, and the appliance must be equipped with a Topic Routing Blade (TRB).
     In addition, the application description property of a Session only takes effect
     in this configuration, but it can be set without any side-effects with earlier
     release appliances or appliances without a TRB.

     The quality of service parameter (part of the flags field of solClient_session_send())
     is only supported when client mode is enabled on a Session. Otherwise, the quality
     of service parameter is ignored.

     File descriptor limits in Linux and Solaris operating systems restrict the
     number of files that can be managed per process to 1024. Because of these
     limitations due to the use of select(2) to poll devices, the Messaging 
     API for C may not manage any single file descriptor which has a numerical 
     value that exceeds 1024.
     
     If the C API is used in an application that connects to a Solace messaging appliance 
     with a Topic Routing Blade (TRB), channel connect failures can occur 
	 if there are more than 1024 Sessions created per process. This limit is 
	 further reduced by any other files not managed by the C API that the 
	 application has open.

     Similarly, on Windows platforms, a single Context (see solClient_context_create()) 
	 cannot manage more than 21 Sessions with pubSub mode enabled (clientMode==0) or 64 
     Sessions in client mode (clientMode==1). Exceeding these limits can 
     cause calls to \link ::solClient_context_processEvents 
     solClient_context_processEvents()\endlink to fail. An application that 
     registers its own file descriptor handlers (by providing non-null 
     function pointers in ::solClient_context_createRegisterFdFuncInfo_t) is not
     limited in the Messaging API for C, but the application might have its own limitations.  

     In all systems, an application that provides its file descriptors to the 
	 C API to manage (by calling solClient_context_registerForFdEvents())
     further reduces the number of Sessions that can be handled in a 
     single Context (Windows) or process-wide (Linux, Solaris).
     
     @subsection certificate-validation OpenSSL Certificate Validation

     <strong>Note on certificate validation:</strong>

     When validating certificates, the messaging APIs for C and .NET use the
     following validation rules, after building the chain from the server
     certificate to a self-signed root certificate using certificates presented by
     the server and certificates in the trust store :

     - Verify the root certificate is trusted.
     - Verify depth of the chain is <= 3.

     Java and JMS messaging APIs use the same rules as C and .NET with the following
     exceptions:
     - Depth validation is not enforced in Java or JMS.
     - When the server presents an incomplete certificate chain, Java/JMS messaging
     APIs only require the signer of the incomplete certificate chain to be in the
     trust store, where this could be insufficient for C/.NET APIs.

     Therefore C and .NET messaging APIs certificate validation rules are more
     restrictive, hence if a certificate is accepted by C or .NET, it will
     definitely be accepted by Java and JMS.

     @section transacted-session Transacted Session
     Client applications can create one or more transacted sessions within a non-transacted session.
     The lifecycle of a transacted session is bounded by the lifecycle of its non-transacted parent session.
     In other words, when a session is destroyed/disposed, all transacted sessions created within this session
     will be destroyed/disposed too.
     A transacted session can have at most one publisher flow and zero or many subscriber flows.
     All messages sent/received by a transacted session must have a guaranteed delivery mode, 
     there is no support for direct transport messages.

     <strong>Message Dispatcher:</strong> 

     Asynchronous message delivery to client applications within a Transacted Session (i.e. message consumers
     created from Transacted Sessions) cannot happen over the context thread. Therefore, asynchronous message
     delivery within a Transacted Session will be done within an implicitly created Message Dispatcher. \n
     There are two types of Message Dispatchers, which client applications can choose when a Transacted Session is created.

     @li Context-bound: A Message Dispatcher lazily created by the Context. 
     A Context can have at most one context-bound Message Dispatcher.
     @li TransactedSession-bound: A Message Dispatcher lazily created by a Transacted Session. 
     
     The lifecycle of a message dispatcher is bounded to the lifecycle of the corresponding API object.
     There are advantages and disadvantages when using TransactedSession-bound message dispatchers. 

     @li The main advantage is improved responsiveness. It is also possible to improve application performance, 
     especially with multi-core processors.
     @li The main disadvantage is that programing and debugging is more complex as 
     applications must be more multi-thread aware. In environments where there is 
     contention for CPU resources, there is more overhead managing and switching between threads.
     
     <strong>Receiving Messages:</strong>
     
     The main differences between receiving messages from a transacted consumer Flow and from a non-transacted Flow are:
     
     @li For a transacted consumer Flows (created by a Transacted Session), function solClient_flow_sendAck() returns OK
     and no operation (NOP) is performed.
     @li If a Rx message callback is provided when a transacted consumer Flow is created, messages received on the flow
     are dispatched to the callbacks with the usual user_p just as on non-transacted Flows, but in the  context of an 
     internal Contex-bound or TransactedSession-bound Message Dispatcher thread.
     @li For transacted Flows created without Rx message callbacks, messages received on Flows are queued internally. 
     Applications must call function ::solClient_flow_receiveMsg directly to retrieve messages from those internal queues.
     
     <strong>Publishing Messages:</strong>
     
     To publish guaranteed messages on a Transacted Session, Transacted Session property 
     ::SOLCLIENT_TRANSACTEDSESSION_PROP_HAS_PUBLISHER must be enabled when the Transacted Session is created.
     
     The main differences between publishing guaranteed messages from a Transacted Session and from a Session are:
     
     @li For a Transacted Session, window Size has no effect.
     @li For a Transacted Session, messages are not retransmitted.
     @li For a Transacted Session, a successful commit acknowledges published messages.
     @li For a Transacted Session, ACKs and NACKs from the appliance are ignored.
     
     <strong>Multithreading:</strong>
     
     Transacted Sessions are not thread-safe and a single Transacted Session should not 
     be accessed by multiple application threads.  
     The legal use cases for a transacted session, which are not enforced by the API, are limited to:
     @li If messages are received by Rx message callbacks, invoke commit/rollback/sendMsg only from 
     receiving message callbacks (i.e. in the context of an internal Message Dispatcher thread). 
     @li If messages are received from an application thread by calling ::solClient_flow_receiveMsg() 
     directly, commit/rollback/sendMsg/receiveMsg functions must be called in the context of the 
     same application thread.
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/*
 * Applications must NOT build with SOLCLIENT_EXPORTS defined.
 * Applications linking against the static library should
 * define SOLCLIENT_STATIC_LIB.
*/
#ifdef WIN32
#    if defined(SOLCLIENT_EXPORTS)
#        include <winsock2.h>
#    elif !defined (_WINSOCKAPI_)
#        error "solClient.h requires winsock.h or winsock2.h to be included by the application before inclusion of solClient.h"
#    endif
#    if defined(SOLCLIENT_STATIC_LIB) || defined(SOLCLIENT_CLR_LIB)
#        define solClient_dllExport
#    elif defined(SOLCLIENT_EXPORTS)
#        define solClient_dllExport __declspec(dllexport)
#    else
#        define solClient_dllExport __declspec(dllimport)
#    endif
#else
#    define solClient_dllExport
#endif
#endif          /* DOXYGEN SHOULD SKIP THIS */

/** @name Basic Data Types
* Data type definitions for various types such as Boolean, unsigned 8-bit integer, 
* signed 8-bit integer, etc.
*/
/*@{*/
typedef unsigned char  solClient_uint8_t;          /**< 8-bit unsigned integer type. */
typedef signed char    solClient_int8_t;           /**< 8-bit signed integer type. */
typedef unsigned short solClient_uint16_t;         /**< 16-bit unsigned integer type. */
typedef short          solClient_int16_t;          /**< 16-bit signed integer type. */
typedef unsigned char  solClient_bool_t;           /**< Boolean type (non-zero is true, 0 is false) .*/
typedef wint_t         solClient_wchar_t;          /**< Wide character type (16-bit unicode character).*/

#if UINT_MAX == 0xffffffff
  typedef int solClient_int32_t;                   /**< 32-bit signed integer type. */
  typedef unsigned int solClient_uint32_t;         /**< 32-bit unsigned integer type. */
#elif ULONG_MAX == 0xffffffff
  typedef long solClient_int32_t;                  /**< 32-bit signed integer type. */
  typedef unsigned long solClient_uint32_t;        /**< 32-bit unsigned integer type. */
#else
#   error Problem with 32-bit types.
#endif

#if defined (SOLCLIENT_CONST_PROPERTIES)
typedef const char ** solClient_propertyArray_pt;  /**< pointer to an array of string pointers for properties */
#else
typedef char ** solClient_propertyArray_pt;        /**< pointer to an array of string pointers for properties */
#endif

#ifdef WIN32
  typedef __int64 solClient_int64_t;               /**< 64-bit signed integer type. */
  typedef unsigned __int64 solClient_uint64_t;     /**< 64-bit unsigned integer type. */
# else
  typedef long long solClient_int64_t;             /**< 64-bit signed integer type. */
  typedef unsigned long long solClient_uint64_t;   /**< 64-bit unsigned integer type. */
#endif
/*@}*/

/*@{*/
/** @name Type Definitions for Opaque Pointers
* Various "objects" that are created are referred to through opaque pointers, which are passed
* to functions that allow access to the item. For example, a ::solClient_opaqueContext_pt pointer
* is returned from solClient_context_create() when a Context is created, and it is passed into other
* APIs that operate on a Context.
 */
typedef void                        *solClient_opaqueContext_pt;   /**< An opaque pointer to a processing Context. */
typedef void                        *solClient_opaqueSession_pt;   /**< An opaque pointer to a Session. */
typedef void                        *solClient_opaqueFlow_pt;      /**< An opaque pointer to a Flow. */
typedef void                        *solClient_opaqueMsg_pt;       /**< An opaque pointer to a message. */
typedef void                        *solClient_opaqueContainer_pt; /**< An opaque pointer to a container (such as a map or stream). */
typedef void                        *solClient_opaqueDatablock_pt; /**< An opaque pointer to a data block. */
typedef void                        *solClient_opaqueTransactedSession_pt;      /**< An opaque pointer to a Transacted Session. */
/*@}*/
typedef void   *                    *solClient_opaquePointer_pt;   /**< An opaque pointer to a pointer */

/**
 * @enum solClient_destinationType
 *
 * Destination Types that can appear in the ReplyTo.
 */

typedef enum solClient_destinationType
{
    SOLCLIENT_NULL_DESTINATION          = -1,
    SOLCLIENT_TOPIC_DESTINATION         = 0,
    SOLCLIENT_QUEUE_DESTINATION         = 1,
    SOLCLIENT_TOPIC_TEMP_DESTINATION    = 2,
    SOLCLIENT_QUEUE_TEMP_DESTINATION    = 3
} solClient_destinationType_t;

/**
 * @struct solClient_destination
 *
 * A data structure to represent the message destination. A publisher can
 * send messages to topics or queues and solClient_destination specifies 
 * the details. 
 */
typedef struct solClient_destination 
{
    solClient_destinationType_t         destType; /**< The type of destination. */
    const char *                        dest;     /**< The name of the destination (as a NULL-terminated UTF-8 string). */
} solClient_destination_t;

/**
 * @enum solClient_fieldType
 * 
 * Data types that can be transmitted by the machine-independent read and write 
 * functions.
 */
typedef enum solClient_fieldType
{
    SOLCLIENT_BOOL      = 0,    /**< Boolean. */
    SOLCLIENT_UINT8     = 1,    /**< 8-bit unsigned integer. */
    SOLCLIENT_INT8      = 2,    /**< 8-bit signed integer. */
    SOLCLIENT_UINT16    = 3,    /**< 16-bit unsigned integer. */
    SOLCLIENT_INT16     = 4,    /**< 16-bit signed integer. */
    SOLCLIENT_UINT32    = 5,    /**< 32-bit unsigned integer. */
    SOLCLIENT_INT32     = 6,    /**< 32-bit signed integer. */
    SOLCLIENT_UINT64    = 7,    /**< 64-bit unsigned integer. */
    SOLCLIENT_INT64     = 8,    /**< 64-bit signed integer. */
    SOLCLIENT_WCHAR     = 9,   /**< 16-bit unicode character. */
    SOLCLIENT_STRING    = 10,   /**< Null terminated string, (ASCII or UTF-8). */
    SOLCLIENT_BYTEARRAY = 11,   /**< Byte array. */
    SOLCLIENT_FLOAT     = 12,   /**< 32-bit floating point number. */
    SOLCLIENT_DOUBLE    = 13,   /**< 64-bit floating point number. */
    SOLCLIENT_MAP       = 14,   /**< Solace Map (container class). */
    SOLCLIENT_STREAM    = 15,   /**< Solace Stream (container class). */
    SOLCLIENT_NULL      = 16,   /**< NULL field.*/
    SOLCLIENT_DESTINATION = 17, /**< Destination field. */
    SOLCLIENT_SMF       = 18,   /**< A complete Solace Message Format (SMF) message is encapsulated in the container. */
    SOLCLIENT_UNKNOWN   = -1    /**< A validly formatted, but unrecognized, data type was received. */
} solClient_fieldType_t;

/**
 * @struct solClient_field
 * The general solClient_field structure is returned by generic accessors to
 * the container. The application must first check the fieldType to determine
 * which member of the union to use.
 */

typedef struct solClient_field {
    solClient_fieldType_t       type;
    solClient_uint32_t          length;
    union {
        solClient_bool_t        boolean;
        solClient_uint8_t       uint8;
        solClient_int8_t        int8;
        solClient_uint16_t      uint16;
        solClient_int16_t       int16;
        solClient_uint32_t      uint32;
        solClient_int32_t       int32;
        solClient_uint64_t      uint64;
        solClient_int64_t       int64;
        solClient_wchar_t       wchar;
        float                   float32;
        double                  float64;
        const char             *string;
        solClient_uint8_t      *bytearray;
        solClient_opaqueContainer_pt
                                map;
        solClient_opaqueContainer_pt
                                stream;
        solClient_destination_t dest;
        solClient_uint8_t      *smf;
        solClient_uint8_t      *unknownField;
    } value;
} solClient_field_t;

/**
* @enum solClient_returnCode
* Return code from API calls.
* A return code can be converted to a string through solClient_returnCodeToString().
*/
  typedef enum solClient_returnCode
  {
    SOLCLIENT_OK = 0,           /**< The API call was successful. */
    SOLCLIENT_WOULD_BLOCK = 1,  /**< The API call would block, but non-blocking was requested. */
    SOLCLIENT_IN_PROGRESS = 2,  /**< An API call is in progress (non-blocking mode). */
    SOLCLIENT_NOT_READY = 3,    /**< The API could not complete as an object is not ready (for example, the Session is not connected). */
    SOLCLIENT_EOS  = 4,         /**< A getNext on a structured container returned End-of-Stream. */
    SOLCLIENT_NOT_FOUND = 5,    /**< A get for a named field in a MAP was not found in the MAP. */
    SOLCLIENT_NOEVENT = 6,      /**< solClient_context_processEventsWait returns this if wait is zero and there is no event to process */
    SOLCLIENT_INCOMPLETE = 7,   /**< The API call completed some, but not all, of the requested function. */
    SOLCLIENT_ROLLBACK = 8,     /**< solClient_transactedSession_commit returns this when the transaction has been rolled back. */
    SOLCLIENT_FAIL = -1         /**< The API call failed. */
  } solClient_returnCode_t;     /**< The type for API return codes. */

/**
 * @enum solClient_rxMsgCallback_returnCode
 *
 * The return code that the application returns to the API on each received message. This is used
 * by the message received functions with the prototype of ::solClient_session_rxMsgCallbackFunc_t
 * or ::solClient_flow_rxMsgCallbackFunc_t.
 */
  typedef enum solClient_rxMsgCallback_returnCode {
    SOLCLIENT_CALLBACK_OK       = 0, /**< Normal return - the message is destroyed by the API upon return. */
    SOLCLIENT_CALLBACK_TAKE_MSG = 1  /**< The application is keeping the rxMsg, and it must not be released or reused by the API .*/
  } solClient_rxMsgCallback_returnCode_t;

/**
* @enum solClient_subCode
* A subcode that provides more detailed error information. The last subcode is stored
* on a per-thread basis and can be retrieved by an application thread.  
*
* <b>NOTE: The error subCode is historic and only updated when an API does not return ::SOLCLIENT_OK. When an API call returns ::SOLCLIENT_OK 
* the subCode is not meaningful and might not always be ::SOLCLIENT_SUBCODE_OK.</b>
*
* A subCode is always set when a API call does not return ::SOLCLIENT_OK.
*
* The application must be able to accept any subcode to allow for new subcodes to be
* added in the future and allow for forward compatibility. When examining and
* acting on the subcode, the application should act on any specific subcodes it
* needs to take different actions on, and then must perform a default action for any
* other subcode value. 
* A subcode can be converted to a string through solClient_subCodeToString().
*
* The following subcodes can be returned from any API entry point: \n
* ::SOLCLIENT_SUBCODE_INIT_NOT_CALLED (except for functions documented as being allowed to be called before ::solClient_initialize) \n
* ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE \n
* ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR (functions that accept pointer parameters) \n
* ::SOLCLIENT_SUBCODE_PARAM_CONFLICT (functions that have interdependent parameters) \n
* ::SOLCLIENT_SUBCODE_INTERNAL_ERROR \n
* ::SOLCLIENT_SUBCODE_OS_ERROR \n
* ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY \n
* ::SOLCLIENT_SUBCODE_CANNOT_BLOCK_IN_CONTEXT (functions that result in an interaction with the message router) \n
*
* A complete list of SolClient subCodes, their meaning, and the appliance response that caused them (when applicable) follows:
*
* <table>
* <tr> <th width="300">SubCode</th> <th width="300">Description</th> <th width="300">Appliance Error Response</th></tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_OK                                    </td>
*     <td width="300"> No error.                                               </td>
*     <td width="300"> 200 OK                                                  </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE</td>
*     <td width="300"> An API call was made with an out-of-range parameter.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_PARAM_NULL_PTR</td>
*     <td width="300"> An API call was made with a NULL pointer parameter.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_PARAM_CONFLICT</td>
*     <td width="300"> An API call was made with a parameter combination that is not valid.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE</td>
*     <td width="300"> An API call failed due to insufficient space to accept more data.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="100"> SOLCLIENT_SUBCODE_OUT_OF_RESOURCES</td>
*     <td width="300"> An API call failed due to lack of resources (for example, starting a timer when all timers are in use).</td>
*     <td width="300"> 400 Not Enough Space                                    </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INTERNAL_ERROR</td>
*     <td width="300"> An API call had an internal error (not an application fault).</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_OUT_OF_MEMORY</td>
*     <td width="300"> An API call failed due to inability to allocate memory.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_PROTOCOL_ERROR</td>
*     <td width="300"> An API call failed due to a protocol error with the appliance (not an application fault).</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INIT_NOT_CALLED</td>
*     <td width="300"> An API call failed due to solClient_initialize() not being called first.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_TIMEOUT</td>
*     <td width="300"> An API call failed due to a timeout.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_KEEP_ALIVE_FAILURE</td>
*     <td width="300"> The Session Keep-Alive detected a failed Session.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED</td>
*     <td width="300"> An API call failed due to the Session not being established.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_OS_ERROR</td>
*     <td width="300"> An API call failed due to a failed operating system call. An error string can be retrieved with solClient_getLastErrorInfo().</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_COMMUNICATION_ERROR</td>
*     <td width="300"> An API call failed due to a communication error. An error string can be retrieved with solClient_getLastErrorInfo().</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_USER_DATA_TOO_LARGE</td>
*     <td width="300"> Attempt to send a message with user data larger than the maximum that is supported.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_TOPIC_TOO_LARGE</td>
*     <td width="300"> An attempt to use a Topic which is longer than the maximum that is supported.</td>
*     <td width="300"> N/A                                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX</td>
*     <td width="300"> An attempt to use a Topic which has a syntax which is not supported.</td>
*     <td width="300"> 400 Topic Parse Error                                   </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_XML_PARSE_ERROR</td>
*     <td width="300"> The appliance could not parse an XML message.</td>
*     <td width="300"> 400 XML Parse Error                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_LOGIN_FAILURE</td>
*     <td width="300"> The client could not log into the appliance (bad username or password, unknown parameter, etc.)</td>
*     <td width="300"> All 400, 401, 403 and 404 error codes from appliance                  </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_VIRTUAL_ADDRESS</td>
*     <td width="300"> An attempt to connect to the wrong IP address on the appliance (must use CVRID if configured), or the appliance CVRID has changed and this was detected on reconnect.</td>
*     <td width="300"> 403 Invalid Virtual Router Address                       </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CLIENT_DELETE_IN_PROGRESS</td>
*     <td width="300"> The client login not currently possible as previous instance of same client still being deleted.</td>
*     <td width="300"> 503 Subscriber Delete In Progress                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_TOO_MANY_CLIENTS</td>
*     <td width="300"> The client login not currently possible due to maximum number of active clients on appliance has already been reached.</td>
*     <td width="300"> "503 Too Many Clients" "503 Too Many Publishers" "503 Too Many Subscribers" "400 Too Many Subscribers"</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_ALREADY_PRESENT</td>
*     <td width="300"> The client attempted to add a subscription which already exists - this subcode is only returned if the Session property SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR is not enabled.</td>
*     <td width="300"> "400 already exists" "400 Subscription Already Exists"   </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_NOT_FOUND</td>
*     <td width="300"> The client attempted to remove a subscription which did not exist - this subcode is only returned if the Session property SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR is not enabled.</td>
*     <td width="300"> "400 not found" "400 Subscription Not Found"             </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_INVALID</td>
*     <td width="300"> The client attempted to add/remove a subscription that is not valid.</td>
*     <td width="300"> "400 not supported" "400 parse error" "400 Subscription Parse Error"</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER</td>
*     <td width="300"> The appliance rejected a subscription add or remove request for a reason not separately enumerated.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CONTROL_OTHER</td>
*     <td width="300"> The appliance rejected a control message for another reason not separately enumerated.</td>
*     <td width="300"> Default error subCode for TRB-equipped appliance error responses</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_DATA_OTHER</td>
*     <td width="300"> The appliance rejected a data message for another reason not separately enumerated.</td>
*     <td width="300"> Default error subCode for error response to published data</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_LOG_FILE_ERROR</td>
*     <td width="300"> Could not open the log file name specified by the application for writing (Deprecated - ::SOLCLIENT_SUBCODE_OS_ERROR is used).</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MESSAGE_TOO_LARGE</td>
*     <td width="300"> The client attempted to send a message larger than that supported by the appliance.</td>
*     <td width="300"> "400 Document Is Too Large" "400 Message Too Long"         </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_TOO_MANY</td>
*     <td width="300"> The client attempted to add a subscription that exceeded the maximum number allowed. </td>
*     <td width="300"> "400 Max Num Subscriptions Exceeded"                       </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION</td>
*     <td width="300"> An API call failed due to the attempted operation not being valid for the Session. </td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_TOPIC_MISSING</td>
*     <td width="300"> A send call was made that did not have a Topic in a mode where one is required (for example, client mode).</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_ASSURED_MESSAGING_NOT_ESTABLISHED</td>
*     <td width="300"> A send call was made to send a Guaranteed message before Guaranteed Mesaging is established (Deprecated).</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_ASSURED_MESSAGING_STATE_ERROR</td>
*     <td width="300"> An attempt was made to start Guaranteed Messaging when it is already started.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_QUEUENAME_TOPIC_CONFLICT</td>
*     <td width="300"> Both Queue name and Topic are specified in solClient_session_send.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_QUEUENAME_TOO_LARGE</td>
*     <td width="300"> An attempt was made to use a Queue name which is longer than the maximum supported length.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_QUEUENAME_INVALID_MODE</td>
*     <td width="300"> An attempt was made to use a Queue name on a non-Guaranteed (that is, Direct) message.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MAX_TOTAL_MSGSIZE_EXCEEDED</td>
*     <td width="300"> An attempt was made to send a message with a total size greater than that supported by the protocol.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_DBLOCK_ALREADY_EXISTS</td>
*     <td width="300"> An attempt was made to allocate a datablock for a msg element when one already exists.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA</td>
*     <td width="300"> An attempt was made to create a container to read structured data where none exists.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CONTAINER_BUSY</td>
*     <td width="300"> An attempt was made to add a field to a map or stream while a submap or substream is being built.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION</td>
*     <td width="300"> An attempt was made to retrieve structured data with the wrong type.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CANNOT_MODIFY_WHILE_NOT_IDLE</td>
*     <td width="300"> An attempt was made to modify a property that cannot be modified while Session is not idle.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MSG_VPN_NOT_ALLOWED</td>
*     <td width="300"> The Message VPN name set for the Session is not allowed for the Session's username.</td>
*     <td width="300"> 403 Message VPN Not Allowed</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CLIENT_NAME_INVALID</td>
*     <td width="300"> The client name chosen has been rejected as invalid by the appliance.</td>
*     <td width="300"> 400 Client Name Parse Error</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MSG_VPN_UNAVAILABLE</td>
*     <td width="300"> The Message VPN name set for the Session (or the default VPN if none was set) is currently shutdown on the appliance.</td>
*     <td width="300"> 503 Message VPN Unavailable</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CLIENT_USERNAME_IS_SHUTDOWN</td>
*     <td width="300"> The username for the client is administratively shutdown on the appliance.</td>
*     <td width="300"> 403 Client Username Is Shutdown </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_DYNAMIC_CLIENTS_NOT_ALLOWED</td>
*     <td width="300"> The username for the Session has not been set and dynamic clients are not allowed.</td>
*     <td width="300"> 403 Dynamic Clients Not Allowed</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CLIENT_NAME_ALREADY_IN_USE</td>
*     <td width="300"> The Session is attempting to use a client, publisher, or subscriber name that is in use by another client, publisher or subscriber and the appliance is configured to reject the new Session. When Message VPNs are in use, the conflicting client name must be in the same Message VPN.</td>
*     <td width="300"> "403 Client Name Already In Use" "403 Publisher Name Already In Use" "403 Subscriber Name Already In Use"</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CACHE_NO_DATA</td>
*     <td width="300"> When the cache request returns ::SOLCLIENT_INCOMPLETE, this subcode indicates there is no cached data in the designated cache.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CACHE_SUSPECT_DATA</td>
*     <td width="300"> When the designated cache responds to a cache request with suspect data the API returns ::SOLCLIENT_INCOMPLETE with this subcode.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CACHE_ERROR_RESPONSE</td>
*     <td width="300"> The cache instance has returned an error response to the request.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CACHE_INVALID_SESSION</td>
*     <td width="300"> The cache session operation failed because the Session has been destroyed.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CACHE_TIMEOUT</td>
*     <td width="300"> The cache session operation failed because the request timeout expired.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CACHE_LIVEDATA_FULFILL</td>
*     <td width="300"> The cache session operation completed when live data arrived on the requested topic.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CACHE_ALREADY_IN_PROGRESS</td>
*     <td width="300"> A cache request has been made when there is already a cache request outstanding on the same Topic and SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FLOWTHRU was not set.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MISSING_REPLY_TO</td>
*     <td width="300"> A message does not have a required reply-to field.</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CANNOT_BIND_TO_QUEUE</td>
*     <td width="300"> Already bound to the Queue or not authorized to bind to the Queue.</td>
*     <td width="300"> "400 Cannot bind to another subscriber's topic queue" "400 Already Bound"</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_TOPIC_NAME_FOR_TE</td>
*     <td width="300"> An attempt was made to bind to a Guaranteed Delivery Topic Endpoint with an invalid topic.</td>
*     <td width="300"> 400 Invalid Topic Name                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNKNOWN_QUEUE_NAME</td>
*     <td width="300"> An attempt was made to bind to an unknown Queue name (for example, not configured on appliance).</td>
*     <td width="300"> 503 Unknown Queue                                          </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNKNOWN_TE_NAME</td>
*     <td width="300"> An attempt was made to bind to an unknown Guaranteed Delivery Topic Endpoint name (for example, not configured on appliance).</td>
*     <td width="300"> "503 Unknown Durable Topic Endpoint"                       </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_QUEUE</td>
*     <td width="300"> An attempt was made to bind to a Guaranteed Delivery Queue which already has a maximum number of clients.</td>
*     <td width="300"> 503 Max clients exceeded for Queue                         </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_TE</td>
*     <td width="300"> An attempt was made to bind to a Guaranteed Delivery Topic Endpoint that already has a maximum number of clients.</td>
*     <td width="300"> 503 Max clients exceeded for durable Topic Endpoint        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNEXPECTED_UNBIND</td>
*     <td width="300"> An unexpected unbind response was received for a Guaranteed Delivery Queue or Topic Endpoint (for example, Queue or Topic Endpoint was deleted from the appliance).</td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_QUEUE_NOT_FOUND</td>
*     <td width="300"> The specified Guaranteed Delivery Queue was not found when publishing a message.</td>
*     <td width="300"> 400 Queue Not Found                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CLIENT_ACL_DENIED</td>
*     <td width="300"> The client login to the appliance was denied because the IP address/netmask combination used for the client is designated in the ACL (Access Control List) as a deny connection for the given Message VPN and username.</td>
*     <td width="300"> 403 Forbidden                                              </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED</td>
*     <td width="300"> The adding of a subscription was denied because it matched a subscription that was defined on the ACL (Access Control List).</td>
*     <td width="300"> 403 Subscription ACL Denied                                </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_PUBLISH_ACL_DENIED</td>
*     <td width="300"> A message could not be published because its Topic matched that of a Topic defined on the ACL (Access Control List).</td>
*     <td width="300"> 403 Publish ACL Denied                                     </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_DELIVER_TO_ONE_INVALID</td>
*     <td width="300"> An attempt was made to set both Deliver-To-One and Guaranteed Delivery in the same message.
*         (Deprecated: DTO will be applied to the corresponding demoted direct message)
*     </td>
*     <td width="300"> N/A                                                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SPOOL_OVER_QUOTA</td>
*     <td width="300"> Message was not delivered because the Guaranteed Message spool is over its allotted space quota.</td>
*     <td width="300"> 503 Spool Over Quota                                       </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_QUEUE_SHUTDOWN</td>
*     <td width="300"> An attempt was made to operate on a shutdown Guaranteed Delivery queue.</td>
*     <td width="300"> 503 Queue Shutdown                                         </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_TE_SHUTDOWN</td>
*     <td width="300"> An attempt was made to operate on a shutdown Guaranteed Delivery Topic Endpoint.</td>
*     <td width="300"> 503 Durable Topic Endpoint Shutdown, 503 TE Shutdown, 503 Endpoint Shutdown                        </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_NO_MORE_NON_DURABLE_QUEUE_OR_TE</td>
*     <td width="300"> An attempt was made to bind to a non-durable Guaranteed Delivery Queue or Topic Endpoint, and the appliance is out of resources.</td>
*     <td width="300"> 503 No More Non-Durable Queue or Topic Endpoint            </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_ENDPOINT_ALREADY_EXISTS</td>
*     <td width="300"> An attempt was made to create a Queue or Topic Endpoint that already exists. This subcode is only returned if the provision flag SOLCLIENT_PROVISION_FLAGS_IGNORE_EXIST_ERRORS is not set in solClient_session_endpointProvision.</td>
*     <td width="300"> 400 Endpoint Already Exists                                </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_PERMISSION_NOT_ALLOWED</td>
*     <td width="300"> An attempt was made to delete or create a Queue or Topic Endpoint when the Session does not have authorization for the action. This subcode is also returned when an attempt is made to remove a message from an endpoint when the Session does not have 'consume' authorization, or when an attempt is made to add or remove a Topic subscription from a Queue when the Session does not have 'modify-topic' authorization.</td>
*     <td width="300"> 403 Permission Denied                                     </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_SELECTOR</td>
*     <td width="300"> An attempt was made to bind to a Queue or Topic Endpoint with an invalid selector.</td>
*     <td width="300"> 406 Invalid Selector                                     </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MAX_MESSAGE_USAGE_EXCEEDED</td>
*     <td width="300"> Publishing the message was denied due to exceeding the maximum spooled message count.</td>
*     <td width="300"> 503 Max message usage exceeded</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_ENDPOINT_PROPERTY_MISMATCH</td>
*     <td width="300"> An attempt was made to create a dynamic durable endpoint, and it was found to exist with different properties. </td>
*     <td width="300"> 401 Endpoint Property Mismatch</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_MANAGER_DENIED</td>
*     <td width="300"> An attempt was made to add a subscription to another client when the Session does not have subscription manager privileges.</td>
*     <td width="300"> 403 Subscription Manager Denied</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNKNOWN_CLIENT_NAME</td>
*     <td width="300"> An attempt was made to add a subscription to another client that is unknown on the appliance.</td>
*     <td width="300"> 403 Unknown Client Name </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_QUOTA_OUT_OF_RANGE</td>
*     <td width="300"> An attempt was made to provision an endpoint with a quota that is out of range.</td>
*     <td width="300"> 400 Quota Out Of Range </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_ATTRIBUTES_CONFLICT</td>
*     <td width="300"> The client attempted to add a subscription which already exists but it has different properties, see ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE. </td>
*     <td width="300"> 400 Subscription Attributes Conflict With Existing Subscription</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_SMF_MESSAGE</td>
*     <td width="300"> The client attempted to send a Solace Message Format (SMF) message using solClient_session_sendSmf() or solClient_session_sendMultipleSmf(), but the buffer did not contain a Direct message.
*     </td>
*     <td width="300"> N/A</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_NO_LOCAL_NOT_SUPPORTED</td>
*     <td width="300"> The client attempted to establish a Session or Flow with No Local enabled and the capability is not supported by the appliance.
*     </td>
*     <td width="300"> N/A</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNSUBSCRIBE_NOT_ALLOWED_CLIENTS_BOUND</td>
*     <td width="300"> The client attempted to unsubscribe a Topic from a Topic Endpoint while there were still Flows bound to the endpoint.
*     </td>
*     <td width="300"> 400 Unsubscribe not allowed, client(s) bound to DTE.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CANNOT_BLOCK_IN_CONTEXT</td>
*     <td width="300"> An API function was invoked in the Context thread that would have blocked otherwise. For example, a call was made to send a message when the Session is configured with ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING enabled and the transport (socket or IPC) channel is full. All application callback functions are executed in the Context thread.</td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_FLOW_ACTIVE_FLOW_INDICATION_UNSUPPORTED</td>
*     <td width="300"> Reserved For Future Use. </td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNRESOLVED_HOST</td>
*     <td width="300"> The client failed to connect because the host name could not be resolved. </td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CUT_THROUGH_UNSUPPORTED</td>
*     <td width="300"> An attempt was made to create a 'cut-through' Flow on a Session that does not support this capability. </td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CUT_THROUGH_ALREADY_BOUND</td>
*     <td width="300">  An attempt was made to create a 'cut-through' Flow on a Session that already has one 'cut-through' Flow. </td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CUT_THROUGH_INCOMPATIBLE_WITH_SESSION</td>
*     <td width="300"> An attempt was made to create a 'cut-through' Flow on a Session with incompatible Session properties. Cut-through may not be enabled on Sessions with SOLCLIENT_SESSION_PROP_TOPIC_DISPATCH enabled.</td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_FLOW_OPERATION</td>
*     <td width="300"> An API call failed due to the attempted operation not being valid for the Flow.</td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNKNOWN_FLOW_NAME</td>
*     <td width="300"> The session was disconnected due to loss of the publisher flow state. All (unacked and unsent) messages held by the API were deleted. To connect the session, applications need to call ::solClient_session_connect again.</td>
*     <td width="200">  400 Unknown Flow Name</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLICATION_IS_STANDBY</td>
*     <td width="300"> An attempt to perform an operation using a VPN that is configured to be STANDBY for replication.</td>
*     <td width="200"> 403 Replication Is Standby</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_LOW_PRIORITY_MSG_CONGESTION</td>
*     <td width="300"> The message was rejected by the appliance as one or more matching endpoints exceeded the reject-low-priority-msg-limit.</td>
*     <td width="200"> 503 Low Priority Msg Congestion</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_FAILED_LOADING_TRUSTSTORE</td>
*     <td width="300"> The client failed to load the trust store.</td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNTRUSTED_CERTIFICATE</td>
*     <td width="300"> The client attempted to connect to an appliance that has a suspect certficate.</td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CERTIFICATE_DATE_INVALID</td>
*     <td width="300"> The client attempted to connect to an appliance that does not have a valid certificate date.</td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_LIBRARY_NOT_LOADED</td>
*     <td width="300"> The client failed to find the library or symbol.</td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNTRUSTED_COMMONNAME</td>
*     <td width="300"> The client attempted to connect to an appliance that has a suspect common name.</td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300">SOLCLIENT_SUBCODE_FAILED_LOADING_CERTIFICATE_AND_KEY</td>
*     <td width="300">The client failed to load certificate and/or private key files. </td>
*     <td width="200"> N/A.</td>
* </tr>
* <tr>
*     <td width="300">SOLCLIENT_SUBCODE_BASIC_AUTHENTICATION_IS_SHUTDOWN </td>
*     <td width="300">The client attempted to connect to an appliance that has the basic authentication shutdown. </td>
*     <td width="200"> 403 Basic Authentication is Shutdown</td>
* </tr>
* <tr>
*     <td width="300">SOLCLIENT_SUBCODE_CLIENT_CERTIFICATE_AUTHENTICATION_IS_SHUTDOWN </td>
*     <td width="300">The client attempted to connect to an appliance that has the client certificate authentication shutdown. </td>
*     <td width="200"> 403 Client Certificate Authentication is Shutdown</td>
* </tr>
* <tr>
*     <td width="300">SOLCLIENT_SUBCODE_KERBEROS_AUTHENTICATION_IS_SHUTDOWN </td>
*     <td width="300">The client attempted to connect to an appliance that has the Kerberos authentication shutdown. </td>
*     <td width="200"> 403 Kerberos Authentication is Shutdown</td>
* </tr>
* <tr>
*     <td width="300">SOLCLIENT_SUBCODE_UNTRUSTED_CLIENT_CERTIFICATE </td>
*     <td width="300"> The client failed to connect to an appliance as it has a suspect client certificate.</td>
*     <td width="200"> "403 Untrusted Certificate"  "403 Certificate Chain Too Long" "403 Certificate Error"</td>
* </tr>
* <tr>
*     <td width="300">SOLCLIENT_SUBCODE_CLIENT_CERTIFICATE_DATE_INVALID </td>
*     <td width="300"> The client failed to connect to an appliance as it does not have a valid client certificate date. </td>
*     <td width="200"> "403 Certificate Not Yet Valid" "403 Certificate Expired"</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CACHE_REQUEST_CANCELLED</td>
*     <td width="300"> The cache session operation failed because the request has been cancelled.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_DELIVERY_MODE_UNSUPPORTED</td>
*     <td width="300"> Attempt was made from a Transacted Session to send a message with the delivery mode SOLCLIENT_DELIVERY_MODE_DIRECT.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_PUBLISHER_NOT_CREATED</td>
*     <td width="300"> Client attempted to send a message from a Transacted Session without creating a default publisher flow.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_FLOW_UNBOUND</td>
*     <td width="300"> The client attempted to receive message from an UNBOUND Flow with no queued messages in memory.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_TRANSACTED_SESSION_ID</td>
*     <td width="300"> The client attempted to commit or rollback a transaction with an invalid Transacted Session Id.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_TRANSACTION_ID</td>
*     <td width="300"> The client attempted to commit or rollback a transaction with an invalid transaction Id.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MAX_TRANSACTED_SESSIONS_EXCEEDED</td>
*     <td width="300"> The client failed to open a Transacted Session as it exceeded the max Transacted Sessions.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_TRANSACTED_SESSION_NAME_IN_USE</td>
*     <td width="300"> The client failed to open a Transacted Session as the Transacted Session name provided is being used by another opened session.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SERVICE_UNAVAILABLE</td>
*     <td width="300"> Guaranteed Delivery services are not enabled on the appliance.</td>
*     <td width="300"> 503 Service Unavailable</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_NO_TRANSACTION_STARTED</td>
*     <td width="300"> The client attempted to commit an unknown transaction.</td>
*     <td width="300"> 400 No transaction started</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_PUBLISHER_NOT_ESTABLISHED</td>
*     <td width="300"> A send call was made on a transacted session before its publisher is established.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MESSAGE_PUBLISH_FAILURE</td>
*     <td width="300"> The client attempted to commit a transaction with a GD publish failure encountered.</td>
*     <td width="300"> 503 Message Publish Failure</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_TRANSACTION_FAILURE</td>
*     <td width="300"> The client attempted to commit a transaction with too many transaction steps.</td>
*     <td width="300"> 503 Transaction Failure </td> 
* </tr>
* <tr>
*     <td width="300">SOLCLIENT_SUBCODE_MESSAGE_CONSUME_FAILURE </td>
*     <td width="300"> The client attempted to commit a transaction with a consume failure encountered. </td>
*     <td width="300"> 503 Message Consume Failure </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_ENDPOINT_MODIFIED</td>
*     <td width="300"> The client attempted to commit a transaction with an Endpoint being shutdown or deleted. </td>
*     <td width="300"> 503 Endpoint Modified </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_INVALID_CONNECTION_OWNER </td>
*     <td width="300"> The client attempted to commit a transaction with an unknown connection ID. </td>
*     <td width="300"> 400 Invalid Connection Owner </td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_COMMIT_OR_ROLLBACK_IN_PROGRESS </td>
*     <td width="300">  The client attempted to send/receive a message or commit/rollback a transaction when a transaction commit/rollback is in progress. </td>
*     <td width="300"> N/A</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNBIND_RESPONSE_LOST </td>
*     <td width="300"> The application called solClient_flow_destroy() and the unbind-response was not received. </td>
*     <td width="300"> N/A</td> 
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MAX_TRANSACTIONS_EXCEEDED</td>
*     <td width="300"> The client failed to open a Transacted Session as the maximum number of transactions was exceeded.</td>
*     <td width="300"> N/A. </td> 
* </tr>
* <tr>
*    <td width="300"> SOLCLIENT_SUBCODE_COMMIT_STATUS_UNKNOWN</td>
*    <td width="300"> The commit response was lost due to a transport layer reconnection to an alternate host in the host list.</td>
*    <td width="300"> N/A. </td>
* </tr>
* <tr>
*    <td width="300"> SOLCLIENT_SUBCODE_PROXY_AUTH_REQUIRED</td>
*    <td width="300"> The host entry did not contain proxy authentication when required by the proxy server.</td>
*    <td width="300"> 407 Proxy Authentication Required</td>
* </tr>
* <tr>
*    <td width="300"> SOLCLIENT_SUBCODE_PROXY_AUTH_FAILURE</td>
*    <td width="300"> The host entry did contained invalid proxy authentication when required by the proxy server.</td>
*    <td width="300"> 407 Proxy Authentication Required</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_NO_SUBSCRIPTION_MATCH </td>
*     <td width="300"> The client attempted to publish a guaranteed message to a topic that did not have any guaranteed subscription matches or only matched a replicated topic. </td>
*     <td width="300"> 503 No Subscription Match </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SUBSCRIPTION_MATCH_ERROR </td>
*     <td width="300"> The client attempted to bind to a non-exclusive topic endpoint that is already bound with a different subscription.</td>
*     <td width="300"> 503 Subscription Does Not Match </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_SELECTOR_MATCH_ERROR </td>
*     <td width="300"> The client attempted to bind to a non-exclusive topic endpoint that is already bound with a different ingress selector.</td>
*     <td width="300"> 503 Selector Does Not Match </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_NOT_SUPPORTED </td>
*     <td width="300"> Replay is not supported in the Solace Message Router. </td>
*     <td width="300"> N/A </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_DISABLED </td>
*     <td width="300"> Replay is not enabled in the message-vpn. </td>
*     <td width="300"> 503 Replay Disabled </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_NON_EXCLUSIVE_NOT_ALLOWED </td>
*     <td width="300"> The client attempted to start replay on a flow bound to a non-exclusive endpoint. </td>
*     <td width="300"> 403 Client Initiated Replay Not Allowed on Non-Exclusive Topic Endpoint, 403 Client Initiated Replay Not Allowed on Non-Exclusive Queue </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_INACTIVE_FLOW_NOT_ALLOWED </td>
*     <td width="300"> The client attempted to start replay on an inactive flow. </td>
*     <td width="300"> 403 Client Initiated Replay from Inactive Flow Not Allowed</td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_BROWSER_FLOW_NOT_ALLOWED </td>
*     <td width="300"> The client attempted to bind with both ::SOLCLIENT_FLOW_PROP_BROWSER enabled and ::SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION set. </td>
*     <td width="300"> 403 Client Initiated Replay from Browser Flow Not Allowed </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_TEMPORARY_NOT_SUPPORTED  </td>
*     <td width="300"> Replay is not supported on temporary endpoints. </td>
*     <td width="300"> 403 Replay Not Supported on Temporary Queue, 403 Replay Not Supported on Temporary Topic Endpoint </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_UNKNOWN_START_LOCATION_TYPE  </td>
*     <td width="300"> The request attempted to start a replay but provided an unknown start location type. </td>
*     <td width="300"> 403 Unknown Start Location Type </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_CANCELLED </td>
*     <td width="300"> A replay in progress on a flow was administratively cancelled, causing the flow to be unbound. </td>
*     <td width="300"> 503 Replay Cancelled </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_MESSAGE_UNAVAILABLE                     </td>
*     <td width="300"> A replay in progress on a flow failed because messages to be replayed were trimmed from the replay log. </td>
*     <td width="300"> 503 Replay Message Unavailable </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_STARTED                     </td>
*     <td width="300"> A replay was started on the queue/topic endpoint, either by another client or by an adminstrator on the message router. </td>
*     <td width="300"> 503 Replay Started </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_START_TIME_NOT_AVAILABLE                </td>
*     <td width="300"> A replay was requested but the requested start time is not available in the replay log. </td>
*     <td width="300"> 503 Replay Start Time Not Available </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_MESSAGE_REJECTED                        </td>
*     <td width="300"> The Solace Message Router attempted to replay a message, but the queue/topic endpoint rejected the message to the sender. </td>
*     <td width="300"> 503 Replayed Message Rejected by Queue, 503 Replayed Message Rejected by Topic Endpoint </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_LOG_MODIFIED                            </td>
*     <td width="300"> A replay in progress on a flow failed because the replay log was modified. </td>
*     <td width="300"> 503 Replay Log Modified </td>
* </tr>

* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_MISMATCHED_ENDPOINT_ERROR_ID                   </td>
*     <td width="300"> Endpoint error ID in the bind request does not match the endpoint's error ID. </td>
*     <td width="300"> 503 Mismatched Endpoint Error ID </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_OUT_OF_REPLAY_RESOURCES                        </td>
*     <td width="300"> A replay was requested, but the router does not have sufficient resources to fulfill the request, due to too many active replays. </td>
*     <td width="300"> 503 Out of Replay Resources </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_TOPIC_OR_SELECTOR_MODIFIED_ON_DURABLE_TOPIC_ENDPOINT </td>
*     <td width="300"> A replay was in progress on a Durable Topic Endpoint (DTE) when its topic or selector was modified, causing the replay to fail. </td>
*     <td width="300"> 503 Topic or Selector Modified on Durable Topic Endpoint </td>
* </tr>

* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_REPLAY_FAILED                                  </td>
*     <td width="300"> A replay in progress on a flow failed. </td>
*     <td width="300"> 503 Replay Failed </td>
* </tr>
* <tr>
*     <td width="300"> SOLCLIENT_SUBCODE_COMPRESSED_SSL_NOT_SUPPORTED </td>
*     <td width="300"> The client attempted to establish a Session or Flow with ssl and compression, but the capability is not supported by the appliance.</td>
*     <td width="300"> N/A </td>
* </tr>
* </table>
*/
  typedef enum solClient_subCode
  {
    SOLCLIENT_SUBCODE_OK                                = 0,  /**< No error. */
    SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE                = 1,  /**< An API call was made with an out-of-range parameter. */
    SOLCLIENT_SUBCODE_PARAM_NULL_PTR                    = 2,  /**< An API call was made with a null or invalid pointer parameter. */
    SOLCLIENT_SUBCODE_PARAM_CONFLICT                    = 3,  /**< An API call was made with a parameter combination that is not valid. */
    SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE                = 4,  /**< An API call failed due to insufficient space to accept more data. */
    SOLCLIENT_SUBCODE_OUT_OF_RESOURCES                  = 5,  /**< An API call failed due to lack of resources (for example, starting a timer when all timers are in use). */
    SOLCLIENT_SUBCODE_INTERNAL_ERROR                    = 6,  /**< An API call had an internal error (not an application fault). */
    SOLCLIENT_SUBCODE_OUT_OF_MEMORY                     = 7,  /**< An API call failed due to inability to allocate memory. */
    SOLCLIENT_SUBCODE_PROTOCOL_ERROR                    = 8,  /**< An API call failed due to a protocol error with the appliance (not an application fault). */
    SOLCLIENT_SUBCODE_INIT_NOT_CALLED                   = 9,  /**< An API call failed due to solClient_initialize() not being called first. */
    SOLCLIENT_SUBCODE_TIMEOUT                           = 10, /**< An API call failed due to a timeout. */
    SOLCLIENT_SUBCODE_KEEP_ALIVE_FAILURE                = 11, /**< The Session Keep-Alive detected a failed Session. */
    SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED           = 12, /**< An API call failed due to the Session not being established. */
    SOLCLIENT_SUBCODE_OS_ERROR                          = 13, /**< An API call failed due to a failed operating system call; an error string can be retrieved with solClient_getLastErrorInfo(). */
    SOLCLIENT_SUBCODE_COMMUNICATION_ERROR               = 14, /**< An API call failed due to a communication error. An error string can be retrieved with solClient_getLastErrorInfo(). */
    SOLCLIENT_SUBCODE_USER_DATA_TOO_LARGE               = 15, /**< An attempt was made to send a message with user data larger than the maximum that is supported. */
    SOLCLIENT_SUBCODE_TOPIC_TOO_LARGE                   = 16, /**< An attempt was made to use a Topic that is longer than the maximum that is supported. */
    SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX              = 17, /**< An attempt was made to use a Topic that has a syntax which is not supported. */
    SOLCLIENT_SUBCODE_XML_PARSE_ERROR                   = 18, /**< The appliance could not parse an XML message. */
    SOLCLIENT_SUBCODE_LOGIN_FAILURE                     = 19, /**< The client could not log into the appliance (bad username or password). */
    SOLCLIENT_SUBCODE_INVALID_VIRTUAL_ADDRESS           = 20, /**< An attempt was made to connect to the wrong IP address on the appliance (must use CVRID if configured) or the appliance CVRID has changed and this was detected on reconnect. */
    SOLCLIENT_SUBCODE_CLIENT_DELETE_IN_PROGRESS         = 21, /**< The client login not currently possible as previous instance of same client still being deleted. */
    SOLCLIENT_SUBCODE_TOO_MANY_CLIENTS                  = 22, /**< The client login not currently possible becuase the maximum number of active clients on appliance has already been reached. */
    SOLCLIENT_SUBCODE_SUBSCRIPTION_ALREADY_PRESENT      = 23, /**< The client attempted to add a subscription which already exists. This subcode is only returned if the Session property SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR is not enabled. */
    SOLCLIENT_SUBCODE_SUBSCRIPTION_NOT_FOUND            = 24, /**< The client attempted to remove a subscription which did not exist. This subcode is only returned if the Session property SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR is not enabled. */
    SOLCLIENT_SUBCODE_SUBSCRIPTION_INVALID              = 25, /**< The client attempted to add/remove a subscription that is not valid. */
    SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER                = 26, /**< The appliance rejected a subscription add or remove request for a reason not separately enumerated. */
    SOLCLIENT_SUBCODE_CONTROL_OTHER                     = 27, /**< The appliance rejected a control message for another reason not separately enumerated. */
    SOLCLIENT_SUBCODE_DATA_OTHER                        = 28, /**< The appliance rejected a data message for another reason not separately enumerated. */
    SOLCLIENT_SUBCODE_LOG_FILE_ERROR                    = 29, /**< Could not open the log file name specified by the application for writing (Deprecated - ::SOLCLIENT_SUBCODE_OS_ERROR is used). */
    SOLCLIENT_SUBCODE_MESSAGE_TOO_LARGE                 = 30, /**< The client attempted to send a message larger than that supported by the appliance. */
    SOLCLIENT_SUBCODE_SUBSCRIPTION_TOO_MANY             = 31, /**< The client attempted to add a subscription that exceeded the maximum number allowed.  */
    SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION         = 32, /**< An API call failed due to the attempted operation not being valid for the Session.  */
    SOLCLIENT_SUBCODE_TOPIC_MISSING                     = 33, /**< A send call was made that did not have a Topic in a mode where one is required (for example, client mode). */
    SOLCLIENT_SUBCODE_ASSURED_MESSAGING_NOT_ESTABLISHED = 34, /**< A send call was made to send a Guaranteed message before Guaranteed Delivery is established (Deprecated). */
    SOLCLIENT_SUBCODE_ASSURED_MESSAGING_STATE_ERROR     = 35, /**< An attempt was made to start Guaranteed Delivery when it is already started. */
    SOLCLIENT_SUBCODE_QUEUENAME_TOPIC_CONFLICT          = 36, /**< Both Queue Name and Topic are specified in solClient_session_send. */
    SOLCLIENT_SUBCODE_QUEUENAME_TOO_LARGE               = 37, /**< An attempt was made to use a Queue name which is longer than the maximum supported length. */
    SOLCLIENT_SUBCODE_QUEUENAME_INVALID_MODE            = 38, /**< An attempt was made to use a Queue name on a non-Guaranteed message. */
    SOLCLIENT_SUBCODE_MAX_TOTAL_MSGSIZE_EXCEEDED        = 39, /**< An attempt was made to send a message with a total size greater than that supported by the protocol. */
    SOLCLIENT_SUBCODE_DBLOCK_ALREADY_EXISTS             = 40, /**< An attempt was made to allocate a datablock for a msg element when one already exists. */
    SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA                = 41, /**< An attempt was made to create a container to read structured data where none exists. */
    SOLCLIENT_SUBCODE_CONTAINER_BUSY                    = 42, /**< An attempt was made to add a field to a map or stream while a sub map or stream is being built. */
    SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION           = 43, /**< An attempt was made to retrieve structured data with wrong type. */
    SOLCLIENT_SUBCODE_CANNOT_MODIFY_WHILE_NOT_IDLE      = 44, /**< An attempt was made to modify a property that cannot be modified while Session is not idle. */
    SOLCLIENT_SUBCODE_MSG_VPN_NOT_ALLOWED               = 45, /**< The Message VPN name set for the Session is not allowed for the Session's username. */
    SOLCLIENT_SUBCODE_CLIENT_NAME_INVALID               = 46, /**< The client name chosen has been rejected as invalid by the appliance. */
    SOLCLIENT_SUBCODE_MSG_VPN_UNAVAILABLE               = 47, /**< The Message VPN name set for the Session (or the default Message VPN, if none was set) is currently shutdown on the appliance. */
    SOLCLIENT_SUBCODE_CLIENT_USERNAME_IS_SHUTDOWN       = 48, /**< The username for the client is administratively shutdown on the appliance. */
    SOLCLIENT_SUBCODE_DYNAMIC_CLIENTS_NOT_ALLOWED       = 49, /**< The username for the Session has not been set and dynamic clients are not allowed. */
    SOLCLIENT_SUBCODE_CLIENT_NAME_ALREADY_IN_USE        = 50, /**< The Session is attempting to use a client, publisher name, or subscriber name that is in use by another client, publisher, or subscriber, and the appliance is configured to reject the new Session. When Message VPNs are in use, the conflicting client name must be in the same Message VPN. */
    SOLCLIENT_SUBCODE_CACHE_NO_DATA                     = 51, /**< When the cache request returns ::SOLCLIENT_INCOMPLETE, this subcode indicates there is no cached data in the designated cache. */
    SOLCLIENT_SUBCODE_CACHE_SUSPECT_DATA                = 52, /**< When the designated cache responds to a cache request with suspect data the API returns ::SOLCLIENT_INCOMPLETE with this subcode. */
    SOLCLIENT_SUBCODE_CACHE_ERROR_RESPONSE              = 53, /**< The cache instance has returned an error response to the request. */
    SOLCLIENT_SUBCODE_CACHE_INVALID_SESSION             = 54, /**< The cache session operation failed because the Session has been destroyed. */
    SOLCLIENT_SUBCODE_CACHE_TIMEOUT                     = 55, /**< The cache session operation failed because the request timeout expired. */
    SOLCLIENT_SUBCODE_CACHE_LIVEDATA_FULFILL            = 56, /**< The cache session operation completed when live data arrived on the Topic requested. */
    SOLCLIENT_SUBCODE_CACHE_ALREADY_IN_PROGRESS         = 57, /**< A cache request has been made when there is already a cache request outstanding on the same Topic and SOLCLIENT_CACHEREQUEST_FLAGS_LIVEDATA_FLOWTHRU was not set. */
    SOLCLIENT_SUBCODE_MISSING_REPLY_TO                  = 58, /**< A message does not have the required reply-to field. */
    SOLCLIENT_SUBCODE_CANNOT_BIND_TO_QUEUE              = 59, /**< Already bound to the queue, or not authorized to bind to the queue. */
    SOLCLIENT_SUBCODE_INVALID_TOPIC_NAME_FOR_TE         = 60, /**< An attempt was made to bind to a Topic Endpoint with an invalid topic. */
    SOLCLIENT_SUBCODE_INVALID_TOPIC_NAME_FOR_DTE        = SOLCLIENT_SUBCODE_INVALID_TOPIC_NAME_FOR_TE, /**< Deprecated name; ::SOLCLIENT_SUBCODE_INVALID_TOPIC_NAME_FOR_TE is preferred. */
    SOLCLIENT_SUBCODE_UNKNOWN_QUEUE_NAME                = 61, /**< An attempt was made to bind to an unknown Queue name (for example, not configured on appliance).*/
    SOLCLIENT_SUBCODE_UNKNOWN_TE_NAME                   = 62, /**< An attempt was made to bind to an unknown Topic Endpoint name (for example, not configured on appliance). */
    SOLCLIENT_SUBCODE_UNKNOWN_DTE_NAME                  = SOLCLIENT_SUBCODE_UNKNOWN_TE_NAME, /**< Deprecated name; ::SOLCLIENT_SUBCODE_UNKNOWN_TE_NAME is preferred. */
    SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_QUEUE             = 63, /**< An attempt was made to bind to a Queue that already has a maximum number of clients. */
    SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_TE                = 64, /**< An attempt was made to bind to a Topic Endpoint that already has a maximum number of clients. */
    SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_DTE               = SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_TE, /**< Deprecated name, ::SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_TE is preferred. */
    SOLCLIENT_SUBCODE_UNEXPECTED_UNBIND                 = 65, /**< An unexpected unbind response was received for a Queue or Topic Endpoint (for example, the Queue or Topic Endpoint was deleted from the appliance). */
    SOLCLIENT_SUBCODE_QUEUE_NOT_FOUND                   = 66, /**< The specified Queue was not found when publishing a message. */
    SOLCLIENT_SUBCODE_CLIENT_ACL_DENIED                 = 67, /**< The client login to the appliance was denied because the IP address/netmask combination used for the client is designated in the ACL (Access Control List) as a deny connection for the given Message VPN and username. */
    SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED           = 68, /**< Adding a subscription was denied because it matched a subscription that was defined on the ACL (Access Control List). */
    SOLCLIENT_SUBCODE_PUBLISH_ACL_DENIED                = 69, /**< A message could not be published because its Topic matched a Topic defined on the ACL (Access Control List). */
    SOLCLIENT_SUBCODE_DELIVER_TO_ONE_INVALID            = 70, /**< An attempt was made to set both Deliver-To-One (DTO) and Guaranteed Delivery in the same message. (Deprecated:  DTO will be applied to the corresponding demoted direct message) */
    SOLCLIENT_SUBCODE_SPOOL_OVER_QUOTA                  = 71, /**< Message was not delivered because the Guaranteed message spool is over its allotted space quota. */
    SOLCLIENT_SUBCODE_QUEUE_SHUTDOWN                    = 72, /**< An attempt was made to operate on a shutdown queue. */
    SOLCLIENT_SUBCODE_TE_SHUTDOWN                       = 73, /**< An attempt was made to bind to a shutdown Topic Endpoint. */
    SOLCLIENT_SUBCODE_NO_MORE_NON_DURABLE_QUEUE_OR_TE   = 74, /**< An attempt was made to bind to a non-durable Queue or Topic Endpoint, and the appliance is out of resources. */
    SOLCLIENT_SUBCODE_ENDPOINT_ALREADY_EXISTS           = 75,  /**< An attempt was made to create a Queue or Topic Endpoint that already exists. This subcode is only returned if the provision flag SOLCLIENT_PROVISION_FLAGS_IGNORE_EXIST_ERRORS is not set. */
    SOLCLIENT_SUBCODE_PERMISSION_NOT_ALLOWED            = 76,  /**< An attempt was made to delete or create a Queue or Topic Endpoint when the Session does not have authorization for the action. This subcode is also returned when an attempt is made to remove a message from an endpoint when the Session does not have 'consume' authorization, or when an attempt is made to add or remove a Topic subscription from a Queue when the Session does not have 'modify-topic' authorization. */
    SOLCLIENT_SUBCODE_INVALID_SELECTOR                  = 77,  /**< An attempt was made to bind to a Queue or Topic Endpoint with an invalid selector. */
    SOLCLIENT_SUBCODE_MAX_MESSAGE_USAGE_EXCEEDED        = 78,  /**< Publishing of message denied because the maximum spooled message count was exceeded.  */
    SOLCLIENT_SUBCODE_ENDPOINT_PROPERTY_MISMATCH        = 79,  /**< An attempt was made to create a dynamic durable endpoint and it was found to exist with different properties. */
    SOLCLIENT_SUBCODE_SUBSCRIPTION_MANAGER_DENIED       = 80,  /**< An attempt was made to add a subscription to another client when Session does not have subscription manager privileges. */
    SOLCLIENT_SUBCODE_UNKNOWN_CLIENT_NAME               = 81,  /**< An attempt was made to add a subscription to another client that is unknown on the appliance. */
    SOLCLIENT_SUBCODE_QUOTA_OUT_OF_RANGE                = 82,  /**< An attempt was made to provision an endpoint with a quota that is out of range. */
    SOLCLIENT_SUBCODE_SUBSCRIPTION_ATTRIBUTES_CONFLICT  = 83,  /**< The client attempted to add a subscription which already exists but it has different properties, see ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE */
    SOLCLIENT_SUBCODE_INVALID_SMF_MESSAGE               = 84, /**< The client attempted to send a Solace Message Format (SMF) message using solClient_session_sendSmf() or solClient_session_sendMultipleSmf(), but the buffer did not contain a Direct message. */
    SOLCLIENT_SUBCODE_NO_LOCAL_NOT_SUPPORTED            = 85, /**< The client attempted to establish a Session or Flow with No Local enabled and the capability is not supported by the appliance. */
    SOLCLIENT_SUBCODE_UNSUBSCRIBE_NOT_ALLOWED_CLIENTS_BOUND = 86,  /**< The client attempted to unsubscribe a Topic from a Topic Endpoint while there were still Flows bound to the endpoint. */
    SOLCLIENT_SUBCODE_CANNOT_BLOCK_IN_CONTEXT           = 87, /**< An API function was invoked in the Context thread that would have blocked otherwise. For an example, a call may have been made to send a message when the Session is configured with ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING enabled and the transport (socket or IPC) channel is full. All application callback functions are executed in the Context thread. */
    SOLCLIENT_SUBCODE_FLOW_ACTIVE_FLOW_INDICATION_UNSUPPORTED  = 88, /**< The client attempted to establish a Flow with Active Flow Indication (SOLCLIENT_FLOW_PROP_ACTIVE_FLOW_IND) enabled and the capability is not supported by the appliance */
    SOLCLIENT_SUBCODE_UNRESOLVED_HOST                   = 89, /**< The client failed to connect because the host name could not be resolved. */
    SOLCLIENT_SUBCODE_CUT_THROUGH_UNSUPPORTED           = 90, /**< An attempt was made to create a 'cut-through' Flow on a Session that does not support this capability */
    SOLCLIENT_SUBCODE_CUT_THROUGH_ALREADY_BOUND         = 91, /**< An attempt was made to create a 'cut-through' Flow on a Session that already has one 'cut-through' Flow */
    SOLCLIENT_SUBCODE_CUT_THROUGH_INCOMPATIBLE_WITH_SESSION = 92, /**< An attempt was made to create a 'cut-through' Flow on a Session with incompatible Session properties. Cut-through may not be enabled on Sessions with SOLCLIENT_SESSION_PROP_TOPIC_DISPATCH enabled.  */
    SOLCLIENT_SUBCODE_INVALID_FLOW_OPERATION            = 93, /**< An API call failed due to the attempted operation not being valid for the Flow.  */
    SOLCLIENT_SUBCODE_UNKNOWN_FLOW_NAME                 = 94, /**<The session was disconnected due to loss of the publisher flow state. All (unacked and unsent) messages held by the API were deleted. To connect the session, applications need to call ::solClient_session_connect again. */
    SOLCLIENT_SUBCODE_REPLICATION_IS_STANDBY            = 95, /**<An attempt to perform an operation using a VPN that is configured to be STANDBY for replication. */
    SOLCLIENT_SUBCODE_LOW_PRIORITY_MSG_CONGESTION       = 96,  /**<The message was rejected by the appliance as one or more matching endpoints exceeded the reject-low-priority-msg-limit. */
    SOLCLIENT_SUBCODE_LIBRARY_NOT_LOADED                = 97,  /**< The client failed to find the library or symbol. */
    SOLCLIENT_SUBCODE_FAILED_LOADING_TRUSTSTORE         = 98, /**< The client failed to load the trust store. */
    SOLCLIENT_SUBCODE_UNTRUSTED_CERTIFICATE             = 99, /**< The client attempted to connect to an appliance that has a suspect certficate. */
    SOLCLIENT_SUBCODE_UNTRUSTED_COMMONNAME              = 100, /**< The client attempted to connect to an appliance that has a suspect common name. */
    SOLCLIENT_SUBCODE_CERTIFICATE_DATE_INVALID          = 101, /**< The client attempted to connect to an appliance that does not have a valid certificate date. */
    SOLCLIENT_SUBCODE_FAILED_LOADING_CERTIFICATE_AND_KEY             = 102, /**< The client failed to load certificate and/or private key files. */
    SOLCLIENT_SUBCODE_BASIC_AUTHENTICATION_IS_SHUTDOWN               = 103, /**<  The client attempted to connect to an appliance that has the basic authentication shutdown. */
    SOLCLIENT_SUBCODE_CLIENT_CERTIFICATE_AUTHENTICATION_IS_SHUTDOWN  = 104, /**<  The client attempted to connect to an appliance that has the client certificate authentication shutdown. */
    SOLCLIENT_SUBCODE_UNTRUSTED_CLIENT_CERTIFICATE                   = 105, /**< The client failed to connect to an appliance as it has a suspect client certificate. */
    SOLCLIENT_SUBCODE_CLIENT_CERTIFICATE_DATE_INVALID                = 106, /**< The client failed to connect to an appliance as it does not have a valid client certificate date. */
    SOLCLIENT_SUBCODE_CACHE_REQUEST_CANCELLED                        = 107, /**< The cache request has been cancelled by the client. */
    SOLCLIENT_SUBCODE_DELIVERY_MODE_UNSUPPORTED                      = 108, /**< Attempt was made from a Transacted Session to send a message with the delivery mode SOLCLIENT_DELIVERY_MODE_DIRECT.*/
    SOLCLIENT_SUBCODE_PUBLISHER_NOT_CREATED                          = 109, /**< Client attempted to send a message from a Transacted Session without creating a default publisher flow. */ 
    SOLCLIENT_SUBCODE_FLOW_UNBOUND                                   = 110, /**< The client attempted to receive message from an UNBOUND Flow with no queued messages in memory. */
    SOLCLIENT_SUBCODE_INVALID_TRANSACTED_SESSION_ID                  = 111, /**< The client attempted to commit or rollback a transaction with an invalid Transacted Session Id. */
    SOLCLIENT_SUBCODE_INVALID_TRANSACTION_ID                         = 112, /**< The client attempted to commit or rollback a transaction with an invalid transaction Id. */
    SOLCLIENT_SUBCODE_MAX_TRANSACTED_SESSIONS_EXCEEDED               = 113, /**< The client failed to open a Transacted Session as it exceeded the max Transacted Sessions. */
    SOLCLIENT_SUBCODE_TRANSACTED_SESSION_NAME_IN_USE                 = 114, /**< The client failed to open a Transacted Session as the Transacted Session name provided is being used by another opened session. */
    SOLCLIENT_SUBCODE_SERVICE_UNAVAILABLE                            = 115, /**< Guaranteed Delivery services are not enabled on the appliance. */
    SOLCLIENT_SUBCODE_NO_TRANSACTION_STARTED                         = 116, /**< The client attempted to commit an unknown transaction. */
    SOLCLIENT_SUBCODE_PUBLISHER_NOT_ESTABLISHED                      = 117, /**< A send call was made on a transacted session before its publisher is established. */
    SOLCLIENT_SUBCODE_MESSAGE_PUBLISH_FAILURE                        = 118, /**< The client attempted to commit a transaction with a GD publish failure encountered.  */
    SOLCLIENT_SUBCODE_TRANSACTION_FAILURE                            = 119, /**< The client attempted to commit a transaction with too many transaction steps. */
    SOLCLIENT_SUBCODE_MESSAGE_CONSUME_FAILURE                        = 120, /**< The client attempted to commit a transaction with a consume failure encountered.  */
    SOLCLIENT_SUBCODE_ENDPOINT_MODIFIED                              = 121, /**< The client attempted to commit a transaction with an Endpoint being shutdown or deleted. */
    SOLCLIENT_SUBCODE_INVALID_CONNECTION_OWNER                       = 122, /**< The client attempted to commit a transaction with an unknown connection ID. */  
    SOLCLIENT_SUBCODE_KERBEROS_AUTHENTICATION_IS_SHUTDOWN            = 123, /**< The client attempted to connect to an appliance that has the Kerberos authentication shutdown. */
    SOLCLIENT_SUBCODE_COMMIT_OR_ROLLBACK_IN_PROGRESS                 = 124, /**< The client attempted to send/receive a message or commit/rollback a transaction when a transaction commit/rollback is in progress. */
    SOLCLIENT_SUBCODE_UNBIND_RESPONSE_LOST                           = 125, /**< The application called solClient_flow_destroy() and the unbind-response was not received. */
    SOLCLIENT_SUBCODE_MAX_TRANSACTIONS_EXCEEDED                      = 126, /**< The client failed to open a Transacted Session as the maximum number of transactions was exceeded.*/
    SOLCLIENT_SUBCODE_COMMIT_STATUS_UNKNOWN                          = 127, /**< The commit response was lost due to a transport layer reconnection to an alternate host in the host list. */
    SOLCLIENT_SUBCODE_PROXY_AUTH_REQUIRED                            = 128, /**< The host entry did not contain proxy authentication when required by the proxy server. */
    SOLCLIENT_SUBCODE_PROXY_AUTH_FAILURE                             = 129, /**< The host entry contained invalid proxy authentication when required by the proxy server. */
    SOLCLIENT_SUBCODE_NO_SUBSCRIPTION_MATCH                          = 130, /**< The client attempted to publish a guaranteed message to a topic that did not have any guaranteed subscription matches or only matched a replicated topic.*/
    SOLCLIENT_SUBCODE_SUBSCRIPTION_MATCH_ERROR                       = 131, /**< The client attempted to bind to a non-exclusive topic endpoint that is already bound with a different subscription.*/
    SOLCLIENT_SUBCODE_SELECTOR_MATCH_ERROR                           = 132, /**< The client attempted to bind to a non-exclusive topic endpoint that is already bound with
    a different ingress selector.*/
    SOLCLIENT_SUBCODE_REPLAY_NOT_SUPPORTED                           = 133, /**< Replay is not supported on the Solace Message Router. */
    SOLCLIENT_SUBCODE_REPLAY_DISABLED                                = 134, /**< Replay is not enabled in the message-vpn. */
    SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_NON_EXCLUSIVE_NOT_ALLOWED = 135, /**< The client attempted to start replay on a flow bound to a non-exclusive endpoint. */
    SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_INACTIVE_FLOW_NOT_ALLOWED = 136, /**< The client attempted to start replay on an inactive flow. */
    SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_BROWSER_FLOW_NOT_ALLOWED = 137, /**< The client attempted to bind with both ::SOLCLIENT_FLOW_PROP_BROWSER enabled and ::SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION set. */
    SOLCLIENT_SUBCODE_REPLAY_TEMPORARY_NOT_SUPPORTED                 = 138, /**< Replay is not supported on temporary endpoints. */
    SOLCLIENT_SUBCODE_UNKNOWN_START_LOCATION_TYPE                    = 139, /**< The request attempted to start a replay but provided an unknown start location type. */
    SOLCLIENT_SUBCODE_REPLAY_MESSAGE_UNAVAILABLE                     = 140, /**< A replay in progress on a flow failed because messages to be replayed were trimmed from the replay log. */
    SOLCLIENT_SUBCODE_REPLAY_STARTED                                 = 141, /**< A replay was started on the queue/topic endpoint, either by another client or by an adminstrator on the message router. */
    SOLCLIENT_SUBCODE_REPLAY_CANCELLED                               = 142, /**< A replay in progress on a flow was administratively cancelled, causing the flow to be unbound. */
    SOLCLIENT_SUBCODE_REPLAY_START_TIME_NOT_AVAILABLE                = 143, /**< A replay was requested but the requested start time is not available in the replay log. */
    SOLCLIENT_SUBCODE_REPLAY_MESSAGE_REJECTED                        = 144, /**< The Solace Message Router attempted to replay a message, but the queue/topic endpoint rejected the message to the sender. */
    SOLCLIENT_SUBCODE_REPLAY_LOG_MODIFIED                            = 145, /**< A replay in progress on a flow failed because the replay log was modified. */
    SOLCLIENT_SUBCODE_MISMATCHED_ENDPOINT_ERROR_ID                   = 146, /**< Endpoint error ID in the bind request does not match the endpoint's error ID. */
    SOLCLIENT_SUBCODE_OUT_OF_REPLAY_RESOURCES                        = 147, /**< A replay was requested, but the router does not have sufficient resources to fulfill the request, due to too many active replays. */
    SOLCLIENT_SUBCODE_TOPIC_OR_SELECTOR_MODIFIED_ON_DURABLE_TOPIC_ENDPOINT = 148, /**< A replay was in progress on a Durable Topic Endpoint (DTE) when its topic or selector was modified, causing the replay to fail. */
    SOLCLIENT_SUBCODE_REPLAY_FAILED                                  = 149, /**< A replay in progress on a flow failed. */
    SOLCLIENT_SUBCODE_COMPRESSED_SSL_NOT_SUPPORTED                   = 150, /**< The client attempted to establish a Session or Flow with ssl and compression, but the capability is not supported by the appliance.*/
    /*
     * ADDING NEW SUBCODES: When adding a new subcode always add a new entry to the HTML table in 
     * the comment above this enumeration 
     */
  } solClient_subCode_t;                                      /**< Type for API sub codes. */

/**
* @enum solClient_log_level
* Definition of SolClient log levels.
* To avoid affecting performance, do not set the API log filter level to
* SOLCLIENT_LOG_INFO or SOLCLIENT_LOG_DEBUG unless required for debugging.
* A log level can be converted to a string through solClient_log_levelToString().
*/
  typedef enum solClient_log_level
  {
    SOLCLIENT_LOG_EMERGENCY = 0, /**< This level is not used by the API. */
    SOLCLIENT_LOG_ALERT = 1,     /**< This level is not used by the API. */
    SOLCLIENT_LOG_CRITICAL = 2,  /**< A serious error that can make the API unusable. */
    SOLCLIENT_LOG_ERROR = 3,     /**< An unexpected condition within the API that can affect its operation. */
    SOLCLIENT_LOG_WARNING = 4,   /**< An unexpected condition within the API that is not expected to affect its operation. */
    SOLCLIENT_LOG_NOTICE = 5,    /**< Significant informational messages about the normal operation of the API. These messages are never output in the normal process of sending or receiving a message from the appliance. */
    SOLCLIENT_LOG_INFO = 6,      /**< Informational messages about the normal operation of the API. These might include information related to sending or receiving messages from the appliance. */
    SOLCLIENT_LOG_DEBUG = 7      /**< Debugging information generally useful to API developers (very verbose). */
  } solClient_log_level_t;       /**< Type for log levels. */

/** @name Default Log Filter Level
* Definition of default log level that should be used for normal operation. This default
* log filter level can be passed to solClient_initialize() and solClient_log_setFilterLevel().
*/
/*@{*/
#define SOLCLIENT_LOG_DEFAULT_FILTER (SOLCLIENT_LOG_NOTICE) /**< Default log filter level. */
/*@}*/

/**
* @enum solClient_log_category
* Definition of SolClient log categories, which is used for log level filter control.
* Each category can have its log level set independently.
* SOLCLIENT_LOG_CATEGORY_SDK refers to logs which are generated internally within the C
* API. SOLCLIENT_LOG_CATEGORY_APP refers to logs raised by the application through
* solClient_log().
* A log category can be converted to a string through solClient_log_categoryToString().
*/
  typedef enum solClient_log_category
  {
    SOLCLIENT_LOG_CATEGORY_ALL = 0, /**< Set log level filter for all categories. */
    SOLCLIENT_LOG_CATEGORY_SDK = 1, /**< Set log level filter for API internal logs. */
    SOLCLIENT_LOG_CATEGORY_APP = 2  /**< Set log level filter for application logs. */
  } solClient_log_category_t;       /**< Type for log filter categories. */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/* Only for use by solClient_log() and solClient_log_va_list() */
  solClient_dllExport extern solClient_log_level_t _solClient_log_appFilterLevel_g;     /* the GLOBAL log filter level for app logs; applies to all Sessions */

/* NEVER call directly; always use solClient_log() */
  solClient_dllExport void
    _solClient_log_output_detail(
        solClient_log_category_t  category,
        solClient_log_level_t     level,
        const char               *filename_p,
        int                       lineNum,
        const char               *format_p, ...);

/* NEVER call directly; always use solClient_log_va_list() */
  solClient_dllExport void
    _solClient_log_output_detail_va_list(
        solClient_log_category_t category,
        solClient_log_level_t    level,
        const char              *filename_p,
        int                      lineNum,
        const char              *format_p,
        va_list                  ap);
#endif

#ifdef WIN32
  typedef SOCKET solClient_fd_t;    /**< Type for a file descriptor. */
#else
  typedef int solClient_fd_t;       /**< Type for a file descriptor. */
#endif

  typedef solClient_uint32_t solClient_fdEvent_t;              /**< A mask of events that can be requested for a file descriptor. */
  typedef solClient_uint32_t solClient_subscribeFlags_t;       /**< A set of \ref subscribeflags "flags"  that can be provided to solClient_session_topicSubscribeExt() and solClient_session_topicUnsubscribeExt(). */
  typedef solClient_uint32_t solClient_session_responseCode_t; /**< An error response code that is returned with Session events. */
  typedef solClient_uint64_t solClient_msgId_t;                /**< A unique msgId assigned to each Persistent and Non-Persistent message. */
  typedef solClient_uint32_t solClient_modifyPropFlags_t;      /**< A set of \ref modifypropflags "flags" that can be provided to a solClient_session_modifyClientInfo() call. */

/** @name File Descriptor Event Types
* Events that can be registered or unregistered for a file descriptor, through 
* solClient_context_registerForFdEvents() and
* solClient_context_unregisterForFdEvents(), and returned when
* the events occur.
* Events can be ORed together.
*/
/*@{*/
#define SOLCLIENT_FD_EVENT_READ  (0x01)  /**< A readable file descriptor event. */
#define SOLCLIENT_FD_EVENT_WRITE (0x02)  /**< A writeable file descriptor event. */
#define SOLCLIENT_FD_EVENT_ALL   (0x03)  /**< This type represents all defined file descriptor events. */
/*@}*/

/**
* @name Class of Service Value Types
* Values that can be used to set the class of service of a message using
* solClient_msg_setClassOfService()
*/
/*@{*/
#define SOLCLIENT_COS_1          (0x00)  /**< The lowest class of service value. For guaranteed messaging, it means the low priority.*/
#define SOLCLIENT_COS_2          (0x01)  /**< The middle class of service value. For guaranteed messaging, it is reserved for future use and is treated as the low priority.*/
#define SOLCLIENT_COS_3          (0x02)  /**< The highest class of service value. For guaranteed messaging, it means the high priority.*/

/**
* @name Delivery Mode Types
* Values that can be used to set the delivery mode of a message using
* solClient_msg_setDeliveryMode()
*/
/*@{*/
#define SOLCLIENT_DELIVERY_MODE_DIRECT         (0x00)  /**< Send a Direct message. */
#define SOLCLIENT_DELIVERY_MODE_PERSISTENT     (0x10)  /**< Send a Persistent message. */
#define SOLCLIENT_DELIVERY_MODE_NONPERSISTENT  (0x20)  /**< Send a Non-Persistent message. */
/*@}*/

/**
 * @anchor subscribeflags
 * @name Subscriber Flag Types
 * Values that can be used as part of the flags field to solClient_session_topicSubscribeExt()
 * and solClient_session_topicUnsubscribeExt().
 */
#define SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM        (0x02) /**< The subscribe/unsubscribe call blocks until a confirmation is received. @see @ref blocking-context "Threading Effects on Blocking Modes" for more information about setting subscribe flags in the Context thread.*/
#define SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE (0x04) /**< This flag, when present in a subscription ADD request, overrides the deliver-to-one property in a message (see ::solClient_msg_setDeliverToOne()) - If the Topic in the message matches, it is delivered to clients with ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE set, in addition to any one client that is subscribed to the Topic without this override. */
#define SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY   (0x08) /**< For the @ref topic-dispatch "topic dispatch" feature, this flag indicates the subscription should only be added to the dispatch table and should not be added to the appliance. */
#define SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM       (0x10) /**< Requests a confirmation for the subscribe/unsubscribe operation. This bit is implied by ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. If ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM is not set when this flag is set, then a confirmation event will be issued through the Session event callback procedure. */
/*@}*/


/**
 * @anchor modifypropflags
 * @name Certain session property modification Flag Types
 * Values that can be used as part of the flags field to solClient_session_modifyClientInfo() function.
 */
#define SOLCLIENT_MODIFYPROP_FLAGS_WAITFORCONFIRM        (0x01) /**< The modifyClientInfo API call blocks until a confirmation is received or its associated timer ::SOLCLIENT_SESSION_PROP_MODIFYPROP_TIMEOUT_MS expires. @see @ref blocking-context "Threading Effects on Blocking Modes" for more information about setting session property modification flags in the Context thread.*/
/*@}*/

/**
* @struct solClient_uuid
*
* A structure used to hold a UUID (Universally Unique Identifier), as per IETF RFC 4122. The structure
* is 128 bits. A UUID can be generated through solClient_generateUUID(). A string form of a UUID
* can be generated through solClient_generateUUIDString().
*/
typedef struct solClient_uuid
{
    solClient_uint32_t   timeLow;               /**< As defined in IETF RFC 4122. */
    solClient_uint16_t   timeMid;               /**< As defined in IETF RFC 4122. */
    solClient_uint16_t   timeHiAndVersion;      /**< As defined in IETF RFC 4122. */
    solClient_uint8_t    clockSeqHiAndReserved; /**< As defined in IETF RFC 4122. */
    solClient_uint8_t    clockSeqLow;           /**< As defined in IETF RFC 4122. */
    solClient_uint8_t    node[6];               /**< As defined in IETF RFC 4122. */
} solClient_uuid_t;

/** @name UUID (Universally Unique Identifier) String Buffer Size
* The size (in bytes) of the buffer required by solClient_generateUUIDString()
* to hold a UUID value in a string representation as per IETF RFC 4122, including the terminating
* NULL character.
*/
/*@{*/
#define SOLCLIENT_UUID_STRING_BUFFER_SIZE (37) /**< The size in bytes for string representation of UUID value. */
/*@}*/

/*@{*/
/** @enum solClient_session_event
 * Session events that can be given to the Session event callback routine registered for
 * a Session. The Session event callback is registered when a Session is created 
 * through solClient_session_create(), and it has the prototype ::solClient_session_eventCallbackFunc_t. 
 * A Session event can be converted to a string value through solClient_session_eventToString().
 */
  typedef enum solClient_session_event
  {
    SOLCLIENT_SESSION_EVENT_UP_NOTICE = 0,              /**< The Session is established. */
    SOLCLIENT_SESSION_EVENT_DOWN_ERROR = 1,             /**< The Session was established and then went down. */
    SOLCLIENT_SESSION_EVENT_CONNECT_FAILED_ERROR = 2,   /**< The Session attempted to connect but was unsuccessful. */
    SOLCLIENT_SESSION_EVENT_REJECTED_MSG_ERROR = 3,     /**< The appliance rejected a published message. */
    SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR = 4,     /**< The appliance rejected a subscription (add or remove). */
    SOLCLIENT_SESSION_EVENT_RX_MSG_TOO_BIG_ERROR = 5,   /**< The API discarded a received message that exceeded the Session buffer size. */
    SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT = 6,        /**< The oldest transmitted Persistent/Non-Persistent message that has been acknowledged. */
    SOLCLIENT_SESSION_EVENT_ASSURED_PUBLISHING_UP = 7,  /**< Deprecated -- see notes in solClient_session_startAssuredPublishing. The AD Handshake (that is, Guaranteed Delivery handshake) has completed for the publisher and Guaranteed messages can be sent. */
    SOLCLIENT_SESSION_EVENT_ASSURED_CONNECT_FAILED = 8, /**< Deprecated -- see notes in solClient_session_startAssuredPublishing. The appliance rejected the AD Handshake to start Guaranteed publishing. Use ::SOLCLIENT_SESSION_EVENT_ASSURED_DELIVERY_DOWN instead. */
    SOLCLIENT_SESSION_EVENT_ASSURED_DELIVERY_DOWN = 8,  /**< Guaranteed Delivery publishing is not available. The guaranteed delivery capability on the session has been disabled by some action on the appliance. */
    SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_ERROR = 9,   /**< The Topic Endpoint unsubscribe command failed. */
    SOLCLIENT_SESSION_EVENT_DTE_UNSUBSCRIBE_ERROR = SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_ERROR,  /**< Deprecated name; ::SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_ERROR is preferred */
    SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_OK = 10,     /**< The Topic Endpoint unsubscribe completed. */
    SOLCLIENT_SESSION_EVENT_DTE_UNSUBSCRIBE_OK = SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_OK,    /**< Deprecated name; ::SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_OK is preferred */
    SOLCLIENT_SESSION_EVENT_CAN_SEND =            11,   /**< The send is no longer blocked. */
    SOLCLIENT_SESSION_EVENT_RECONNECTING_NOTICE = 12,   /**< The Session has gone down, and an automatic reconnect attempt is in progress. */
    SOLCLIENT_SESSION_EVENT_RECONNECTED_NOTICE =  13,   /**< The automatic reconnect of the Session was successful, and the Session was established again. */
    SOLCLIENT_SESSION_EVENT_PROVISION_ERROR =     14,   /**< The endpoint create/delete command failed. */
    SOLCLIENT_SESSION_EVENT_PROVISION_OK    =     15,   /**< The endpoint create/delete command completed. */
    SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_OK =     16,   /**< The subscribe or unsubscribe operation has succeeded. */
    SOLCLIENT_SESSION_EVENT_VIRTUAL_ROUTER_NAME_CHANGED = 17, /**< The appliance's Virtual Router Name changed during a reconnect operation. This could render existing queues or temporary topics invalid. */
    SOLCLIENT_SESSION_EVENT_MODIFYPROP_OK   =     18,   /**< The session property modification completed. */
    SOLCLIENT_SESSION_EVENT_MODIFYPROP_FAIL   =   19,   /**< The session property modification failed. */
    SOLCLIENT_SESSION_EVENT_REPUBLISH_UNACKED_MESSAGES = 20  /**< After successfully reconnecting a disconnected session, the SDK received an unknown publisher flow name response when reconnecting the GD publisher flow. If configured to auto-retry (See ::SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION.) this event is generated to indicate how many unacknowledged messages are retransmitted on success. As the publisher state has been lost on failover, receiving this event may indicate that some messages have been duplicated in the system.*/
  } solClient_session_event_t;                          /**< Type for Session events. */

/*@{*/
/** @enum solClient_flow_event
 * Flow events that can be given to the Flow event callback routine registered for
 * a Flow. The Flow event callback is registered when a Flow is created through 
 * solClient_session_createFlow() and has the prototype ::solClient_flow_eventCallbackFunc_t. 
 * A Flow event can be converted to a string value through solClient_flow_eventToString().
 */
  typedef enum solClient_flow_event
  {
    SOLCLIENT_FLOW_EVENT_UP_NOTICE = 0,            /**< The Flow is established. */
    SOLCLIENT_FLOW_EVENT_DOWN_ERROR = 1,           /**< The Flow was established and then disconnected by the appliance, likely due to operator intervention. The Flow must be destroyed. */
    SOLCLIENT_FLOW_EVENT_BIND_FAILED_ERROR = 2,    /**< The Flow attempted to connect but was unsuccessful. */
    SOLCLIENT_FLOW_EVENT_REJECTED_MSG_ERROR = 3,   /**< This event is deprecated and will never be raised. */
    SOLCLIENT_FLOW_EVENT_SESSION_DOWN       = 4,   /**< The Session for the Flow was disconnected. The Flow will rebound automatically when the Session is reconnected.*/
    SOLCLIENT_FLOW_EVENT_ACTIVE             = 5,   /**< The flow has become active */
    SOLCLIENT_FLOW_EVENT_INACTIVE           = 6    /**< The flow has become inactive */
  } solClient_flow_event_t;                        /**< Type for Flow events. */

/** 
 *@name Acknowledgment Event Mode
 * It specifies if a session event ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT 
 * acknowledges a single message or a range of messages. 
 */
/*@{*/
#define SOLCLIENT_SESSION_PROP_ACK_EVENT_MODE_PER_MSG     "SESSION_ACK_EVENT_MODE_PER_MSG"   /**< In this mode,  a session event ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT acknowledges a single message.*/
#define SOLCLIENT_SESSION_PROP_ACK_EVENT_MODE_WINDOWED    "SESSION_ACK_EVENT_MODE_WINDOWED"  /**< In this mode, a session event ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT is a ranged acknowledgment. It acknowledges the message received as well as all preceding messages. */
/*@}*/

/** @name Configuration Properties Enable/Disable
* For configuration property items that are on/off in nature,
* the value "1" is used to enable the property, and "0" is used to
* disable the property. Any non-zero value is converted to "1" for
* such properties.
*/
/*@{*/
#define SOLCLIENT_PROP_ENABLE_VAL  "1" /**< The value used to enable the property. */
#define SOLCLIENT_PROP_DISABLE_VAL "0" /**< The value used to disable the property. */
/*@}*/

/** @anchor globalProps
 * @name Global Configuration Properties 
 * Items that can be configured globally for an API instance.  Global properties are set in
 * solClient_initialize().  Global properties may not be changed after this, they exist for 
 * the duration of the API instance.
 *
 * Note that the data buffer sizes for quantas 0 through 4 must be in increasing order of size.
 */
/*@{*/
#define SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_0 "GLOBAL_DBQUANTA_SIZE_0" /**< \ref globalProps "The size (in bytes) of data buffers in the pool of smallest buffers." The valid range is > 0. Default: ::SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_0 */
#define SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_1 "GLOBAL_DBQUANTA_SIZE_1" /**< \ref globalProps "The size (in bytes) of data buffers in the second pool of buffers." The valid range is > 0. Default: ::SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_1 */
#define SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_2 "GLOBAL_DBQUANTA_SIZE_2" /**< \ref globalProps "The size (in bytes) of data buffers in the third pool of buffers." The valid range is > 0. Default: ::SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_2 */
#define SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_3 "GLOBAL_DBQUANTA_SIZE_3" /**< \ref globalProps "The size (in bytes) of data buffers in the fourth pool of buffers." The valid range is > 0. Default: ::SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_3 */
#define SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_4 "GLOBAL_DBQUANTA_SIZE_4" /**< \ref globalProps "The size (in bytes) of data buffers in the fifth (last) pool of buffers." The valid range is > 0. Default: ::SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_4 */
#define SOLCLIENT_GLOBAL_PROP_MAXPOOLMEM     "GLOBAL_MAXPOOLMEM"      /**< \ref globalProps "The maximum amount of memory (in bytes) the API can save in its data and message pools." Once it reaches this size, data blocks are released back to the heap and are not kept in a API pool. */
#define SOLCLIENT_GLOBAL_PROP_GSS_KRB_LIB "GLOBAL_GSS_KRB_LIB"  /**< \ref globalProps The GSS Kerberos library name. 
      Default: \n
      @li ::SOLCLIENT_GLOBAL_PROP_DEFAULT_GSS_KRB_LIB_LINUX for Linux.
      @li ::SOLCLIENT_GLOBAL_PROP_DEFAULT_GSS_KRB_LIB_SOLARIS for Solaris.
      @li ::SOLCLIENT_GLOBAL_PROP_DEFAULT_GSS_KRB_LIB_WINDOWS for Windows.*/
#define SOLCLIENT_GLOBAL_PROP_IBM_CODESET "GLOBAL_IBM_CODESET" /**< Only valid on the z/TPF mainframe. This selects the EBCDIC character set in use by the application. Default: ::SOLCLIENT_GLOBAL_PROP_DEFAULT_IBM_CODESET. */
  /*@}*/

/** @name Default global configuration properties
* The default values for global configuration properties that are not explicitly set.
*/
/*@{*/
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_0   "10240"      /**< The default size (10 KB) of data buffers in the pool of smallest buffers. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_1   "32768"      /**< The default size (32 KB) of data buffers in the second pool of buffers. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_2   "65536"      /**< The default size (64 KB) of data buffers in the third pool of buffers. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_3   "262144"     /**< The default size (256 KB) of data buffers in the 4th pool of buffers. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_DBQUANTASIZE_4   "1048576"    /**< The default size (1 MB) of data buffers in the 5th (last) pool of buffers. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_MAXPOOLMEM       "1073741824" /**< The default maximum memory pool size (1 GB). */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_GSS_KRB_LIB_LINUX "libgssapi_krb5.so.2" /**< The default GSS Kerberos library name for Linux. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_GSS_KRB_LIB_SOLARIS "mech_krb5.so.1"    /**< The default GSS Kerberos library name for Solaris. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_GSS_KRB_LIB_WINDOWS "secur32.dll"       /**< The default GSS Kerberos library name for Windows. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_GSS_KRB_LIB_AIX "libgssapi_krb5.a(libgssapi_krb5.a.so)" /**< The default GSS Kerberos library name for AIX. */
#define SOLCLIENT_GLOBAL_PROP_DEFAULT_IBM_CODESET       "TPF_CCSID_IBM1047" /**< The default IBM character set in use by the application */

/*@}*/

/** @defgroup ContextProps Context Configuration Properties
* Items that can be configured for a Context.
*/
/*@{*/
#define SOLCLIENT_CONTEXT_PROP_TIME_RES_MS    "CONTEXT_TIME_RES_MS"    /**< The internal timer resolution (in milliseconds). Valid range is >= 10 and <= 10000. Default:  ::SOLCLIENT_CONTEXT_PROP_DEFAULT_TIME_RES_MS */
#define SOLCLIENT_CONTEXT_PROP_CREATE_THREAD  "CONTEXT_CREATE_THREAD"  /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to have the Context thread created automatically (as opposed to the application creating and destroying this thread). Default: ::SOLCLIENT_CONTEXT_PROP_DEFAULT_CREATE_THREAD */
#define SOLCLIENT_CONTEXT_PROP_THREAD_AFFINITY  "CONTEXT_THREAD_AFFINITY"  /**< The desired thread affinity mask for the Context thread. A thread affinity mask is a bit vector in which each bit represents a logical processor that a thread is allowed to run on. Only used if the Context thread is automatically created. For Solaris, only 1 bit can be set (that is, the thread will run on only one processor). Not supported on AIX. A value of zero means the affinity is not set, and the parent's affinity is used. A value of 1 means use processor 0, a value of 2 means use processor 1, a value of 3 means use both processor 0 and 1, and so on. Default: ::SOLCLIENT_CONTEXT_PROP_DEFAULT_THREAD_AFFINITY */
/*@}*/

/** @defgroup DefaultContextProps Default Context Configuration Properties
* Default values for Context configuration properties that are not explicitly set.
*/
/*@{*/
#define SOLCLIENT_CONTEXT_PROP_DEFAULT_TIME_RES_MS    "50"  /**< The default value for timer resolution (in milliseconds). */
#define SOLCLIENT_CONTEXT_PROP_DEFAULT_CREATE_THREAD  SOLCLIENT_PROP_DISABLE_VAL /**< The default value for create Context thread. By default the thread is created and destroyed by the application. */
#define SOLCLIENT_CONTEXT_PROP_DEFAULT_THREAD_AFFINITY  "0"  /**< By default, the thread affinity for the auto-created Context thread is not set. */
/*@}*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
solClient_dllExport extern const char *_solClient_contextPropsDefaultWithCreateThread[]; /* Do not use directly; use SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD */
#endif

/** @name Context properties with Context thread created automatically and all other properties at default values.
* A convenience set of Context properties for use with ::solClient_context_create() that specify that the
* Context thread should be created automatically. All other Context properties are the default values, (that is, 
* {SOLCLIENT_CONTEXT_PROP_CREATE_THREAD, SOLCLIENT_PROP_ENABLE_VAL, NULL}).
*/
/*@{*/
#define SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD ((solClient_propertyArray_pt )_solClient_contextPropsDefaultWithCreateThread) /**< Use with ::solClient_context_create() to create a Context in which the automatic Context thread is automatically created and all other properties are set with default values. */
/*@}*/

/** @defgroup SessionProps Session Configuration Properties
* Items that can be configured for a Session.  
*
* Some Session Properties can also be set as environment variables.  If a session property can be set as an environment variable, the 
* API will only look for that environment variable if the property is not specified in the sesssion property list passed to
* ::solClient_session_create.
*
* The following environment variables are recognized:
* @li ::SOLCLIENT_SESSION_PROP_PASSWORD
* @li ::SOLCLIENT_SESSION_PROP_HOST
* @li ::SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION
*/
/*@{*/
#define SOLCLIENT_SESSION_PROP_USERNAME                      "SESSION_USERNAME" /**< The username required for authentication. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_USERNAME */
#define SOLCLIENT_SESSION_PROP_PASSWORD                      "SESSION_PASSWORD" /**< The password required for authentication. May be set as an environment variable (See @ref SessionProps). Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_PASSWORD */
#define SOLCLIENT_SESSION_PROP_HOST                          "SESSION_HOST"     /**< The IP address (or host name) to connect to. @ref host-list "Multiple entries" (up to ::SOLCLIENT_SESSION_PROP_MAX_HOSTS) are allowed, separated by commas. @ref host-entry "The entry for the SOLCLIENT_SESSION_PROP_HOST property should provide a protocol, host, and port". See @ref host-list "Configuring Multiple Hosts for Redundancy and Failover" for a discussion of Guaranteed Messaging considerations. May be set as an environment variable (See @ref SessionProps). Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_HOST */
#define SOLCLIENT_SESSION_PROP_PORT                          "SESSION_PORT"     /**< Deprecated. While still supported, the port number can also now be specified as part of the host in ::SOLCLIENT_SESSION_PROP_HOST (for example, "hostname:55555"). In general, port numbers are not needed except in special situations, because the API chooses the correct port to connect to the appliance. If ::SOLCLIENT_SESSION_PROP_PORT is set, this port number is used for all entries in ::SOLCLIENT_SESSION_PROP_HOST that do not explicitly specify port. The port number to connect to. The valid range is 1..65535. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_PORT or ::SOLCLIENT_SESSION_PROP_DEFAULT_PORT_COMPRESSION, based on setting of ::SOLCLIENT_SESSION_PROP_COMPRESSION_LEVEL */
#define SOLCLIENT_SESSION_PROP_BUFFER_SIZE                   "SESSION_BUFFER_SIZE" /**< The maximum amount of messages to buffer (in bytes) when the TCP session is flow controlled (see \ref message-buffer "Message Buffer Size Configuration"). The valid range is > 0. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_BUFFER_SIZE */
#define SOLCLIENT_SESSION_PROP_CONNECT_BLOCKING              "SESSION_CONNECT_BLOCKING" /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to enable blocking connect operation. A blocking connect operation suspends until the Session is successfully connected, including restoring all remembered subscriptions if ::SOLCLIENT_SESSION_PROP_REAPPLY_SUBSCRIPTIONS is enabled. Otherwise solClient_session_connect() returns SOLCLIENT_IN_PROGRESS. @see @ref blocking-context "Threading Effects on Blocking Modes" for a discussion of blocking operation in the Context thread. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_CONNECT_BLOCKING */
#define SOLCLIENT_SESSION_PROP_SEND_BLOCKING                 "SESSION_SEND_BLOCKING" /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to enable blocking send operation. A blocking send operation suspends when the Session is transport flow controlled, otherwise the send operation returns SOLCLIENT_WOULD_BLOCK. Successful return from a blocking send operation only means the message has been accepted by the transport, it does not guarantee the message has been processed by the appliance. For the latter you must used Guaranteed Message Delivery mode and wait for the session event (::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT)  that acknowledges the message. @see @ref blocking-context "Threading Effects on Blocking Modes" for a discussion of blocking operation in the Context thread. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SEND_BLOCKING */
#define SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING            "SESSION_SUBSCRIBE_BLOCKING" /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to enable blocking subscribe/unsubscribe operation. A blocking subscribe operation will suspend when the Session is transport flow controlled, otherwise the subscribe operation returns SOLCLIENT_WOULD_BLOCK. A successful return from a blocking subscribe operation only means the subscription has been accepted by the transport, it does not guarantee the subscription has been processed by the appliance. For the latter you must use a confirmed operation (see ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM)  @see @ref blocking-context "Threading Effects on Blocking Modes" for a discussion of blocking operation in the Context thread. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SUBSCRIBE_BLOCKING */
#define SOLCLIENT_SESSION_PROP_BLOCK_WHILE_CONNECTING        "SESSION_BLOCK_WHILE_CONNECTING" /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to block the calling thread on operations such as sending a message, subscribing, or unsubscribing when the Session is being connected or reconnected. The operation must already be blocking (see ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING and ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING). Otherwise, ::SOLCLIENT_NOT_READY is returned if the Session is being connected. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_BLOCK_WHILE_CONNECTING */ 
#define SOLCLIENT_SESSION_PROP_BLOCKING_WRITE_TIMEOUT_MS     "SESSION_WRITE_TIMEOUT_MS" /**< The timeout period (in milliseconds) for blocking write operation. The valid range is > 0. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_BLOCKING_WRITE_TIMEOUT_MS */
#define SOLCLIENT_SESSION_PROP_CONNECT_TIMEOUT_MS            "SESSION_CONNECT_TIMEOUT_MS" /**< The timeout period (in milliseconds) for a connect operation to a given host (per host). The valid range is > 0. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_CONNECT_TIMEOUT_MS */
#define SOLCLIENT_SESSION_PROP_SUBCONFIRM_TIMEOUT_MS         "SESSION_SUBCONFIRM_TIMEOUT_MS" /**< The timeout period (in milliseconds) for subscription confirm (add or remove). The valid range is >= 1000. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SUBCONFIRM_TIMEOUT_MS */
#define SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR "SESSION_IGNORE_DUP_SUBSCRIPTION_ERROR" /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to ignore errors for duplicate subscription/topic on subscribe or subscription not found errors on unsubscribe. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_IGNORE_DUP_SUBSCRIPTION_ERROR */
#define SOLCLIENT_SESSION_PROP_TCP_NODELAY                   "SESSION_TCP_NODELAY" /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to enable TCP no delay. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_TCP_NODELAY */
#define SOLCLIENT_SESSION_PROP_SOCKET_SEND_BUF_SIZE          "SESSION_SOCKET_SEND_BUF_SIZE" /**< The value for the socket send buffer size (in bytes). 0 indicates do not set and leave at operating system default. The valid range is 0 or >= 1024. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SOCKET_SEND_BUF_SIZE.
*
* Note that Linux operating system actually allocates twice the size of the buffer requested in the setsockopt(2) call, and so a succeeding getsockopt(2) call will not return the same size of buffer as requested in the setsockopt(2) call. TCP uses the extra space for administrative purposes and internal kernel structures, and the sysctl variables reflect the larger sizes compared to the actual TCP windows.
 */
#define SOLCLIENT_SESSION_PROP_SOCKET_RCV_BUF_SIZE           "SESSION_SOCKET_RCV_BUF_SIZE" /**< The value for socket receive buffer size  (in bytes). 0 indicates do not set and leave at operating system default. The valid range is 0 or >= 1024. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SOCKET_RCV_BUF_SIZE  */
#define SOLCLIENT_SESSION_PROP_KEEP_ALIVE_INT_MS             "SESSION_KEEP_ALIVE_INTERVAL_MS" /**< The amount of time (in milliseconds) to wait between sending out Keep-Alive messages. Typically, this feature should be enabled for message receivers. Use 0 to disable Keep-Alives (0 is required before appliance release 4.2). The valid range is 0 (disabled) or >= 50. Default:  ::SOLCLIENT_SESSION_PROP_DEFAULT_KEEP_ALIVE_INT_MS */
#define SOLCLIENT_SESSION_PROP_KEEP_ALIVE_LIMIT              "SESSION_KEEP_ALIVE_LIMIT" /**< The maximum number of consecutive Keep-Alive messages that can be sent without receiving a response before the connection is closed by the API. The valid range is >= 3. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_KEEP_ALIVE_LIMIT */
#define SOLCLIENT_SESSION_PROP_APPLICATION_DESCRIPTION       "SESSION_APPLICATION_DESCRIPTION" /**< A string that uniquely describes the application instance. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_APPLICATION_DESCRIPTION */
#define SOLCLIENT_SESSION_PROP_CLIENT_MODE                   "SESSION_CLIENT_MODE" /**< Deprecated. The CCSMP API detects the appliance capabilities, so it is no longer necessary to specify to use 'clientMode' or not. This property is ignored when specified. */
#define SOLCLIENT_SESSION_PROP_BIND_IP                       "SESSION_BIND_IP" /**< (Optional) The hostname or IP address of the machine on which the application is running. On a multihomed machine, it is strongly recommended to provide this parameter to ensure that the API uses the correct network interface at Session connect time. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_BIND_IP */
#define SOLCLIENT_SESSION_PROP_PUB_WINDOW_SIZE               "SESSION_PUB_WINDOW_SIZE" /**< The publisher window size for Guaranteed messages. The Guaranteed Message Publish Window Size property limits the maximum number of messages that can be published before the API must receive an acknowledgment from the appliance. The valid range is 1..255, or 0 to disable publishing Guaranteed messages. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_PUB_WINDOW_SIZE */
#define SOLCLIENT_SESSION_PROP_PUB_ACK_TIMER                 "SESSION_PUB_ACK_TIMER"    /**< The duration of publisher acknowledgment timer (in milliseconds). When a published message is not acknowledged within the time specified for this timer, the API automatically retransmits the message. There is no limit on the number of retransmissions for any message. However, while the API is resending, applications can become flow controlled. The flow control behavior is controlled by ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING and ::SOLCLIENT_SESSION_PROP_BLOCKING_WRITE_TIMEOUT_MS. The valid range is 20..60000. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_PUB_ACK_TIMER */
#define SOLCLIENT_SESSION_PROP_VPN_NAME                      "SESSION_VPN_NAME"    /**< The name of the Message VPN to attempt to join when connecting to an appliance running SolOS-TR. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_VPN_NAME */
#define SOLCLIENT_SESSION_PROP_VPN_NAME_IN_USE               "SESSION_VPN_NAME_IN_USE"    /**< A read-only Session property that indicates which Message VPN the Session is connected to. When not connected, an empty string is returned. */
#define SOLCLIENT_SESSION_PROP_CLIENT_NAME                   "SESSION_CLIENT_NAME" /**< The Session client name that is used during client login to create a unique Session. An empty string causes a unique client name to be generated automatically. If specified, it must be a valid Topic name, and a maximum of 160 bytes in length. For all appliances (SolOS-TR or SolOS-CR) the SOLCLIENT_SESSION_PROP_CLIENT_NAME is also used to uniquely identify the sender in a message's senderId field if ::SOLCLIENT_SESSION_PROP_GENERATE_SENDER_ID is set. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_CLIENT_NAME */
#define SOLCLIENT_SESSION_PROP_SUBSCRIBER_LOCAL_PRIORITY     "SESSION_SUBSCRIBER_LOCAL_PRIORITY" /**< Subscriber priorities are used to choose a client to receive messages that are sent with the ::SOLCLIENT_SEND_FLAGS_DELIVER_TO_ONE property set. These messages are sent to the subscriber with the highest priority. Subscribers have two priorities; this priority is for messages published locally. The valid range is 1..4. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SUBSCRIBER_LOCAL_PRIORITY */
#define SOLCLIENT_SESSION_PROP_SUBSCRIBER_NETWORK_PRIORITY   "SESSION_SUBSCRIBER_NETWORK_PRIORITY" /**< Subscriber priorities are used to choose a client to receive messages that are sent with the ::SOLCLIENT_SEND_FLAGS_DELIVER_TO_ONE property set. These messages are sent to the subscriber with the highest priority. Subscribers have two priorities; this priority is for messages published on appliances other than the one that the client is connected to. The valid range is 1..4. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SUBSCRIBER_NETWORK_PRIORITY */
#define SOLCLIENT_SESSION_PROP_COMPRESSION_LEVEL             "SESSION_COMPRESSION_LEVEL"  /**< Enables messages to be compressed with ZLIB before transmission and decompressed on receive. The valid range is 0 (off) or 1..9, where 1 is less compression (fastest) and 9 is most compression (slowest). Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_COMPRESSION_LEVEL

Note: If no port is specified in the SESSION_HOST property, the API will automatically connect to either the default non-compressed listen port (55555)
or default compressed listen port (55003) based on the specified COMPRESSION_LEVEL. If a port is specified in the SESSION_HOST property you must
specify the non-compressed listen port if not using compression (compression level 0) or the compressed listen port if using compression (compression levels 1 to 9). */
#define SOLCLIENT_SESSION_PROP_GENERATE_RCV_TIMESTAMPS       "SESSION_RCV_TIMESTAMP"  /**< When enabled, a receive timestamp is recorded for each message and passed to the application callback in the rxCallbackInfo_t structure. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_GENERATE_RCV_TIMESTAMPS */
#define SOLCLIENT_SESSION_PROP_GENERATE_SEND_TIMESTAMPS      "SESSION_SEND_TIMESTAMP" /**< When enabled, a send timestamp is automatically included (if not already present) in the Solace-defined fields for each message sent. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_GENERATE_SEND_TIMESTAMPS */ 
#define SOLCLIENT_SESSION_PROP_GENERATE_SENDER_ID            "SESSION_SEND_SENDER_ID" /**< When enabled, a sender ID is automatically included (if not already present) in the Solace-defined fields for each message sent. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_GENERATE_SENDER_ID */ 
#define SOLCLIENT_SESSION_PROP_GENERATE_SEQUENCE_NUMBER      "SESSION_SEND_SEQUENCE_NUMBER" /**< When enabled, a sequence number is automatically included (if not already present) in the Solace-defined fields for each message sent. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_GENERATE_SEQUENCE_NUMBER */ 
#define SOLCLIENT_SESSION_PROP_CONNECT_RETRIES_PER_HOST      "SESSION_CONNECT_RETRIES_PER_HOST" /**<  When using a host list, this property defines how many times to try to connect or reconnect to a single host before moving to the next host in the list. A value of 0 (the default) means make a single connection attempt (that is, 0 retries). A value of -1 means attempt an infinite number of reconnect retries (that is, the API will only try to connect or reconnect to first host listed.) NOTE: This property works in conjunction with the connect and reconnect retries Session properties; it does not replace them.*/
#define SOLCLIENT_SESSION_PROP_CONNECT_RETRIES               "SESSION_CONNECT_RETRIES" /**< How many times to try to connect to the host appliance (or list of appliances) during connection setup. Zero means no automatic connection retries (that is, try once and give up). -1 means try to connect forever. The default valid range is >= -1. 

When using a host list, each time the API works through the host list without establishing a connection is considered an connect retry. For example, if a SOLCLIENT_SESSION_PROP_CONNECT_RETRIES value of two is used, the API could possibly work through all of the listed hosts without connecting to them three times: one time through for the initial connect attempt, and then two times through for connect retries. Each connect retry begins with the first host listed. 
After each unsuccessful attempt to connect to a host, the API waits for the amount of time set for SOLCLIENT_SESSION_PROP_RECONNECT_RETRY_WAIT_MS before attempting another connection to a host, and the number times to attempt to connect to one host before moving on to the next listed host is determined by the value set for SOLCLIENT_SESSION_PROP_CONNECT_RETRIES_PER_HOST. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_CONNECT_RETRIES  */
#define SOLCLIENT_SESSION_PROP_RECONNECT_RETRIES             "SESSION_RECONNECT_RETRIES" /**<How many times to retry to reconnect to the host appliance (or list of appliances) after a connected Session goes down. Zero means no automatic reconnection attempts. -1 means try to reconnect forever. The default valid range is >= -1. 

When using a host list, each time the API works through the host list without establishing a connection is considered a reconnect retry.  Each reconnect retry begins with the first host listed. 
After each unsuccessful attempt to reconnect to a host, the API waits for the amount of time set for SOLCLIENT_SESSION_PROP_RECONNECT_RETRY_WAIT_MS before attempting another connection to a host, and the number times to attempt to connect to one host before moving on to the next listed host is determined by the value set for SOLCLIENT_SESSION_PROP_CONNECT_RETRIES_PER_HOST. 
Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_RECONNECT_RETRIES */
#define SOLCLIENT_SESSION_PROP_RECONNECT_RETRY_WAIT_MS       "SESSION_RECONNECT_RETRY_WAIT_MS" /**< How much time (in ms) to wait between each attempt to connect or reconnect to a host. If a connect or reconnect attempt to host is not successful, the API waits for the amount of time set for SOLCLIENT_SESSION_PROP_RECONNECT_RETRY_WAIT_MS, and then makes another connect or reconnect attempt. SESSION_CONNECT_RETRIES_PER_HOST sets how many connection or reconnection attempts can be made before moving
on to the next host in the list. 
The valid range is >=0. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_RECONNECT_RETRY_WAIT_MS. */
#define SOLCLIENT_SESSION_PROP_USER_ID                       "SESSION_USER_ID" /**< A read-only informational string providing information about the application, such as the name of operating system user that is running the application, the hostname, and the PID of the application. */
#define SOLCLIENT_SESSION_PROP_P2PINBOX_IN_USE               "SESSION_REPLY_TO_DEFAULT_DEST" /**< A read-only informational string that indicates the default reply-to destination that is used when a request message is sent that does not have a reply-to destination specified. See solClient_session_sendRequest(). This parameter is only valid when the Session is connected. */
#define SOLCLIENT_SESSION_PROP_REPLY_TO_DEFAULT_DEST         SOLCLIENT_SESSION_PROP_P2PINBOX_IN_USE /**< Deprecated: see ::SOLCLIENT_SESSION_PROP_P2PINBOX_IN_USE */
#define SOLCLIENT_SESSION_PROP_REAPPLY_SUBSCRIPTIONS         "SESSION_REAPPLY_SUBSCRIPTIONS" /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to have the API remember subscriptions and reapply them upon a Session reconnect. Reapply subscriptions will only apply direct topic subscriptions upon a Session reconnect. It will not reapply topic subscriptions on durable and non-durable endpoints. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_REAPPLY_SUBSCRIPTIONS */
#define SOLCLIENT_SESSION_PROP_TOPIC_DISPATCH                "SESSION_TOPIC_DISPATCH" /**< Use ::SOLCLIENT_PROP_ENABLE_VAL to have the API dispatch messages based on Topic (see @ref topic-dispatch). Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_TOPIC_DISPATCH */
#define SOLCLIENT_SESSION_PROP_PROVISION_TIMEOUT_MS          "SESSION_PROVISION_TIMEOUT_MS" /**< Maximum amount of time (in milliseconds) to wait for a provision command (create or delete an endpoint) */
#define SOLCLIENT_SESSION_PROP_CALCULATE_MESSAGE_EXPIRATION  "SESSION_CALCULATE_MESSAGE_EXPIRATION" /**< If this property is true and time-to-live (::solClient_msg_setTimeToLive()) has a positive value in a message, the expiration time is calculated when the message is sent or received and can be retrieved with ::solClient_msg_getExpiration. */
#define SOLCLIENT_SESSION_PROP_VIRTUAL_ROUTER_NAME           "SESSION_VIRTUAL_ROUTER_NAME" /**< A read-only property that indicates the connected appliance's virtual router name. Appliance endpoint and destination names created with a virtual router name are valid for use with that appliance, or to address destinations on remote appliances (in a multiple-appliance network) when publishing messages. Applications requiring the virtual router name do not need to poll this property every time it is required, and they may cache the name. Applications should query the name once after connecting the Session, and again after a reconnect operation reports the ::SOLCLIENT_SESSION_EVENT_VIRTUAL_ROUTER_NAME_CHANGED event. Prior to connecting, an empty string is returned. */
#define SOLCLIENT_SESSION_PROP_NO_LOCAL                       "SESSION_NO_LOCAL"      /**< If this property is true, messages published on the Session cannot be received on the same Session even if the client has a subscription that matches the published topic. If this restriction is requested, and the appliance does not have No Local support, the Session connect will fail with subcode ::SOLCLIENT_SUBCODE_NO_LOCAL_NOT_SUPPORTED. */
#define SOLCLIENT_SESSION_PROP_AD_PUB_ROUTER_WINDOWED_ACK     "SESSION_AD_PUB_ROUTER_WINDOWED_ACK"    /**< When disabled, initiate a window size of 1 to appliance, but do not wait for acknowledgments before transmitting up to the actual window size. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_AD_PUB_ROUTER_WINDOWED_ACK */
#define SOLCLIENT_SESSION_PROP_MODIFYPROP_TIMEOUT_MS         "SESSION_MODIFYPROP_TIMEOUT_MS" /**< Maximum amount of time (in milliseconds) to wait for session property modification. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_MODIFYPROP_TIMEOUT_MS */
#define SOLCLIENT_SESSION_PROP_ACK_EVENT_MODE                 "SESSION_ACK_EVENT_MODE"              /**< This property specifies if a session event ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT acknowledges a single message (see ::SOLCLIENT_SESSION_PROP_ACK_EVENT_MODE_PER_MSG) or a range of messages (see ::SOLCLIENT_SESSION_PROP_ACK_EVENT_MODE_WINDOWED).  Default: ::SOLCLIENT_SESSION_PROP_ACK_EVENT_MODE_PER_MSG. \n Setting this property to ::SOLCLIENT_SESSION_PROP_ACK_EVENT_MODE_WINDOWED will not affect RejectedMessageError events, they will still be emitted on a per message basis. */
#define SOLCLIENT_SESSION_PROP_SSL_EXCLUDED_PROTOCOLS        "SESSION_SSL_EXCLUDED_PROTOCOLS"  /**< This property specifies a comma separated list of excluded SSL protocol(s). Valid SSL protocols are 'SSLv3', 'TLSv1', 'TLSv1.1', 'TLSv1.2'. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SSL_EXCLUDED_PROTOCOLS. */
#define SOLCLIENT_SESSION_PROP_SSL_VALIDATE_CERTIFICATE      "SESSION_SSL_VALIDATE_CERTIFICATE"      /**< This property indicates if the certificate validation with certificates in the truststore is enabled. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SSL_VALIDATE_CERTIFICATE.  */
#define SOLCLIENT_SESSION_PROP_SSL_CLIENT_CERTIFICATE_FILE           "SESSION_SSL_CLIENT_CERTIFICATE_FILE"         /**< This property specifies the client certificate file name. */
#define SOLCLIENT_SESSION_PROP_SSL_CLIENT_PRIVATE_KEY_FILE           "SESSION_SSL_CLIENT_PRIVATE_KEY_FILE"  /**< This property specifies the client private key file name. */
#define SOLCLIENT_SESSION_PROP_SSL_CLIENT_PRIVATE_KEY_FILE_PASSWORD  "SESSION_SSL_CLIENT_PRIVATE_KEY_FILE_PASSWORD"     /**< This property specifies the password used to encrypt the client private key file. */
#define SOLCLIENT_SESSION_PROP_SSL_CONNECTION_DOWNGRADE_TO       "SESSION_SSL_CONNECTION_DOWNGRADE_TO"  /**< This property specifies a transport protocol that SSL connection will be downgraded to after client authentication. Allowed transport protocol is "PLAIN_TEXT". May be combined with non-zero compression level to achieve compression without encryption. */
#define SOLCLIENT_SESSION_PROP_INITIAL_RECEIVE_BUFFER_SIZE   "SESSION_INITIAL_RECEIVE_BUFFER_SIZE" /**< If not zero, the minimum starting size for the API receive buffer. Must be zero or >= 1024 and <=64*1024*1024 */
#define SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME  "SESSION_AUTHENTICATION_SCHEME"   /**< This property specifies the authentication scheme. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_AUTHENTICATION_SCHEME. */
#define SOLCLIENT_SESSION_PROP_KRB_SERVICE_NAME        "SESSION_KRB_SERVICE_NAME"  /**< This property specifies the first part of Kerberos Service Principal Name (SPN) of the form <i>ServiceName/Hostname\@REALM</i> (for Windows) or Host Based Service of the form <i>ServiceName\@Hostname</i> (for Linux and SunOS).
Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_KRB_SERVICE_NAME.

The <i>Hostname</i> of SPN or Host Based Service is the host portion of an entry specified by the session property ::SOLCLIENT_SESSION_PROP_HOST. The <i>REALM</i> of the SPN is Kerberos realm.
The maximum length of an SPN is 256 characters. \n

Note: This property is used for all entries specified by the property ::SOLCLIENT_SESSION_PROP_HOST. 

*/
#define SOLCLIENT_SESSION_PROP_UNBIND_FAIL_ACTION       "SESSION_UNBIND_FAIL_ACTION"    /**< A property to define the behavior if an unbind-response is not received after an unbind-request (::solClient_flow_destroy()) is sent to the Solace Appliance. If this occurs it is possible that the endpoint may be still bound and unavailable until the session is terminated. In this occurrence the session can be configured to retry sending the unbind-request or to fail the session transport. If the sesion transport fails, the session will behave as defined by the ::SOLCLIENT_SESSION_PROP_RECONNECT_RETRIES configuration.  Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_UNBIND_FAIL_ACTION. */
#define SOLCLIENT_SESSION_PROP_WEB_TRANSPORT_PROTOCOL                     ("SESSION_WEB_TRANSPORT_PROTOCOL")        /**< This property specifies a WEB Transport Protocol in the default WEB Transport Protocol downgrade list to use for the session connection.
                                                                                                                          Valid values are \ref transportProtocol. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_WEB_TRANSPORT_PROTOCOL 
                                                                                                                          If the connection fails, the API will try to connect using the next (less efficient) availabe protocol in the list until it finds one that works.
                                                                                                                          If there is none available, then the session connection fails.
                                                                                                                     */
#define SOLCLIENT_SESSION_PROP_WEB_TRANSPORT_PROTOCOL_IN_USE              ("SESSION_WEB_TRANSPORT_PROTOCOL_IN_USE") /**< Read-only property which returns the WEB Transport Protocol currently in use for web messaging. An empty string is returned when a session
                                                                                                                         is not connected or it is not a web messaging session.  Valid values are \ref transportProtocol. Valid values are \ref transportProtocol.
                                                                                                                     */
#define SOLCLIENT_SESSION_PROP_WEB_TRANSPORT_PROTOCOL_LIST            ("SESSION_WEB_TRANSPORT_PROTOCOL_LIST")   /**< This property specifies a comma separated list of WEB Transport Protocols to use for session connection. The API will use the first one in
                                                                                                                     the list to start session connection. If the connection fails, the API will try to connect using the next available protocol in the list until
                                                                                                                     it finds one that works. If there is none available, then the session connection fails.
                                                                                                                     Note: Only one of the session properties ::SOLCLIENT_SESSION_PROP_WEB_TRANSPORT_PROTOCOL and ::SOLCLIENT_SESSION_PROP_WEB_TRANSPORT_PROTOCOL_LIST
                                                                                                                     is allowed to be configured. There shall be no duplicates in the list. */
#define SOLCLIENT_SESSION_PROP_TRANSPORT_PROTOCOL_DOWNGRADE_TIMEOUT_MS ("SESSION_TRANSPORT_PROTOCOL_DOWNGRADE_TIMEOUT_MS")  /**< Specifies how long to wait (in milliseconds) for a login response before moving to the next available protocol in a user specified or the default WEB Transport Protocol downgradelist.
                                                                                                                     Default ::SOLCLIENT_SESSION_PROP_DEFAULT_TRANSPORT_PROTOCOL_DOWNGRADE_TIMEOUT_MS */
#define SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION  "SESSION_GD_RECONNECT_FAIL_ACTION"       /**< A property to define the behavior when the CCSMP API is unable 
                                                                                                       to reconnect guaranteed delivery after reconnecting the session.
                                                                                                       This may occur if the session is configured with a host list where
                                                                                                       each Solace router in the host list is unaware of state on the
                                                                                                       previous router. It can also occur if the time to reconnect to the
                                                                                                       same router exceeds the publisher flow timeout on the router.
                                                                                                       May be set as an environment variable (See @ref SessionProps).\n
                                                                                                       Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_GD_RECONNECT_FAIL_ACTION 
                                                                                                       */

/*@}*/
/**
 * @anchor transportProtocol
 * @name Session transport protocol types
 * Definition of the valid set of transport protocols when setting ::SOLCLIENT_SESSION_PROP_WEB_TRANSPORT_PROTOCOL, or returned
 * via the read-only session property ::SOLCLIENT_SESSION_WEB_PROP_TRANSPORT_PROTOCOL_IN_USE
 */
/*@{*/
#define SOLCLIENT_TRANSPORT_PROTOCOL_NULL                    ("")                      /**< An empty value for ::SOLCLIENT_SESSION_PROP_WEB_TRANSPORT_PROTOCOL means "use best available". An empty return value for ::SOLCLIENT_SESSION_PROP_WEB_TRANSPORT_PROTOCOL_IN_USE means not connected or not a web messaging session. */
#define SOLCLIENT_TRANSPORT_PROTOCOL_WS_BINARY               ("WS_BINARY")             /**< Binary-encoded, using the WebSocket protocol */
#define SOLCLIENT_TRANSPORT_PROTOCOL_HTTP_BINARY_STREAMING   ("HTTP_BINARY_STREAMING") /**< Binary encoded, responses are received in streaming mode for higher efficiency */
#define SOLCLIENT_TRANSPORT_PROTOCOL_HTTP_BINARY             ("HTTP_BINARY")           /**< Binary-encoded, responses are received in COMET style */
/*@}*/

/**
 *@anchor sslDowngradeProtocol
 *@name Transport Protocols for SSL Downgrade
 */
#define SOLCLIENT_TRANSPORT_PROTOCOL_PLAIN_TEXT             ("PLAIN_TEXT")
/*@}*/

/** 
 *@name Authentication Scheme
 */
/*@{*/
#define SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME_BASIC    "AUTHENTICATION_SCHEME_BASIC"
#define SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME_CLIENT_CERTIFICATE  "AUTHENTICATION_SCHEME_CLIENT_CERTIFICATE"
#define SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME_GSS_KRB  "AUTHENTICATION_SCHEME_GSS_KRB"
/*@}*/
/** 
 *@name Unbind Failure Actions
 */
/*@{*/
#define SOLCLIENT_SESSION_PROP_UNBIND_FAIL_ACTION_RETRY             "UNBIND_FAIL_ACTION_RETRY"
#define SOLCLIENT_SESSION_PROP_UNBIND_FAIL_ACTION_DISCONNECT        "UNBIND_FAIL_ACTION_DISCONNECT"
/*@}*/

/** 
@see @ref feature-limitations
*/
#define SOLCLIENT_SESSION_PROP_SSL_VALIDATE_CERTIFICATE_DATE    "SESSION_SSL_VALIDATE_CERTIFICATE_DATE"  /**< This property indicates if the session connection should fail when a certificate with an invalid date is received. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SSL_VALIDATE_CERTIFICATE_DATE. */
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_SUITES      "SESSION_SSL_CIPHER_SUITES"    /**< This property specifies a comma separated list of the cipher suites. Allowed cipher suites are: 'ECDHE-RSA-AES256-GCM-SHA384', 'ECDHE-RSA-AES256-SHA384', 'ECDHE-RSA-AES256-SHA', 'AES256-GCM-SHA384', 'AES256-SHA256', 'AES256-SHA', 'ECDHE-RSA-DES-CBC3-SHA', 'DES-CBC3-SHA', 'ECDHE-RSA-AES128-GCM-SHA256', 'ECDHE-RSA-AES128-SHA256', 'ECDHE-RSA-AES128-SHA', 'AES128-GCM-SHA256', 'AES128-SHA256', 'AES128-SHA', 'RC4-SHA', 'RC4-MD5', 'TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384', 'TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384', 'TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA', 'TLS_RSA_WITH_AES_256_GCM_SHA384', 'TLS_RSA_WITH_AES_256_CBC_SHA256', 'TLS_RSA_WITH_AES_256_CBC_SHA', 'TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA', 'SSL_RSA_WITH_3DES_EDE_CBC_SHA', 'TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256', 'TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256', 'TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA', 'TLS_RSA_WITH_AES_128_GCM_SHA256', 'TLS_RSA_WITH_AES_128_CBC_SHA256', 'TLS_RSA_WITH_AES_128_CBC_SHA', 'SSL_RSA_WITH_RC4_128_SHA', 'SSL_RSA_WITH_RC4_128_MD5'. Default: ::SOLCLIENT_SESSION_PROP_DEFAULT_SSL_CIPHER_SUITES. */
#define SOLCLIENT_SESSION_PROP_SSL_TRUST_STORE_DIR    "SESSION_SSL_TRUST_STORE_DIR"        /**< This property specifies the directory where the trusted certificates are. A maximum of 64 certificate files are allowed in the trust store directory. The maximum depth for the certificate chain verification that shall be allowed is 3. */
#define SOLCLIENT_SESSION_PROP_SSL_TRUSTED_COMMON_NAME_LIST  "SESSION_SSL_TRUSTED_COMMON_NAME_LIST"    /**< This property specifies  a comma separated list of acceptable common names in certificate validation. The number of common names specified by an applications is limited to 16. Leading and trailing whitespaces are considered to be part of the common names and are not ignored. If the application does not provide any common names, there is no common name verification. */
/*@}*/

/**
 * @name Guaranteed Delivery Reconnect Fail Actions
 * Defines the valid set of actions the API will take if it is unable to reconnect
 * guaranteed delivery after a session reconnect.  This will occur when a host-list
 * is used, such as for disaster recovery. After session reconnect to the next router
 * in the host-list the guaranteed delivery reconnect will not succeed as guaranteed
 * delivery state is only preserved between a high-availability pair.
 */
/*@{*/
#define SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION_AUTO_RETRY    ("GD_RECONNECT_FAIL_ACTION_AUTO_RETRY")  /**< Clear the publisher state and reconnect the publisher flow. Then republish all unacknowledged messages, this may cause duplication. The API then continues the reconnect process as usual.  */
#define SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION_DISCONNECT   ("GD_RECONNECT_FAIL_ACTION_DISCONNECT") /**< Disconnect the session, even if SOLCLIENT_SESSION_PROP_RECONNECT_RETRIES is configured to a non-zero value. This is the legacy behavior. If the application attempts to manually reconnect the session, it is also responsible for unacknowledged messages. If the application chooses to resend those messages, there may be duplication. If the application chooses not to resend those messages there may be message loss. \n
* Special considerations are required for ::SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION_DISCONNECT:
 @li When a reconnect occurs on a different host, an application publishing
* guaranteed messages will receive the SOLCLIENT_SESSION_EVENT_DOWN_ERROR event with the ::SOLCLIENT_SUBCODE_UNKNOWN_FLOW_NAME sub-code.  
* When this occurs, any queued messages are flushed.  Messages published after this event has been raised will be queued then sent after a
* subsequent connect initiated by the application by calling solClient_session_connect().
* @li Multi-threaded applications should be aware that some, but not necessarily all, messages published on one thread may be flushed due to 
* failed reconnect, as messages published after the SOLCLIENT_SESSION_EVENT_DOWN_ERROR event are not flushed. If the application chooses
* to republish some or all unacknowledged messages after the send queue has been flushed there is
* a possibility that these old, republished messages may be queued after newly published messages. 
* @li If the possibility of old messages after new messages is a concern, it is recommended that instead of calling 
* solClient_session_connect() on the session that has gone down, this session should instead be destroyed
* and a new session created to establish a new connection.*/
/*@}*/

/** @defgroup DefaultSessionProps Default Session Configuration Properties
* Default values for Session configuration properties that are not explicitly set.
*/
/*@{*/
#define SOLCLIENT_SESSION_PROP_DEFAULT_USERNAME                      ""          /**< The default value for username. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_PASSWORD                      ""          /**< The default value for password. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_HOST                          "127.0.0.1" /**< The default value for the appliance IP address. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_PORT                          "55555"     /**< The default value for the appliance TCP port when compression is not in use (::SOLCLIENT_SESSION_PROP_COMPRESSION_LEVEL of zero). */
#define SOLCLIENT_SESSION_PROP_DEFAULT_PORT_COMPRESSION              "55003"     /**< The default value for the appliance TCP port when compression is in use (::SOLCLIENT_SESSION_PROP_COMPRESSION_LEVEL of non-zero). */
#define SOLCLIENT_SESSION_PROP_DEFAULT_PORT_SSL                      "55443"     /**< The default value for the appliance SSL port over TCP regardless of compression. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_BUFFER_SIZE                   "90000"     /**< The default size (in bytes) of internal buffer for transmit buffering. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_CONNECT_BLOCKING              SOLCLIENT_PROP_ENABLE_VAL /**< The default is blocking connect operation. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SEND_BLOCKING                 SOLCLIENT_PROP_ENABLE_VAL /**< The default is blocking send operation. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SUBSCRIBE_BLOCKING            SOLCLIENT_PROP_ENABLE_VAL /**< The default is blocking subscribe/unsubscribe operation. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_BLOCK_WHILE_CONNECTING        SOLCLIENT_PROP_ENABLE_VAL /**< The default is to block operations such as sending a message, subscribing, or unsubscribing if the Session is being connected. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_BLOCKING_WRITE_TIMEOUT_MS     "100000"    /**< The default blocking write timeout (in milliseconds). */
#define SOLCLIENT_SESSION_PROP_DEFAULT_CONNECT_TIMEOUT_MS            "30000"     /**< The default connect timeout (in milliseconds). */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SUBCONFIRM_TIMEOUT_MS         "10000"     /**< The default subscription confirm (add or remove) timeout (in milliseconds). */
#define SOLCLIENT_SESSION_PROP_DEFAULT_IGNORE_DUP_SUBSCRIPTION_ERROR SOLCLIENT_PROP_ENABLE_VAL /**< The default is ignore errors for duplicate subscription/topic on subscribe or unsubscribe. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_TCP_NODELAY                   SOLCLIENT_PROP_ENABLE_VAL /**< The default value for TCP no delay. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SOCKET_SEND_BUF_SIZE          "90000"     /**< Use 0 to set the socket send buffer size to the operating system default. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SOCKET_RCV_BUF_SIZE           "150000"    /**< Use 0 to set the socket receive buffer size to the operating system default. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_KEEP_ALIVE_INT_MS             "3000"         /**< The default amount of time (in milliseconds) to wait between sending out Keep-Alive messages. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_KEEP_ALIVE_LIMIT              "3"         /**< The default value for the number of consecutive Keep-Alive messages that can be sent without receiving a response before the connection is closed by the API.*/
#define SOLCLIENT_SESSION_PROP_DEFAULT_APPLICATION_DESCRIPTION       ""          /**< The default value for the application description. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_CLIENT_MODE                   SOLCLIENT_PROP_DISABLE_VAL /**< The default value for client mode. When disabled, the Session uses three TCP connections for non-client mode. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_BIND_IP                       ""          /**< The default value for local IP on connect is unset (bind to any) .*/
#define SOLCLIENT_SESSION_PROP_DEFAULT_PUB_ACK_TIMER                 "2000"      /**< The default value for publisher acknowledgment timer (in milliseconds). When a published message is not acknowledged within the time specified for this timer, the API automatically retransmits the message. There is no limit on the number of retransmissions for any message. However, while the API is resending, applications can become flow controlled. The flow control behavior is controlled by ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING and ::SOLCLIENT_SESSION_PROP_BLOCKING_WRITE_TIMEOUT_MS.*/
#define SOLCLIENT_SESSION_PROP_DEFAULT_PUB_WINDOW_SIZE               "50"        /**< The default Publisher Window size for Guaranteed messages. The Guaranteed Message Publish Window Size property limits the maximum number of messages that can be published before the API must receive an acknowledgment from the appliance.*/
#define SOLCLIENT_SESSION_PROP_DEFAULT_VPN_NAME                      ""          /**< The default Message VPN name to connect this Session to. The default is to not specify the VPN name; the default Message VPN provisioned on the appliance is used. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_CLIENT_NAME                   ""          /**< The default Session Client Name is a null string to have the C API generate one. */ 
#define SOLCLIENT_SESSION_PROP_DEFAULT_SUBSCRIBER_LOCAL_PRIORITY     "1"         /**< The default subscriber priority for locally published messages.  */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SUBSCRIBER_NETWORK_PRIORITY   "1"         /**< The default subscriber priority for remotely published messages.  */
#define SOLCLIENT_SESSION_PROP_DEFAULT_COMPRESSION_LEVEL             "0"         /**< The default compression level (no compression). */
#define SOLCLIENT_SESSION_PROP_DEFAULT_GENERATE_RCV_TIMESTAMPS       SOLCLIENT_PROP_DISABLE_VAL /**< The default receive message timestamps. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_GENERATE_SEND_TIMESTAMPS      SOLCLIENT_PROP_DISABLE_VAL /**< The default for automatically include send message timestamps. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_GENERATE_SENDER_ID            SOLCLIENT_PROP_DISABLE_VAL /**< The default for automatically include a sender id. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_GENERATE_SEQUENCE_NUMBER      SOLCLIENT_PROP_DISABLE_VAL /**< The default for automatically include a sequence number. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_CONNECT_RETRIES_PER_HOST      "0"         /**< The default number of connect retries per host. Zero means only try once when connecting. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_CONNECT_RETRIES               "0"         /**< The default number of connect retries. Zero means only try once when connecting. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_RECONNECT_RETRIES             "0"         /**< The default number of reconnect retries. Zero means no automatic connection retries after a Session goes down. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_RECONNECT_RETRY_WAIT_MS       "3000"      /**< The default amount of time in (milliseconds) to wait before attempting a reconnect attempt. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_REAPPLY_SUBSCRIPTIONS         SOLCLIENT_PROP_DISABLE_VAL      /**< The default value for ::SOLCLIENT_SESSION_PROP_REAPPLY_SUBSCRIPTIONS */
#define SOLCLIENT_SESSION_PROP_DEFAULT_TOPIC_DISPATCH                SOLCLIENT_PROP_DISABLE_VAL      /**< The default value for ::SOLCLIENT_SESSION_PROP_TOPIC_DISPATCH (see @ref topic-dispatch) */
#define SOLCLIENT_SESSION_PROP_DEFAULT_PROVISION_TIMEOUT_MS          "3000"      /**< The default amount of time (in milliseconds) to wait for a provision command. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_MODIFYPROP_TIMEOUT_MS         "10000"      /**< The default amount of time (in milliseconds) to wait for session property modification. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_CALCULATE_EXPIRATION_TIME SOLCLIENT_PROP_DISABLE_VAL       /**< The default value for ::SOLCLIENT_SESSION_PROP_CALCULATE_MESSAGE_EXPIRATION */
#define SOLCLIENT_SESSION_PROP_DEFAULT_NO_LOCAL                      SOLCLIENT_PROP_DISABLE_VAL   /**< The default value for ::SOLCLIENT_SESSION_PROP_NO_LOCAL */
#define SOLCLIENT_SESSION_PROP_DEFAULT_AD_PUB_ROUTER_WINDOWED_ACK     SOLCLIENT_PROP_ENABLE_VAL   /**< The default value for ::SOLCLIENT_SESSION_PROP_AD_PUB_ROUTER_WINDOWED_ACK */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SSL_EXCLUDED_PROTOCOLS         ""         /**< The default value for ::SOLCLIENT_SESSION_PROP_SSL_EXCLUDED_PROTOCOLS*/
#define SOLCLIENT_SESSION_PROP_DEFAULT_SSL_VALIDATE_CERTIFICATE     SOLCLIENT_PROP_ENABLE_VAL      /**< The default value for ::SOLCLIENT_SESSION_PROP_SSL_VALIDATE_CERTIFICATE. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SSL_VALIDATE_CERTIFICATE_DATE   SOLCLIENT_PROP_ENABLE_VAL   /**< The default value for ::SOLCLIENT_SESSION_PROP_SSL_VALIDATE_CERTIFICATE_DATE. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_SSL_CIPHER_SUITES  ("ECDHE-RSA-AES256-GCM-SHA384,ECDHE-RSA-AES256-SHA384,ECDHE-RSA-AES256-SHA,AES256-GCM-SHA384,AES256-SHA256,AES256-SHA,ECDHE-RSA-DES-CBC3-SHA,DES-CBC3-SHA,ECDHE-RSA-AES128-GCM-SHA256,ECDHE-RSA-AES128-SHA256,ECDHE-RSA-AES128-SHA,AES128-GCM-SHA256,AES128-SHA256,AES128-SHA,RC4-SHA,RC4-MD5") /**< The default value for ::SOLCLIENT_SESSION_PROP_SSL_CIPHER_SUITES. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_INITIAL_RECEIVE_BUFFER_SIZE "0" /**< The default value for ::SOLCLIENT_SESSION_PROP_INITIAL_RECEIVE_BUFFER_SIZE */
#define SOLCLIENT_SESSION_PROP_DEFAULT_AUTHENTICATION_SCHEME              SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME_BASIC   /**< The default value for ::SOLCLIENT_SESSION_PROP_AUTHENTICATION_SCHEME. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_KRB_SERVICE_NAME          "solace"                                 /**< The default for ::SOLCLIENT_SESSION_PROP_KRB_SERVICE_NAME. */
#define SOLCLIENT_SESSION_PROP_DEFAULT_UNBIND_FAIL_ACTION       SOLCLIENT_SESSION_PROP_UNBIND_FAIL_ACTION_RETRY /**< The default value for ::SOLCLIENT_SESSION_PROP_UNBIND_FAIL_ACTION */
#define SOLCLIENT_SESSION_PROP_DEFAULT_WEB_TRANSPORT_PROTOCOL             SOLCLIENT_TRANSPORT_PROTOCOL_NULL          /**< The default value for web messaging Transport Protocol. Default is "use best available". */
#define SOLCLIENT_SESSION_PROP_DEFAULT_TRANSPORT_PROTOCOL_DOWNGRADE_TIMEOUT_MS  ("3000")                        /**< The default value for the Transport Protocol downgrade timeout in milliseconds.*/
#define SOLCLIENT_SESSION_PROP_DEFAULT_GD_RECONNECT_FAIL_ACTION      SOLCLIENT_SESSION_PROP_GD_RECONNECT_FAIL_ACTION_AUTO_RETRY  /**< The default action when the CCSMP API is unable to reestablish the publisher flow is to complete the reconnect on the session. The CCSMP API will open a new publisher flow for the session,  which may lead to publishing duplicate messages.  */
/*@}*/


/** @name SSL ciphers
*/
/*@{*/
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_ECDHE_RSA_AES256_GCM_SHA384              ("ECDHE-RSA-AES256-GCM-SHA384")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384    ("TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_ECDHE_RSA_AES256_SHA384                  ("ECDHE-RSA-AES256-SHA384")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384    ("TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_ECDHE_RSA_AES256_SHA                     ("ECDHE-RSA-AES256-SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA       ("TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_AES256_GCM_SHA384                        ("AES256-GCM-SHA384")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_RSA_WITH_AES_256_GCM_SHA384          ("TLS_RSA_WITH_AES_256_GCM_SHA384")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_AES256_SHA256                            ("AES256-SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_RSA_WITH_AES_256_CBC_SHA256          ("TLS_RSA_WITH_AES_256_CBC_SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_AES256_SHA                               ("AES256-SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_RSA_WITH_AES_256_CBC_SHA             ("TLS_RSA_WITH_AES_256_CBC_SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_ECDHE_RSA_DES_CBC3_SHA                   ("ECDHE-RSA-DES-CBC3-SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA      ("TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_DES_CBC3_SHA                             ("DES-CBC3-SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_SSL_RSA_WITH_3DES_EDE_CBC_SHA            ("SSL_RSA_WITH_3DES_EDE_CBC_SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_ECDHE_RSA_AES128_GCM_SHA256              ("ECDHE-RSA-AES128-GCM-SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256    ("TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_ECDHE_RSA_AES128_SHA256                  ("ECDHE-RSA-AES128-SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256    ("TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_ECDHE_RSA_AES128_SHA                     ("ECDHE-RSA-AES128-SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA       ("TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_AES128_GCM_SHA256                        ("AES128-GCM-SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_RSA_WITH_AES_128_GCM_SHA256          ("TLS_RSA_WITH_AES_128_GCM_SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_AES128_SHA256                            ("AES128-SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_RSA_WITH_AES_128_CBC_SHA256          ("TLS_RSA_WITH_AES_128_CBC_SHA256")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_AES128_SHA                               ("AES128-SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_TLS_RSA_WITH_AES_128_CBC_SHA             ("TLS_RSA_WITH_AES_128_CBC_SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_RC4_SHA                                  ("RC4-SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_SSL_RSA_WITH_RC4_128_SHA                 ("SSL_RSA_WITH_RC4_128_SHA")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_RC4_MD5                                  ("RC4-MD5")
#define SOLCLIENT_SESSION_PROP_SSL_CIPHER_SSL_RSA_WITH_RC4_128_MD5                 ("SSL_RSA_WITH_RC4_128_MD5")
/*@}*/

/** @name SSL Protocols
*/
/*@{*/
#define SOLCLIENT_SESSION_PROP_SSL_PROTOCOL_TLSV1_2				  ("TLSv1.2")
#define SOLCLIENT_SESSION_PROP_SSL_PROTOCOL_TLSV1_1				  ("TLSv1.1")
#define SOLCLIENT_SESSION_PROP_SSL_PROTOCOL_TLSV1                 ("TLSv1")
#define SOLCLIENT_SESSION_PROP_SSL_PROTOCOL_SSLV3                 ("SSLv3")
/*@}*/

/** @name Configuration Properties Maximum Sizes
* The maximum sizes for certain configuration property values. Maximum string lengths do not include the terminating NULL.
* The actual strings including the terminating NULL can be one character longer.
*/
/*@{*/
#define SOLCLIENT_SESSION_PROP_MAX_USERNAME_LEN    (189)  /**< The maximum length of username string (Session property), not including the NULL terminator. */
#define SOLCLIENT_SESSION_PROP_MAX_PASSWORD_LEN    (128) /**< The maximum length of password string (Session property), not including the NULL terminator. */
#define SOLCLIENT_SESSION_PROP_MAX_HOSTS           (16)  /**< The maximum number of @ref host-list "hosts" that can appear in the Session host property. */
#define SOLCLIENT_SESSION_PROP_MAX_APP_DESC        (255) /**< The maximum length of application (that is, the client) description string (Session property), not including the NULL terminator. */
#define SOLCLIENT_SESSION_PROP_MAX_CLIENT_NAME_LEN (160) /**< The maximum length of client name string (Session property), not including the NULL terminator. */
#define SOLCLIENT_SESSION_PROP_MAX_VPN_NAME_LEN    (32)  /**< The maximum length of a Message VPN name string (Session property), not including the NULL terminator. */
#define SOLCLIENT_SESSION_PROP_MAX_VIRTUAL_ROUTER_NAME_LEN (52)  /**< The maximum length of a virtual router name (read-only Session property), not including the NULL terminator. */
/*@}*/

/** @anchor flowProps
 *  @name Flow Configuration Properties
 * Items that can be configured for a Flow.
 */

#define SOLCLIENT_FLOW_PROP_BIND_BLOCKING      "FLOW_BIND_BLOCKING"        /**< This property controls whether or not to block in solClient_session_createFlow(). Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_BIND_BLOCKING */
#define SOLCLIENT_FLOW_PROP_BIND_TIMEOUT_MS    "FLOW_BIND_TIMEOUT_MS"      /**< The timeout (in milliseconds) used when creating a Flow in blocking mode. The valid range is > 0. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_BIND_TIMEOUT_MS */
#define SOLCLIENT_FLOW_PROP_BIND_ENTITY_ID     "FLOW_BIND_ENTITY_ID"       /**< The type of object to which this Flow is bound. The valid values are ::SOLCLIENT_FLOW_PROP_BIND_ENTITY_SUB, ::SOLCLIENT_FLOW_PROP_BIND_ENTITY_QUEUE, and ::SOLCLIENT_FLOW_PROP_BIND_ENTITY_TE. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_BIND_ENTITY_ID */
#define SOLCLIENT_FLOW_PROP_BIND_ENTITY_DURABLE "FLOW_BIND_ENTITY_DURABLE" /**< The durability of the object to which this Flow is bound. Default: ::SOLCLIENT_PROP_ENABLE_VAL, which means the endpoint is durable. When set to SOLCLIENT_PROP_DISABLE_VAL, a temporary endpoint is created. */
#define SOLCLIENT_FLOW_PROP_BIND_NAME          "FLOW_BIND_NAME"            /**< The name of the Queue or Topic Endpoint that is the target of the bind. This property is ignored when the BIND_ENTITY_ID is ::SOLCLIENT_FLOW_PROP_BIND_ENTITY_SUB. The maximum length (not including NULL terminator) is ::SOLCLIENT_BUFINFO_MAX_QUEUENAME_SIZE except for durable queues, which has a limit of ::SOLCLIENT_BUFINFO_MAX_DURABLE_QUEUENAME_SIZE. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_BIND_NAME */
#define SOLCLIENT_FLOW_PROP_WINDOWSIZE         "FLOW_WINDOWSIZE"           /**< The Guaranteed message window size for the Flow. This sets the maximum number of messages that can be in transit (that is, the messages are sent from the appliance but are not yet delivered to the application). The valid range is 1..255. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_WINDOWSIZE */
#define SOLCLIENT_FLOW_PROP_AUTOACK            "FLOW_AUTOACK"              /**< Deprecated: When set to ::SOLCLIENT_PROP_ENABLE_VAL, the API generates application level acknowledgments when the receive callback function returns. This property is ignored if ::SOLCLIENT_FLOW_PROP_ACKMODE is specified. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_AUTOACK */
#define SOLCLIENT_FLOW_PROP_ACKMODE            "FLOW_ACKMODE"              /**< Controls how acknowledgments are generated for received Guaranteed messages. Possible values are ::SOLCLIENT_FLOW_PROP_ACKMODE_AUTO and ::SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT. Default ::SOLCLIENT_FLOW_PROP_ACKMODE_AUTO */
#define SOLCLIENT_FLOW_PROP_TOPIC              "FLOW_TOPIC"                /**< When binding to a Topic endpoint, the Topic may be set in the bind. This parameter is ignored for Queue or subscriber binding. The maximum length (not including NULL terminator) is ::SOLCLIENT_BUFINFO_MAX_TOPIC_SIZE. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_TOPIC */
#define SOLCLIENT_FLOW_PROP_MAX_BIND_TRIES     "FLOW_MAX_BIND_TRIES"       /**< When creating a non-blocking Flow, or a Flow that is bound due to Session-connect, the maximum number of bind attempts to make. The valid range is >= 1. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_MAX_BIND_TRIES */
#define SOLCLIENT_FLOW_PROP_ACK_TIMER_MS       "FLOW_ACK_TIMER_MS"         /**< The duration of the Flow acknowledgment timer (in milliseconds). The valid range is 20..1500. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_ACK_TIMER_MS */
#define SOLCLIENT_FLOW_PROP_ACK_THRESHOLD      "FLOW_ACK_THRESHOLD"        /**< The threshold for sending an acknowledgement, configured as a percentage. 
 * The API sends a transport acknowledgment every N messages where N is calculated as this percentage of the flow window size if the endpoint's max-delivered-unacked-msgs-per-flow setting at bind time is greater than or equal to the transport window size. 
 * Otherwise, N is calculated as this percentage of the endpoint's max-delivered-unacked-msgs-per-flow setting at bind time. 
 *
 * The valid range is 1..75. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_ACK_THRESHOLD 
 */
#define SOLCLIENT_FLOW_PROP_START_STATE        "FLOW_START_STATE"          /**< This property controls whether the Flow should be created in a start or stop state with respect to receiving messages. Flow start/stop state can be changed later through solClient_flow_start() or solClient_flow_stop(). Default ::SOLCLIENT_FLOW_PROP_DEFAULT_START_STATE */ 
#define SOLCLIENT_FLOW_PROP_SELECTOR           "FLOW_SELECTOR"             /**< A Java Message System (JMS) defined selector. */
#define SOLCLIENT_FLOW_PROP_NO_LOCAL           "FLOW_NO_LOCAL"             /**< When a Flow has the No Local property enabled, messages published on the Session cannot appear in a Flow created in the same Session, even if the endpoint contains a subscription that matches the published message. The appliance that the Session connects to must have the No Local capability, and the capability must be enabled. If the appliance the Session is connected to does not support No Local, a call to solClient_session_createFlow() returns SOLCLIENT_FAIL and subcode ::SOLCLIENT_SUBCODE_NO_LOCAL_NOT_SUPPORTED set. */
#define SOLCLIENT_FLOW_PROP_FORWARDING_MODE    "FLOW_FORWARDING_MODE"      /**< Set the @ref forwardingModes "forwarding mode" for messages received on the Flow. If the @ref forwardingModes "forwarding mode" chosen is not supported by the appliance ::solClient_session_createFlow() will fail. Default: ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_STORE_AND_FORWARD. */
#define SOLCLIENT_FLOW_PROP_MAX_UNACKED_MESSAGES "FLOW_MAX_UNACKED_MESSAGES" /**< This property may only be set when the Flow property ::SOLCLIENT_FLOW_PROP_ACKMODE is set to ::SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT and when the ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE is set to ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_STORE_AND_FORWARD.. When set to a positive value, this property controls the maximum number of messages that may be unacknowledged on the Flow (solClient_flow_sendAck() is called to acknowledge messages and remove those messages from the message spool). This property cannot be used to increase the appliance configured maximum number of acknowledged messages on the endpoint.  When set to -1, the appliance configured maximum controls how many unacknowledged messages may be received by the application. Valid values are -1 and >0.  Default ::SOLCLIENT_FLOW_PROP_DEFAULT_MAX_UNACKED_MESSAGES */
#define SOLCLIENT_FLOW_PROP_BROWSER            "FLOW_BROWSER"              /**< Set browser mode on flow and signal it is a browser flow to appliance on bind. A browser flow allows client applications to look at messages spooled on Endpoints without removing them. Messages are browsed from oldest to newest. The flow window size will be reduced as messages are received. Applications have to call ::solClient_flow_start() to get more messages. After being browsed, messages are still available for consumption over normal flows. However, it is possible to selectively remove messages (by calling ::solClient_flow_sendAck()) from the persistent store of an Endpoint, in this case, these removed messages will no longer be available for consumption. Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_BROWSER \n\n
                                                          *  <b>NOTE:</b> If browsing a queue with an active consumer, no guarantee is made that
                                                          *  the browser will receive all messages published to the queue. The consumer can
                                                          *  receive and acknowledge messages before they are delivered to the browser.
                                                          */
#define SOLCLIENT_FLOW_PROP_ACTIVE_FLOW_IND    "FLOW_ACTIVE_FLOW_IND"      /**< When a Flow has the Active Flow Indication property enabled, the application will receive flow events when the flow becomes active, or inactive.  If the underlying session capabilities indicate that the appliance does not support active flow indications, then solClient_session_createFlow() will fail immediately (SOLCLIENT_FAIL) and set the subCode SOLCLIENT_SUBCODE_FLOW_ACTIVE_FLOW_INDICATION_UNSUPPORTED.  Default: ::SOLCLIENT_FLOW_PROP_DEFAULT_ACTIVE_FLOW_IND */
#define SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION "FLOW_REPLAY_START_LOCATION" /**< When a Flow is created, the application may request replay of messages from the replay log, even messages that have been previously delivered and removed the from topic endpoint or queue.  The replay start location may be ::SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION_BEGINNING to indicate that all messages available should be replayed.  Or the replay start location may be a string that begins with "DATE:" followed by a date in one of two formats. The date may be a string representing a long integer, which is the number of seconds since the epoch - 0:00:00 Jan 1, 1970.  The date may be a string as specified in RFC3339 - 'YYYY-MM-DDTHH:MM:SS[.1*DIGIT]Z' or 'YYYY-MM-DDTHH:MM:SS[.1*DIGIT]("+"/"-")HH:MM'. */

/** @name Default Flow Configuration Properties
 *  The default values for Flow configuration.
 */
/*@{*/
#define SOLCLIENT_FLOW_PROP_DEFAULT_BIND_BLOCKING       SOLCLIENT_PROP_ENABLE_VAL    /**< The default is bind blocking.  */
#define SOLCLIENT_FLOW_PROP_DEFAULT_BIND_TIMEOUT_MS     "10000" /**< The default bind timeout in milliseconds. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_BIND_ENTITY_ID      SOLCLIENT_FLOW_PROP_BIND_ENTITY_SUB     /**< The default bind target type. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_BIND_ENTITY_DURABLE SOLCLIENT_PROP_ENABLE_VAL               /**< The default bind target durability. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_BIND_NAME           ""      /**< The default bind target. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_WINDOWSIZE          "255"   /**< The default Flow window size. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_AUTOACK             SOLCLIENT_PROP_ENABLE_VAL    /**< The default acknowledgment mode is AutoAcknowledgment. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_TOPIC               ""      /**< The default Topic for a Topic Endpoint bind. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_MAX_BIND_TRIES      "3"     /**< The default maximum number of bind attempts. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_ACK_TIMER_MS        "1000"  /**< The default Flow acknowledgment timer in milliseconds. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_ACK_THRESHOLD       "60"    /**< The default threshold for sending an acknowledgment, as a percentage of the Flow window size. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_START_STATE         SOLCLIENT_PROP_ENABLE_VAL    /**< The default value for the ::SOLCLIENT_FLOW_PROP_START_STATE property. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_SELECTOR            ""      /**< The default selector when binding to an endpoint.  */
#define SOLCLIENT_FLOW_PROP_DEFAULT_NO_LOCAL            SOLCLIENT_PROP_DISABLE_VAL   /**< The default value for the ::SOLCLIENT_FLOW_PROP_NO_LOCAL property. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_FORWARDING_MODE     SOLCLIENT_FLOW_PROP_FORWARDING_MODE_STORE_AND_FORWARD  /**< The default value for the ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE property. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_MAX_UNACKED_MESSAGES "-1"   /**< The default value for ::SOLCLIENT_FLOW_PROP_MAX_UNACKED_MESSAGES */
#define SOLCLIENT_FLOW_PROP_DEFAULT_BROWSER             SOLCLIENT_PROP_DISABLE_VAL /**< The default value for the ::SOLCLIENT_FLOW_PROP_BROWSER property. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_ACTIVE_FLOW_IND     SOLCLIENT_PROP_DISABLE_VAL /**< The default value for the ::SOLCLIENT_FLOW_PROP_ACTIVE_FLOW_IND property. */
#define SOLCLIENT_FLOW_PROP_DEFAULT_REPLAY_START_LOCATION ""   /**< The default value for ::SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION is no replay requested. */
/*@}*/

/** @name Flow Bind Entities
 */
/*@{*/
#define SOLCLIENT_FLOW_PROP_BIND_ENTITY_SUB             "1"     /**< A bind target of subscriber. */
#define SOLCLIENT_FLOW_PROP_BIND_ENTITY_QUEUE           "2"     /**< A bind target of Queue. */
#define SOLCLIENT_FLOW_PROP_BIND_ENTITY_TE              "3"     /**< A bind target of Topic Endpoint. */
#define SOLCLIENT_FLOW_PROP_BIND_ENTITY_DTE             SOLCLIENT_FLOW_PROP_BIND_ENTITY_TE     /**< Deprecated name; ::SOLCLIENT_FLOW_PROP_BIND_ENTITY_TE is preferred */
/*@}*/

/** @name Flow Acknowledgment Modes
 */
/*@(*/
#define SOLCLIENT_FLOW_PROP_ACKMODE_AUTO               "1"      /**< Automatic application acknowledgment of all received messages. If application calls ::solClient_flow_sendAck() in the ::SOLCLIENT_FLOW_PROP_ACKMODE_AUTO mode, a warning is generated. */
#define SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT             "2"      /**< Client must call solClient_flow_sendAck() to acknowledge the msgId specified. */
/*@}*/

/** @anchor forwardingModes
 *  @name Flow Forwarding Modes 
 *
 * By default, all Guaranteed messages are stored in the Solace messaging appliance before being forwarded
 * (::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_STORE_AND_FORWARD). In this manner, when publishers receive a Guaranteed delivery
 * acknowledgment, it is assured that all matching endpoint subscribers can eventually receive the message. Similiarly subscribers currently 
 * unbound from the topic-endpoint or queue, will receive the Guaranteed message after the next bind.
 *
 * Beginning with release 5.3 of the Solace messaging appliance, it is possible to configure an endpoint connection as 
 * ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_CUT_THROUGH. The Session capability, ::SOLCLIENT_SESSION_CAPABILITY_CUT_THROUGH is
 * true on Sessions that support this mode. In this manner, published messages are forwarded by the appliance as
 * soon as they are received and adding the message to persistent storage is undertaken in parallel. This method of operation
 * can significantly reduce end-to-end latency on message delivery.
 * 
 * However, applications must be aware that the message store operation
 * may have not succeeded. The publisher application may be notified of a publishing failure when the subscribing application successfully 
 * receives the message.  This can lead to duplication if the publishers chooses to resend the failed message.  It also means that there is
 * no guarantee that a received message is available in persistent store on endpoint. So if a receiving applications chooses to discard the 
 * received message, unbinding (::solClient_flow_destroy())  and rebinding (::solClient_session_createFlow()) to the Flow is not guaranteed to
 * redeliver the discard message such as would be the case in ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_STORE_AND_FORWARD mode.
 *
 * Further, ordering and delivering requirements place further limits on the use of ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_CUT_THROUGH. 
 * A Session may have only one ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_CUT_THROUGH configured Flow. With one Flow in 
 * this mode, the same Session can support any number of parallel Flows that do <b>not</b> set 
 * ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_CUT_THROUGH (that is, in the default forwarding mode, ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE_STORE_AND_FORWARD).
 */
#define SOLCLIENT_FLOW_PROP_FORWARDING_MODE_STORE_AND_FORWARD  "1"  /**< The appliance stores all messages in persistent storage before forwarding on the Flow */
#define SOLCLIENT_FLOW_PROP_FORWARDING_MODE_CUT_THROUGH  "2"  /**< The appliance stores all messages in persistent storage after forwarding on the Flow */

/*@{*/
/** @anchor endpointProps
 *  @name Endpoint Properties
 * Endpoint properties are passed to solClient_session_endpointProvision()/solClient_session_endpointDeprovision(). The 
 * properties describe the endpoint (Queue or Topic Endpoint) to be created or destroyed on the target appliance.
 *
 * Endpoint properties can be used to describe a non-durable endpoint (Queue or Topic Endpoint) in 
 * solClient_session_createFlow(). 
 *
 * Endpoint properties are also used to identify in the endpoint (Queue or ClientName) in 
 * solClient_session_endpointTopicSubscribe() and solClient_session_endpointTopicUnsubscribe(). Only in this interface 
 * is the special endpoint, ::SOLCLIENT_ENDPOINT_PROP_CLIENT_NAME a valid option. Authorized Sessions can add a
 * subscription to queues or to other client Sessions.
 * 
 *
 * Items that can be configured for a create endpoint operation.
 */
#define SOLCLIENT_ENDPOINT_PROP_ID              "ENDPOINT_ID"             /**< The type of endpoint, the valid values are ::SOLCLIENT_ENDPOINT_PROP_QUEUE, ::SOLCLIENT_ENDPOINT_PROP_TE, and ::SOLCLIENT_ENDPOINT_PROP_CLIENT_NAME. Default: ::SOLCLIENT_ENDPOINT_PROP_TE */
#define SOLCLIENT_ENDPOINT_PROP_NAME            "ENDPOINT_NAME"           /**< The name of the Queue or Topic endpoint as a NULL-terminated UTF-8 encoded string. */
#define SOLCLIENT_ENDPOINT_PROP_DURABLE         "ENDPOINT_DURABLE"        /**< The durability of the endpoint to name. Default: ::SOLCLIENT_PROP_ENABLE_VAL, which means the endpoint is durable. Only ::SOLCLIENT_PROP_ENABLE_VAL is supported in solClient_session_endpointProvision(). This property is ignored in solClient_session_creatFlow(). */
#define SOLCLIENT_ENDPOINT_PROP_PERMISSION      "ENDPOINT_PERMISSION"     /**< The created entity's permissions, a single character string. Permissions can be ::SOLCLIENT_ENDPOINT_PERM_DELETE, ::SOLCLIENT_ENDPOINT_PERM_MODIFY_TOPIC, ::SOLCLIENT_ENDPOINT_PERM_CONSUME, ::SOLCLIENT_ENDPOINT_PERM_READ_ONLY, ::SOLCLIENT_ENDPOINT_PERM_NONE. */
#define SOLCLIENT_ENDPOINT_PROP_ACCESSTYPE      "ENDPOINT_ACCESSTYPE"     /**< Sets the access type for the endpoint. This applies to durable Queues only. */
#define SOLCLIENT_ENDPOINT_PROP_QUOTA_MB        "ENDPOINT_QUOTA_MB"       /**< Maximum quota (in megabytes) for the endpoint. The valid range is 1 through 800000.
* 
* A value of 0 configures the endpoint to act as a Last-Value-Queue (LVQ), where the appliance enforces a Queue depth of one, and only the most current message is spooled by the endpoint. When a new message is received, the current queued message is automatically deleted from the endpoint and the new message is spooled.*/
#define SOLCLIENT_ENDPOINT_PROP_MAXMSG_SIZE     "ENDPOINT_MAXMSG_SIZE"    /**< Maximum size (in bytes) for any one message stored in the endpoint. */
#define SOLCLIENT_ENDPOINT_PROP_RESPECTS_MSG_TTL "ENDPOINT_RESPECTS_MSG_TTL"  /**< The endpoint observes message Time-to-Live (TTL) values and can remove expired messages. Default: ::SOLCLIENT_ENDPOINT_PROP_DEFAULT_RESPECTS_MSG_TTL */
#define SOLCLIENT_ENDPOINT_PROP_DISCARD_BEHAVIOR "ENDPOINT_DISCARD_BEHAVIOR" /**< When a message cannot be added to an endpoint (for example, maximum quota (::ENDPOINT_QUOTA_MB) exceeded), this property controls the action the appliance will perform towards the publisher. */
#define SOLCLIENT_ENDPOINT_PROP_MAXMSG_REDELIVERY "ENDPOINT_MAXMSG_REDELIVERY" /**< Defines how many message redelivery retries before discarding or moving the message to the DMQ. The valid ranges is {0..255} where 0 means retry forever. Default: 0 */
/*@}*/

/** @name Default Endpoint Configuration Properties
*/
/*@{*/
#define SOLCLIENT_ENDPOINT_PROP_DEFAULT_ID       SOLCLIENT_ENDPOINT_PROP_TE    /**< The endpoint type of the endpoint. */
#define SOLCLIENT_ENDPOINT_PROP_DEFAULT_DURABLE  SOLCLIENT_PROP_ENABLE_VAL     /**< Whether the endpoint is durable. */
#define SOLCLIENT_ENDPOINT_PROP_DEFAULT_RESPECTS_MSG_TTL SOLCLIENT_PROP_DISABLE_VAL /**< Whether the endpoint observes message Time-to-Live values and can remove expired messages. */
/*@}*/

/** @name Endpoint Naming Entities, used as values for ENDPOINT properties in 
 * solClient_session_endpointProvision()/solClient_session_endpointDeprovision(), in solClient_session_createFlow(), and
 * in solClient_session_endpointTopicSubscribe() / solClient_session_endpointTopicUnsubscribe().
*/
/*@{*/
#define SOLCLIENT_ENDPOINT_PROP_QUEUE           "2"     /**< Request is for a Queue. */
#define SOLCLIENT_ENDPOINT_PROP_TE              "3"     /**< Request is for a Topic Endpoint. */
#define SOLCLIENT_ENDPOINT_PROP_CLIENT_NAME     "4"     /**< Request is for a Client name  (solClient_session_endpointTopicSubscribe() / solClient_session_endpointTopicUnsubscribe() only.) */
/*@}*/

/** @name Endpoint Naming Entities, used as values for ENDPOINT properties in solClient_session_endpointProvision().
*/
/*@{*/
#define SOLCLIENT_ENDPOINT_PROP_ACCESSTYPE_NONEXCLUSIVE        "0"     /**< A non-exclusive (shared) Queue. Each client to bind receives messages in a round robin fashion. */
#define SOLCLIENT_ENDPOINT_PROP_ACCESSTYPE_EXCLUSIVE           "1"     /**< An exclusive Queue. The first client to bind receives the stored messages on the Endpoint. */
/*@}*/


/** @name Endpoint Permissions 
 */
/*@(*/
#define SOLCLIENT_ENDPOINT_PERM_NONE           "n"     /**< No permissions for other clients */
#define SOLCLIENT_ENDPOINT_PERM_READ_ONLY      "r"     /**< Read-only permission — other clients may not consume messages. */
#define SOLCLIENT_ENDPOINT_PERM_CONSUME        "c"     /**< Consumer permission — other clients may read and consume messages. */
#define SOLCLIENT_ENDPOINT_PERM_MODIFY_TOPIC   "m"     /**< Modify Topic permission — other clients may read and consume messages, and modify Topic on a Topic Endpoint. */
#define SOLCLIENT_ENDPOINT_PERM_DELETE         "d"     /**< Delete permission — other clients may read and consume messages, modify the Topic on a Topic Endpoint, and delete the endpoint.  */
/*@}*/

/** @name Endpoint Discard Msg Behavior */
/*@(*/
#define SOLCLIENT_ENDPOINT_PROP_DISCARD_NOTIFY_SENDER_ON      "1"       /**< Send the publisher a message reject notification ::SOLCLIENT_SESSION_EVENT_REJECTED_MSG_ERROR. */
#define SOLCLIENT_ENDPOINT_PROP_DISCARD_NOTIFY_SENDER_OFF     "2"       /**< Discard the message and acknowledge it. */
/*@}*/


/*@(*/
/** @anchor provisionflags
 *  @name Provision Flags
 * The provision operation may be modified by the use of one or more of the following flags:
 */

#define SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM        (0x01)  /**< The provision operation blocks until it has completed successfully on the appliance or failed. */
#define SOLCLIENT_PROVISION_FLAGS_IGNORE_EXIST_ERRORS   (0x02)  /**< When set, it is not considered an error if the endpoint already exists (create) or does not exist (delete). */
/*@}*/

/** @name Replay Start Location */
/*@(*/
#define SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION_BEGINNING      "BEGINNING"   /**< Replay all messages in the replay log */
/*@}*/

/**
* @anchor sessioncapabilities
* @name Session Capabilities
* The capabilities of the Session after connecting to a peer. Capabilities can vary depending on 
* the appliance platform or peer connection. Capabilities can be retrieved with the ::solClient_session_getCapability function.
*/
#define SOLCLIENT_SESSION_CAPABILITY_PUB_GUARANTEED                 "SESSION_CAPABILITY_PUB_GUARANTEED"   /**< Boolean - The Session allows publishing of Guaranteed messages. */
#define SOLCLIENT_SESSION_CAPABILITY_SUB_FLOW_GUARANTEED            "SESSION_CAPABILITY_SUB_FLOW_GUARANTEED" /**< Boolean - The Session allows binding a Guaranteed Flow to an endpoint. */
#define SOLCLIENT_SESSION_CAPABILITY_BROWSER                        "SESSION_CAPABILITY_BROWSER"      /**< Boolean - The Session allows binding to a Queue as a Browser.*/
#define SOLCLIENT_SESSION_CAPABILITY_TEMP_ENDPOINT                  "SESSION_CAPABILITY_TEMP_ENDPOINT"   /**< Boolean - The Session allows the creation of temporary endpoints (Queues / TopicEndpoints). */
#define SOLCLIENT_SESSION_CAPABILITY_JNDI                           "SESSION_CAPABILITY_JNDI"        /**< Boolean - The Session accepts JNDI queries. */
#define SOLCLIENT_SESSION_CAPABILITY_COMPRESSION                    "SESSION_CAPABILITY_COMPRESSION" /**< Boolean - The Session accepts compressed (DEFLATE) data. */
#define SOLCLIENT_SESSION_CAPABILITY_SELECTOR                       "SESSION_CAPABILITY_SELECTOR"    /**< Boolean - The Session supports a selector on Flows. */
#define SOLCLIENT_SESSION_CAPABILITY_ENDPOINT_MANAGEMENT            "SESSION_CAPABILITY_ENDPOINT_MANAGEMENT"   /**< The Session is allowed to create/delete durable endpoints dynamically. */
#define SOLCLIENT_SESSION_PEER_PLATFORM                             "SESSION_PEER_PLATFORM"    /**< String - A appliance/peer returned string that describes the hardware platform. */ 
#define SOLCLIENT_SESSION_PEER_SOFTWARE_DATE                        "SESSION_PEER_SOFTWARE_DATE"        /**< String - A appliance/peer returned string that contains the release date for the appliance software. */
#define SOLCLIENT_SESSION_PEER_SOFTWARE_VERSION                     "SESSION_PEER_SOFTWARE_VERSION"     /**< String - A appliance/peer returned string that contains the version information. */
#define SOLCLIENT_SESSION_PEER_PORT_SPEED                           "SESSION_PEER_PORT_SPEED"           /**< Uint32 - The maximum rate (in Megabits/second) supported by the appliance/peer port. */
#define SOLCLIENT_SESSION_PEER_PORT_TYPE                            "SESSION_PEER_PORT_TYPE"            /**< String - The appliance/peer port type. */
#define SOLCLIENT_SESSION_PEER_ROUTER_NAME                          "SESSION_PEER_ROUTER_NAME"          /**< String - The appliance/peer Appliance Name, typically used to direct management requests. */
#define SOLCLIENT_SESSION_CAPABILITY_MAX_GUARANTEED_MSG_SIZE        "SESSION_CAPABILITY_MAX_GUARANTEED_MSG_SIZE"  /**< Uint32 - The maximum size (in bytes) of a Guaranteed message, including all optional message headers and data. */
#define SOLCLIENT_SESSION_CAPABILITY_MAX_DIRECT_MSG_SIZE            "SESSION_CAPABILITY_MAX_DIRECT_MSG_SIZE"   /**< Uint32 - The maximum size (in bytes) of a Direct message, including all optional message headers and data. */
#define SOLCLIENT_SESSION_CAPABILITY_ENDPOINT_MESSAGE_TTL           "SESSION_CAPABILITY_ENDPOINT_MESSAGE_TTL"  /**< Boolean - The Session supports message Time-to-Live (TTL) (this only applies to messages that are spooled) in published messages. */
#define SOLCLIENT_SESSION_CAPABILITY_QUEUE_SUBSCRIPTIONS            "SESSION_CAPABILITY_QUEUE_SUBSCRIPTIONS"   /**< Boolean - The Session supports adding subscription to durable and non-durable queues on the peer. */
#define SOLCLIENT_SESSION_CAPABILITY_SUBSCRIPTION_MANAGER           "SESSION_CAPABILITY_SUBSCRIPTION_MANAGER"  /**< Boolean - The appliance supports adding subscriptions on behalf of other client names.*/
#define SOLCLIENT_SESSION_CAPABILITY_MESSAGE_ELIDING                "SESSION_CAPABILITY_MESSAGE_ELIDING"   /**< Boolean - The Session supports message eliding. */
#define SOLCLIENT_SESSION_CAPABILITY_NO_LOCAL                       "SESSION_CAPABILITY_NO_LOCAL"          /**< Boolean - The Session supports No Local. Flows may be created to Queues and Topic Endpoints that will not receive messages published on the same Session. */
#define SOLCLIENT_SESSION_CAPABILITY_PER_TOPIC_SEQUENCE_NUMBERING   "SESSION_CAPABILITY_PER_TOPIC_SEQUENCE_NUMBERING"   /**< Boolean - The peer can insert per Topic sequence numbers. */
#define SOLCLIENT_SESSION_CAPABILITY_ENDPOINT_DISCARD_BEHAVIOR      "SESSION_CAPABILITY_ENDPOINT_DISCARD_BEHAVIOR"  /**< Boolean - The peer endpoints can be provisioned with discard behavior */
#define SOLCLIENT_SESSION_CAPABILITY_CUT_THROUGH                    "SESSION_CAPABILITY_CUT_THROUGH"      /**< Boolean - The peer supports the cut-through forwarding mode for Guaranteed message delivery. */
#define SOLCLIENT_SESSION_CAPABILITY_ACTIVE_FLOW_INDICATION         "SESSION_CAPABILITY_ACTIVE_FLOW_INDICATION"   /**< Boolean - TRUE if session supports active flow indication parameter */
#define SOLCLIENT_SESSION_CAPABILITY_TRANSACTED_SESSION             "SESSION_CAPABILITY_TRANSACTED_SESSION" /**< Boolean - The Session allows Guaranteed Data Transacted Sessions. */
#define SOLCLIENT_SESSION_CAPABILITY_OPENMAMA                       "SESSION_CAPABILITY_OPENMAMA"           /**< Boolean - The Session allows the OpenMAMA API to be used. */
#define SOLCLIENT_SESSION_CAPABILITY_MESSAGE_REPLAY                 "SESSION_CAPABILITY_MESSAGE_REPLAY"     /**< Boolean - The Session allow Message Replay on flow create */
#define SOLCLIENT_SESSION_CAPABILITY_COMPRESSED_SSL                 "SESSION_CAPABILITY_COMPRESSED_SSL" /**< Boolean - The peer can support ssl downgrade to compression. */
#define SOLCLIENT_SESSION_CAPABILITY_LONG_SELECTORS                 "SESSION_CAPABILITY_LONG_SELECTORS" /**< Boolean - The peer can support selectors longer than 1023 bytes */
/*@}*/

/** @name TransactedSessionProps
 */
/*@{*/
#define SOLCLIENT_TRANSACTEDSESSION_PROP_HAS_PUBLISHER "TRANSACTEDSESSION_HAS_PUBLISHER" /**<If it is enabled, a publisher flow is created when a Transacted Session is created successfully. Default: ::SOLCLIENT_TRANSACTEDSESSION_PROP_DEFAULT_HAS_PUBLISHER. */
#define SOLCLIENT_TRANSACTEDSESSION_PROP_CREATE_MESSAGE_DISPATCHER "TRANSACTEDSESSION_CREATE_MESSAGE_DISPATCHER" /**<If it is enabled, a TransactedSession-bound Message Dispatcher is lazily created for asynchronous message delivery within a Transacted Session. Default: ::SOLCLIENT_TRANSACTEDSESSION_PROP_DEFAULT_CREATE_MESSAGE_DISPATCHER */  
#define SOLCLIENT_TRANSACTEDSESSION_PROP_REQUESTREPLY_TIMEOUT_MS "TRANSACTEDSESSION_REQUESTREPLY_TIMEOUT_MS" /**< Timeout (in milliseconds) to wait for a response. The minimum configuration value is 1000. Default: ::SOLCLIENT_TRANSACTEDSESSION_PROP_DEFAULT_REQUESTREPLY_TIMEOUT_MS. */

/*@}*/

/** @name DefaultTransactedSessionProps
 */
/*@{*/
#define SOLCLIENT_TRANSACTEDSESSION_PROP_DEFAULT_HAS_PUBLISHER SOLCLIENT_PROP_ENABLE_VAL /**<By default, a publisher flow is automatically created when a Transacted Session is created. */
#define SOLCLIENT_TRANSACTEDSESSION_PROP_DEFAULT_CREATE_MESSAGE_DISPATCHER SOLCLIENT_PROP_DISABLE_VAL /**<By default, the default Context-bound Message Dispatcher is used for asynchronous message delivery within a transacted session. */
#define SOLCLIENT_TRANSACTEDSESSION_PROP_DEFAULT_REQUESTREPLY_TIMEOUT_MS  "10000" /**<The default Transacted Session request timer in milliseconds. */
/*@}*/

/**
* @struct solClient_errorInfo
*
* The structure used to record more detailed error information for a failed API call.
* @see ::solClient_subCode for a description of all subcodes.
*/

#define SOLCLIENT_ERRORINFO_STR_SIZE (256) /**< The maximum size of error string including terminating NULL character. */

  typedef struct solClient_errorInfo
  {
    solClient_subCode_t subCode;                                              /**< A subcode indicating the type of error. */
    solClient_session_responseCode_t responseCode;                            /**< A response code that is returned for some subcodes; otherwise zero. */
    char errorStr[SOLCLIENT_ERRORINFO_STR_SIZE];                              /**< An information string (max length ::SOLCLIENT_ERRORINFO_STR_SIZE) for certain types of subcodes (empty string, if not used). */
  } solClient_errorInfo_t, *solClient_errorInfo_pt; /**< A pointer to a ::solClient_errorInfo structure returned from ::solClient_getLastErrorInfo() .*/

/**
* @struct solClient_bufInfo
* A structure used to point to a message part and indicate the size of that part
* (in bytes). 
*/
  typedef struct solClient_bufInfo
  {
    void *buf_p;                  /**< A pointer to buffer. */
    solClient_uint32_t bufSize;   /**< The number of valid bytes in buffer. */
  } solClient_bufInfo_t;
  typedef solClient_bufInfo_t *solClient_bufInfo_pt; /**< A pointer to ::solClient_bufInfo structure used to point to a message part and to indicate the size of that part (in bytes). */




/** @name Limits on sizes of message portions
*/
/*@{*/
#define SOLCLIENT_BUFINFO_MAX_USER_DATA_SIZE (36)          /**< The maximum size allowed for the user-data portion. */
#define SOLCLIENT_BUFINFO_MAX_CORRELATION_TAG_SIZE (16)    /**< The maximum size allowed for the correlation tag portion. */
#define SOLCLIENT_BUFINFO_MAX_TOPIC_SIZE     (250)         /**< The maximum size allowed for the topic portion (not including the terminating NULL). */
#define SOLCLIENT_BUFINFO_MAX_QUEUENAME_SIZE (250)         /**< The maximum size allowed for the Queue name portion (not including the terminating NULL). */
#define SOLCLIENT_BUFINFO_MAX_DURABLE_QUEUENAME_SIZE (200) /**< The maximum size allowed for a durable Queue name (not including the terminating NULL). */
#define SOLCLIENT_SESSION_SEND_MULTIPLE_LIMIT 50           /**> The maximum number of messages which can be sent through a single call to solClient_session_sendMultipleMsg()*/
/*@}*/

  typedef solClient_uint64_t solClient_stats_t,  /**< Type of a statistics value. */
   *solClient_stats_pt;                          /**< Type of a pointer to a statistics value. */

/**
* @enum solClient_stats_rx
* Receive statistics (64-bit counters). Index into array of receive statistics.
*/
  typedef enum solClient_stats_rx
  {
    SOLCLIENT_STATS_RX_DIRECT_BYTES                    = 0,  /**< The number of bytes received. */
    SOLCLIENT_STATS_RX_BYTES                           = SOLCLIENT_STATS_RX_DIRECT_BYTES, /**< Deprecated name, ::SOLCLIENT_STATS_RX_DIRECT_BYTES is preferred */
    SOLCLIENT_STATS_RX_DIRECT_MSGS                     = 1,  /**< The number of messages received. */
    SOLCLIENT_STATS_RX_MSGS                            = SOLCLIENT_STATS_RX_DIRECT_MSGS,  /**< Deprecated name, ::SOLCLIENT_STATS_RX_DIRECT_MSGS is preferred. */
    SOLCLIENT_STATS_RX_READS                           = 2,  /**< The number of non-empty reads. */
    SOLCLIENT_STATS_RX_DISCARD_IND                     = 3,  /**< The number of receive messages with discard indication set. */
    SOLCLIENT_STATS_RX_DISCARD_SMF_UNKNOWN_ELEMENT     = 4,  /**< The number of messages discarded due to the presence of an unknown element or unknown protocol in the Solace Message Format (SMF) header. */
    SOLCLIENT_STATS_RX_DISCARD_MSG_HDR_ERROR           = SOLCLIENT_STATS_RX_DISCARD_SMF_UNKNOWN_ELEMENT,   /**< Deprecated, use the more accurately named SOLCLIENT_STATS_RX_DISCARD_SMF_UNKNOWN_ELEMENT instead. */
    SOLCLIENT_STATS_RX_DISCARD_MSG_TOO_BIG             = 5,  /**< The number of messages discarded due to msg too large. */
    SOLCLIENT_STATS_RX_ACKED                           = 6,  /**< The number of acknowledgments sent for Guaranteed messages. */
    SOLCLIENT_STATS_RX_DISCARD_DUPLICATE               = 7,  /**< The number of Guaranteed messages dropped for being duplicates. */
    SOLCLIENT_STATS_RX_DISCARD_NO_MATCHING_FLOW        = 8,  /**< The number of Guaranteed messages discarded due to no match on the flowId. */
    SOLCLIENT_STATS_RX_DISCARD_OUTOFORDER              = 9,  /**< The number of Guaranteed messages discarded for being received out of order. */
    SOLCLIENT_STATS_RX_PERSISTENT_BYTES                = 10, /**< The number of Persistent bytes received on the Flow. On the Session, it is the total number of Persistent bytes received across all Flows. */
    SOLCLIENT_STATS_RX_PERSISTENT_MSGS                 = 11, /**< The number of Persistent messages received on the Flow. On the Session, it is the total number of Persistent messages received across all Flows. */
    SOLCLIENT_STATS_RX_NONPERSISTENT_BYTES             = 12, /**< The number of Persistent bytes received on the Flow. On the Session, it is the total number of Persistent bytes received across all Flows. */
    SOLCLIENT_STATS_RX_NONPERSISTENT_MSGS              = 13, /**< The number of Persistent messages received on the Flow. On the Session, it is the total number of Persistent messages received across all Flows. */
    SOLCLIENT_STATS_RX_CTL_MSGS                        = 14, /**< The number of control (non-data) messages received. */
    SOLCLIENT_STATS_RX_CTL_BYTES                       = 15, /**< The number of bytes received in control (non-data) messages. */
    SOLCLIENT_STATS_RX_TOTAL_DATA_BYTES                = 16,  /**< The total number of data bytes received. */
    SOLCLIENT_STATS_RX_TOTAL_DATA_MSGS                 = 17,  /**< The total number of data messages received. */
    SOLCLIENT_STATS_RX_COMPRESSED_BYTES                = 18, /**< The number of bytes received before decompression. */
    SOLCLIENT_STATS_RX_REPLY_MSG                       = 19, /**<  The reply messages received. */
    SOLCLIENT_STATS_RX_REPLY_MSG_DISCARD               = 20, /**<  The reply messages (including cache request response) discarded due to errors in response format or no outstanding request.*/
    SOLCLIENT_STATS_RX_CACHEREQUEST_OK_RESPONSE        = 21, /**< Cache requests completed OK. */
    SOLCLIENT_STATS_RX_CACHEREQUEST_FULFILL_DATA       = 22, /**< Cache requests fulfilled by live data. */
    SOLCLIENT_STATS_RX_CACHEREQUEST_ERROR_RESPONSE     = 23, /**< Cache requests failed due to solCache error response. */
    SOLCLIENT_STATS_RX_CACHEREQUEST_DISCARD_RESPONSE   = 24, /**< Cache request response discarded due to errors in response format or no outstanding cache request. */
    SOLCLIENT_STATS_RX_CACHEMSG                        = 25, /**< Cached messages delivered to application. */
    SOLCLIENT_STATS_RX_FOUND_CTSYNC                    = 26, /**< On a cut-through Flow, the number of times the Flow entered cut-through delivery mode. */
    SOLCLIENT_STATS_RX_LOST_CTSYNC                     = 27, /**< On a cut-through Flow, the number of times the Flow left cut-through delivery mode to resynchronize with the Guaranteed message storage on the appliance */
    SOLCLIENT_STATS_RX_LOST_CTSYNC_GM                  = 28, /**< On a cut-through Flow, the number of times the Flow left cut-through delivery mode to resynchronize with the Guaranteed message storage due to receiving a Guaranteed message that was not previously received as Direct. */
    SOLCLIENT_STATS_RX_OVERFLOW_CTSYNC_BUFFER          = 29, /**< On a cut-through Flow, the number of times the synchronization buffer overflowed, delaying synchronization. */
    SOLCLIENT_STATS_RX_ALREADY_CUT_THROUGH             = 30, /**< On a cut-through Flow, the number of Guaranteed messages discarded because they had already been received on the cut-through Flow.*/
    SOLCLIENT_STATS_RX_DISCARD_FROM_CTSYNC             = 31, /**< On a cut-through Flow, the number of messages discarded from the synchronization list other than those discarded due to overflow. */
    SOLCLIENT_STATS_RX_DISCARD_MSG_FLOW_UNBOUND_PENDING = 32, /**< On a transacted flow, the number of messages discarded because the flow is in a UNBOUND pending state. */
    SOLCLIENT_STATS_RX_DISCARD_MSG_TRANSACTION_ROLLBACK = 33, /**< On a transacted flow, the number of messages discarded after a transaction rollback and becomes a message comes in with prevMsgId=0. */
    SOLCLIENT_STATS_RX_DISCARD_TRANSACTION_RESPONSE     = 34, /**< On a transacted session, the number of transaction responses discarded due to reconnection. */
    SOLCLIENT_STATS_RX_SSL_READ_EVENTS                  = 35,
    SOLCLIENT_STATS_RX_SSL_READ_CALLS                   = 36,
    SOLCLIENT_STATS_RX_NUM_STATS                       = 37  /**< The size of receive stats array. */
  } solClient_stats_rx_t; /**< Type that indicates which receive statistic. */

/**
* @enum solClient_stats_tx
* Transmit statistics (64-bit counters). Index into array of transmit statistics.
*/
  typedef enum solClient_stats_tx
  {
    SOLCLIENT_STATS_TX_TOTAL_DATA_BYTES                = 0,  /**< The number of data bytes transmitted in total. */
    SOLCLIENT_STATS_TX_BYTES                           = SOLCLIENT_STATS_TX_TOTAL_DATA_BYTES, /**< Deprecated name; ::SOLCLIENT_STATS_TX_TOTAL_DATA_BYTES is preferred */
    SOLCLIENT_STATS_TX_TOTAL_DATA_MSGS                 = 1,  /**< The number of data messages transmitted in total. */
    SOLCLIENT_STATS_TX_MSGS                            = SOLCLIENT_STATS_TX_TOTAL_DATA_MSGS, /**< Deprecated name; ::SOLCLIENT_STATS_TX_TOTAL_DATA_MSGS is preferred */
    SOLCLIENT_STATS_TX_WOULD_BLOCK                     = 2,  /**< The number of messages not accepted due to would block (non-blocking only). */
    SOLCLIENT_STATS_TX_SOCKET_FULL                     = 3,  /**< The number of times the socket was full when send done (data buffered). */
    SOLCLIENT_STATS_TX_DIRECT_BYTES                    = 4,  /**< The number of bytes transmitted in Direct messages. */
    SOLCLIENT_STATS_TX_DIRECT_MSGS                     = 5,  /**< The number of Direct messages transmitted. */
    SOLCLIENT_STATS_TX_PERSISTENT_BYTES                = 6,  /**< The number of bytes transmitted in Persistent messages. */
    SOLCLIENT_STATS_TX_NONPERSISTENT_BYTES             = 7,  /**< The number of bytes transmitted in Non-Persistent messages. */
    SOLCLIENT_STATS_TX_PERSISTENT_MSGS                 = 8,  /**< The number of Persistent messages transmitted. */
    SOLCLIENT_STATS_TX_NONPERSISTENT_MSGS              = 9,  /**< The number of Non-Persistent messages transmitted. */
    SOLCLIENT_STATS_TX_PERSISTENT_REDELIVERED          = 10, /**< The number of Persistent messages redelivered. */
    SOLCLIENT_STATS_TX_NONPERSISTENT_REDELIVERED       = 11, /**< The number of Non-Persistent messages redelivered. */
    SOLCLIENT_STATS_TX_PERSISTENT_BYTES_REDELIVERED    = 12, /**< The number of bytes redelivered in Persistent messages. */
    SOLCLIENT_STATS_TX_NONPERSISTENT_BYTES_REDELIVERED = 13, /**< The number of bytes redelivered in Non-Persistent messages. */
    SOLCLIENT_STATS_TX_ACKS_RXED                       = 14, /**< The number of acknowledgments received. */
    SOLCLIENT_STATS_TX_WINDOW_CLOSE                    = 15, /**< The number of times the transmit window closed. */
    SOLCLIENT_STATS_TX_ACK_TIMEOUT                     = 16, /**< The number of times the acknowledgment timer expired. */
    SOLCLIENT_STATS_TX_CTL_MSGS                        = 17, /**< The number of control (non-data) messages transmitted. */
    SOLCLIENT_STATS_TX_CTL_BYTES                       = 18, /**< The number of bytes transmitted in control (non-data) messages. */
    SOLCLIENT_STATS_TX_COMPRESSED_BYTES                = 19, /**< The number of bytes transmitted after compression. */
    SOLCLIENT_STATS_TX_TOTAL_CONNECTION_ATTEMPTS       = 20, /**< The total number of TCP connections attempted by this Session. */
    SOLCLIENT_STATS_TX_REQUEST_SENT                    = 21, /**< The request messages sent. */
    SOLCLIENT_STATS_TX_REQUEST_TIMEOUT                 = 22, /**< The request messages sent that did not receive a reply due to timeout. */
    SOLCLIENT_STATS_TX_CACHEREQUEST_SENT               = 23, /**< The cache requests sent. */
    SOLCLIENT_STATS_TX_GUARANTEED_MSGS_SENT_CONFIRMED  = 24, /**< Guaranteed messages (Persistent/Non-Persistent) published that have been acknowledged. */
    SOLCLIENT_STATS_TX_DISCARD_NO_MATCH                = 25, /**< When the IPC add-on is in use, the counter of messages discarded due to no subscription match with connected peers */
    SOLCLIENT_STATS_TX_DISCARD_CHANNEL_ERROR           = 26, /**< Messages discarded due to channel failure */
    SOLCLIENT_STATS_TX_BLOCKED_ON_SEND                 = 27, /**< The number of times Session blocked on socket full (blocking only) occurred. */
    SOLCLIENT_STATS_TX_NUM_STATS                       = 28  /**< The size of transmit stats array. */
  } solClient_stats_tx_t; /**< Type that indicates which transmit statistic. */


/**
* @struct solClient_session_eventCallbackInfo
*
* A structure that is returned (as a pointer) for each event in the event 
* callback for a Session. A structure is used so that new fields
* can be added in the future without affecting existing applications.
* For a sessionEvent of ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT, info_p
* will be the pointer or value specified in the
* ::SOLCLIENT_BUFINFO_CORRELATION_TAG_PART of the message that is being
* acknowledged. This is used to correlate a published message to
* the acknowledgment received from
* the appliance. In all other events info_p is a pointer to a NULL-terminated 
* string.
*/
  typedef struct solClient_session_eventCallbackInfo
  {
    solClient_session_event_t sessionEvent;         /**< The Session event that has occurred. */
    solClient_session_responseCode_t responseCode;  /**< A response code that is returned for some events, otherwise zero. */
    const char *info_p;                             /**< Except for ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT (see Detailed Description above), a pointer to a NULL-terminated string providing further information about the event, when available. This pointer is never NULL */
    void       *correlation_p;                      /**< Application-supplied correlation pointer where applicable. Used when acknowledging or rejecting Guaranteed messages, in responses to any function calls that pass a correlationTag that will be returned in a Session Event. */
  } solClient_session_eventCallbackInfo_t, *solClient_session_eventCallbackInfo_pt; /**< A pointer to ::solClient_session_eventCallbackInfo structure of information returned with a Session event. */


/**
* @struct solClient_flow_eventCallbackInfo
*
* A structure that is returned (as a pointer) for each event in the event callback
* callback for a Flow. A structure is used so that new fields can be added
* in the future without affecting existing applications.
*/
  typedef struct solClient_flow_eventCallbackInfo
  {
    solClient_flow_event_t flowEvent;         /**< The Session event that has occurred. */
    solClient_session_responseCode_t responseCode;  /**< A response code that is returned for some events; otherwise zero. */
    const char *info_p;                             /**< A pointer to a NULL-terminated string providing further information about the event, when available. This pointer is never NULL. */
  } solClient_flow_eventCallbackInfo_t, *solClient_flow_eventCallbackInfo_pt; /**< A pointer to ::solClient_flow_eventCallbackInfo structure of information returned with a Session event. */

/**
* A callback prototype for Session events. The callback with this prototype that is
* registered for a Session is called for each Session event that occurs.
* 
* When a Session event is received that indicates an error condition (such as the Session going down), additional
* error information is recorded and can be retrieved through ::solClient_getLastErrorInfo.
* @subcodes
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @li ::SOLCLIENT_SUBCODE_OUT_OF_RESOURCES - The appliance cannot accept any more Topic subscriptions. (This subcode only occurs when using the Topic Routing Blade.)
* @li ::SOLCLIENT_SUBCODE_PROTOCOL_ERROR - A protocol error occurred between the API and the appliance.
* @li ::SOLCLIENT_SUBCODE_KEEP_ALIVE_FAILURE - The Session went down due to a Keep-Alive failure.
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX - A subscription was rejected by the appliance due to invalid Topic syntax.
* @li ::SOLCLIENT_SUBCODE_XML_PARSE_ERROR - The appliance rejected a published XML message due to an XML parse error.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - A timeout occurred on the Session connection.
* @li ::SOLCLIENT_SUBCODE_LOGIN_FAILURE
* @li ::SOLCLIENT_SUBCODE_MSG_VPN_NOT_ALLOWED
* @li ::SOLCLIENT_SUBCODE_MSG_VPN_UNAVAILABLE
* @li ::SOLCLIENT_SUBCODE_CLIENT_USERNAME_IS_SHUTDOWN
* @li ::SOLCLIENT_SUBCODE_DYNAMIC_CLIENTS_NOT_ALLOWED
* @li ::SOLCLIENT_SUBCODE_CLIENT_NAME_ALREADY_IN_USE
* @li ::SOLCLIENT_SUBCODE_INVALID_VIRTUAL_ADDRESS
* @li ::SOLCLIENT_SUBCODE_CLIENT_DELETE_IN_PROGRESS
* @li ::SOLCLIENT_SUBCODE_TOO_MANY_CLIENTS
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ALREADY_PRESENT (see ::SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_NOT_FOUND (see ::SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_INVALID
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_TOO_MANY
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER
* @li ::SOLCLIENT_SUBCODE_CONTROL_OTHER
* @li ::SOLCLIENT_SUBCODE_DATA_OTHER
* @li ::SOLCLIENT_SUBCODE_MESSAGE_TOO_LARGE
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_NAME_FOR_TE
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_QUEUE_NAME
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_TE_NAME
* @li ::SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_QUEUE
* @li ::SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_TE
* @li ::SOLCLIENT_SUBCODE_UNEXPECTED_UNBIND
* @li ::SOLCLIENT_SUBCODE_QUEUE_NOT_FOUND
* @li ::SOLCLIENT_SUBCODE_PUBLISH_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_CLIENT_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_SPOOL_OVER_QUOTA
* @li ::SOLCLIENT_SUBCODE_QUEUE_SHUTDOWN
* @li ::SOLCLIENT_SUBCODE_TE_SHUTDOWN
* @li ::SOLCLIENT_SUBCODE_NO_MORE_NON_DURABLE_QUEUE_OR_TE
* @li ::solClient_subCode for a description of all subcodes.
*
* @param opaqueSession_p A pointer to the Session to which the event applies. This pointer is never NULL.
* @param eventInfo_p     A pointer to information about the Session event, such as the event type. This pointer is never NULL.
* @param user_p          A pointer to opaque user data provided when the callback is registered.
* @see solClient_session_create()
*/
  typedef void (*solClient_session_eventCallbackFunc_t) (solClient_opaqueSession_pt opaqueSession_p, solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p);
                                                                    /**< A callback prototype for Session events. */

/**
* A callback prototype for Flow events. The callback with this prototype that is
* registered for a Flow is called for each Flow event that occurs.
* 
* When a Flow event is received that indicates an error condition (such as a bind failure), additional
* error information is recorded and can be retrieved through ::solClient_getLastErrorInfo. 
* @subcodes
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed
* @li ::SOLCLIENT_SUBCODE_PROTOCOL_ERROR - A protocol error occurred between the API and the appliance.
* @li ::SOLCLIENT_SUBCODE_KEEP_ALIVE_FAILURE - The Session went down due to a Keep-Alive failure.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - A timeout occurred on an operation such as binding to a Flow.
* @li ::solClient_subCode for a description of all subcodes.
*
* @param opaqueFlow_p    A pointer to the Flow to which the event applies. This pointer is never NULL.
* @param eventInfo_p     A pointer to information about the Flow event, such as the event type. This pointer is never NULL.
* @param user_p          A pointer to opaque user data provided when callback is registered.
* @see solClient_session_createFlow()
*/
  typedef void (*solClient_flow_eventCallbackFunc_t) (solClient_opaqueFlow_pt opaqueFlow_p, solClient_flow_eventCallbackInfo_pt eventInfo_p, void *user_p);
                                                                 /**< A callback prototype for Session events. */

/**
* A callback prototype for file descriptor events. The callback with this prototype that is
* registered for a file descriptor event is called for each file descriptor event that occurs.
* Multiple registered file descriptor events can be returned in a single callback.
* @param opaqueContext_p A pointer to the Context under which the file descriptor event occurred. This pointer is never NULL.
* @param fd The file descriptor for which the event has occurred.
* @param events A bit vector containing the file descriptor events that have occurred ORed together.
* @param user_p A pointer to opaque user data, provided when the callback registered.
* @see solClient_context_registerForFdEvents()
*/
  typedef void (*solClient_context_fdCallbackFunc_t) (solClient_opaqueContext_pt opaqueContext_p, solClient_fd_t fd, solClient_fdEvent_t events, void *user_p); /**< Callback prototype for FD events. */

/**
* A callback prototype for received messages. A callback with this prototype 
* can be registered for a Session. If an application registers this callback, it
* is called for each received message.
* When the callback is invoked for a received message, the application can use 
* all the structured data accessors on the received message. It can also choose
* to keep all or part of the received message for later use.
*
* If the application requires a message (or some of its parts) after this 
* callback returns, the application has several options available:
* @li The application may duplicate the message (solClient_msg_dup), which creates a copy of the message but uses the same data area. The message can be used to access the data in the future.
* @li The application may keep the 'msg' entirely by returning ::SOLCLIENT_CALLBACK_TAKE_MSG from this function.
*
* In these cases, the application must call solClient_msg_free() to release the 
* msg that was created by solClient_msg_dup. If the application takes control of the 
* received msg it must later call solClient_msg_free() for that msg.
*
* The implementation of this callback must return ::SOLCLIENT_CALLBACK_OK or ::SOLCLIENT_CALLBACK_TAKE_MSG
* @param opaqueSession_p A pointer to the Session for which the message has been received. This pointer is never NULL.
* @param msg_p A pointer to an opaque message pointer (solClient_opaqueMsg_pt) for the received message. This pointer is never NULL.
* @param user_p A pointer to opaque user data provided when callback registered.
* @see solClient_session_create()
*/
  typedef solClient_rxMsgCallback_returnCode_t
  (*solClient_session_rxMsgCallbackFunc_t) (solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p);

/**
* A callback prototype for received messages on a Flow. A callback with this prototype 
* can be registered for a Flow. If an application registers this callback it
* is called for each received message.
* When the callback is invoked for a received message, the application can use 
* all the structured data accessors on the received message. It may also choose
* to keep all or part of the received message for later use.
*
* If the application requires a message (or some of its parts) after this 
* callback returns, the application has several options available:
* @li The application may duplicate the message (solClient_msg_dup), which creates a copy of the message but uses the same data area. The message can be used to access the data in the future.
* @li The application can keep the 'msg' entirely by returning ::SOLCLIENT_CALLBACK_TAKE_MSG from this function.
*
* In these cases, the application must call solClient_msg_free() to release the 
* msg that was created by solClient_msg_dup. If the application takes control of the 
* received msg it must later call solClient_msg_free() for that msg.
*
* The implementation of this callback must return ::SOLCLIENT_CALLBACK_OK or ::SOLCLIENT_CALLBACK_TAKE_MSG
* @param opaqueFlow_p A pointer to the Flow for which the message has been received. This pointer is never NULL.
* @param msg_p A pointer to an opaque message pointer (solClient_opaqueMsg_pt) for the received message. This pointer is never NULL.
* @param user_p A pointer to opaque user data provided when the callback registered.
* @see solClient_session_create()
*/

  typedef solClient_rxMsgCallback_returnCode_t
  (*solClient_flow_rxMsgCallbackFunc_t) (solClient_opaqueFlow_pt opaqueFlow_p, solClient_opaqueMsg_pt msg_p, void *user_p);

/**
* @struct solClient_log_callbackInfo_t
*
* A structure that is returned (as a pointer) for each log to the log callback routine
* that has been optionally registered. A structure is used so that new fields can be added
* in the future without affecting existing applications.
*/
  typedef struct solClient_log_callbackInfo
  {
    solClient_log_category_t category;  /**< The log category associated with the log being generated. */
    solClient_log_level_t level;        /**< The log level associated with the log being generated. */
    const char *msg_p;                  /**< A pointer to the log message text, which is NULL terminated. This pointer is never NULL. */
  } solClient_log_callbackInfo_t, *solClient_log_callbackInfo_pt; /**< A pointer to ::solClient_log_callbackInfo structure of information returned with logs. */

/**
* A callback prototype for generated logs. The callback with this prototype that is
* optionally globally registered is called for each log generated. Logs are
* filtered according to the log filter level set, and this routine is only called for
* logs that pass the level filter criteria. The log text must be used before
* the callback returns, as the pointer is no longer valid once the callback returns.
* @param logInfo A pointer to information about the log being generated, such as level and message. This pointer is never NULL.
* @param user_p  A pointer to opaque user data provided when the callback registered.
* @see solClient_log_setFilterLevel()
*/
  typedef void (*solClient_log_callbackFunc_t) (solClient_log_callbackInfo_pt logInfo_p, void *user_p);
                                                            /**< Callback prototype for generated logs. */

/**
* A callback prototype for timer expiry. The callback with this prototype that is
* registered when a timer is started.
* @param opaqueContext_p A pointer to Context under which the timer was started previously. This pointer is never NULL.
* @param user_p          A pointer to opaque user data that is provided when timer started.
* @see solClient_context_startTimer()
*/
  typedef void (*solClient_context_timerCallbackFunc_t) (solClient_opaqueContext_pt opaqueContext_p, void *user_p);
                                                         /**< A callback prototype for timer expiry. */

/** @name Type for a timer identifier
*/
/*@{*/
  typedef solClient_uint32_t solClient_context_timerId_t;
                                                        /**<Type for a timer identifier. */
/*@}*/

/** @name Invalid Value for a Timer Identifier
* This can be used to distinguish between
* a timer identifier value that has been allocated and one that has not.
*/
/*@{*/
#define SOLCLIENT_CONTEXT_TIMER_ID_INVALID (0xffffffff)  /**< An invalid value for a timer identifier. */
/*@}*/

/**
* @enum solClient_context_timerMode
* The type of timer to start through solClient_context_startTimer().
*/
  typedef enum solClient_context_timerMode
  {
    SOLCLIENT_CONTEXT_TIMER_ONE_SHOT = 0, /**< The timer expires once and then is automatically stopped. */
    SOLCLIENT_CONTEXT_TIMER_REPEAT = 1    /**< The timer is automatically rescheduled for the same duration upon expiry until stopped. */
  } solClient_context_timerMode_t;        /**< Type of timer to start through solClient_context_startTimer(). */
/**
 * @enum solClient_dispatchType
 * Type of dispatch function to perform for messages received on the
 * topic. see \ref ::solClient_session_rxMsgDispatchFuncInfo and \ref ::solClient_flow_rxMsgDispatchFuncInfo
 */
 typedef enum solClient_dispatchType 
 {
    SOLCLIENT_DISPATCH_TYPE_CALLBACK    = 1  /**< Callback on the dispatch function immediately when a message arrives */
 } solClient_dispatchType_t;

/**
* A function prototype for OPTIONAL application-supplied file descriptor registration service.
* @param app_p      The opaque pointer that was supplied when the file descriptor register function was configured.
*                   This pointer was passed in as user_p through solClient_context_registerFdFuncInfo_t.
* @param fd         The file descriptor for which events are being registered.
* @param events     File descriptor events of interest, OR'ed together. These events must be added to any
* existing events registered for this file descriptor.
* @param callback_p The event function to call when events occur on the file descriptor. This pointer is never NULL.
* @param user_p     An opaque pointer to be supplied to the callback routine when invoked.
* @returns          ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @see solClient_context_create()
*/
  typedef solClient_returnCode_t (*solClient_context_registerFdFunc_t) (void *app_p, solClient_fd_t fd, solClient_fdEvent_t events, solClient_context_fdCallbackFunc_t callback_p, void *user_p);
                                                                /**< A function prototype for optional application-supplied file descriptor registration service. */
/**
* A function prototype for OPTIONAL application-supplied file descriptor unregistration service.
* @param app_p      An opaque pointer supplied when the file descriptor unregister function was configured.
*                   This pointer was passed in as user_p through solClient_context_registerFdFuncInfo_t.
* @param fd         The file descriptor for which events are being unregistered.
* @param events     Events no longer of interest, OR'ed together.
* @returns          ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @see solClient_context_create()
*/
  typedef solClient_returnCode_t (*solClient_context_unregisterFdFunc_t) (void *app_p, solClient_fd_t fd, solClient_fdEvent_t events);
                                                     /**< A function prototype for optional application-supplied file descriptor unregistration service. */

/**
* @struct solClient_context_createRegisterFdFuncInfo
*
* Function information for file descriptor registration and file descriptor unregistration functions.
* This is set on a per-Context basis. Providing these functions is optional. If provided, both
* must be non-NULL, and if not provided, both must be NULL. 
*
* These functions are used when the application wants to own event generation, and they supply
* file descriptor events to the API. Such applications typically want to poll several different devices, of
* which the API is only one. When these functions are provided, the API does not manage its own 
* devices. Instead, when a device is created, the provided 'register' function is called to register the 
* device file descriptor for read and/or write events. It is the responsibility of the application to call back 
* into API when the appropriate event occurs on the device file descriptor. The API callback is 
* provided to the register function (see ::solClient_context_registerFdFunc_t). If this interface is chosen,
* the application <b>must</b> also call solClient_context_timerTick() at regular intervals.
*
* Normally these are not used, and the API owns event registrations. If an internal Context thread is used 
* by enabling ::SOLCLIENT_CONTEXT_PROP_CREATE_THREAD  (see also ::SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD),
* the API takes care of all devices and timers and no action is required by the application. If the internal thread is
* not enabled, the application must call solClient_context_processEvents() to provide scheduling time to the API.
* 
* When the API owns event registrations, it also provides file descriptor register/unregister service to the application. 
* solClient_context_registerForFdEvents() and solClient_context_unregisterForFdEvents() can be used by applications to 
* pass file descriptors to the API for managing, keeping event generation localized to the internal thread or the thread
* that calls solClient_context_processEvents().
*
*/
  typedef struct solClient_context_createRegisterFdFuncInfo
  {
    solClient_context_registerFdFunc_t regFdFunc_p;
    solClient_context_unregisterFdFunc_t unregFdFunc_p;
    void *user_p;
  } solClient_context_createRegisterFdFuncInfo_t;

/**
* @struct solClient_context_createFuncInfo
*
* Function information for Context creation. This is set on a per-Context basis.
*/
  typedef struct solClient_context_createFuncInfo
  {
    solClient_context_createRegisterFdFuncInfo_t regFdInfo;
  } solClient_context_createFuncInfo_t;

#define SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER {{NULL, NULL, NULL}}

/**
* @struct solClient_session_createRxCallbackFuncInfo
*
* <b>DEPRECATED</b>. Applications should use solClient_session_createRxMsgCallbackFuncInfo. 
* Callback information for Session message receive callback. This is set on a per-Session basis.
*/
  typedef struct solClient_session_createRxCallbackFuncInfo
  {
    void *callback_p;
    void *user_p;
  } solClient_session_createRxCallbackFuncInfo_t;

/**
* @struct solClient_session_createRxMsgCallbackFuncInfo
*
* Callback information for Session message receive callbacks. This is set on a per-Session basis.
*/
  typedef struct solClient_session_createRxMsgCallbackFuncInfo
  {
    solClient_session_rxMsgCallbackFunc_t callback_p;
    void *user_p;
  } solClient_session_createRxMsgCallbackFuncInfo_t;

/**
* @struct solClient_session_createEventCallbackFuncInfo
*
* Callback information for Session event callback. This is set on a per-Session basis.
*/
  typedef struct solClient_session_createEventCallbackFuncInfo
  {
    solClient_session_eventCallbackFunc_t callback_p;
    void *user_p;
  } solClient_session_createEventCallbackFuncInfo_t;

/**
 * @struct solClient_session_rxMsgDispatchFuncInfo
 *
 * Callback information for Session message receive dispatch. This can be set on a per-subscription basis.
 * This structure is used with ::solClient_session_topicSubscribeWithDispatch and ::solClient_session_topicUnsubscribeWithDispatch.
 * 
 */
  typedef struct solClient_session_rxMsgDispatchFuncInfo
  {
    solClient_dispatchType_t           dispatchType;    /**< The type of dispatch described. */
    solClient_session_rxMsgCallbackFunc_t callback_p;   /**< An application-defined callback function; may be NULL if there is no callback. */
    void                                 *user_p;       /**< A user pointer to return with the callback; must be NULL if callback_p is NULL. */
    void                                 *rfu_p;        /**< Reserved for Future use; must be NULL. */
  } solClient_session_rxMsgDispatchFuncInfo_t;

/**
 * @struct solClient_flow_createRxCallbackFuncInfo
 *
 * Callback information for Flow message receive callback. This is set on a per-Flow basis.
 */
  typedef struct solClient_flow_createRxCallbackFuncInfo
  {
    void *callback_p;
    void *user_p;
  } solClient_flow_createRxCallbackFuncInfo_t;

/**
* @struct solClient_flow_createRxMsgCallbackFuncInfo
*
* Callback information for Flow message receive callback. This is set on a per-Flow basis.
*/
  typedef struct solClient_flow_createRxMsgCallbackFuncInfo
  {
    solClient_flow_rxMsgCallbackFunc_t callback_p;
    void *user_p;
  } solClient_flow_createRxMsgCallbackFuncInfo_t;

/**
 * @struct solClient_flow_rxMsgDispatchFuncInfo
 *
 * Callback information for Flow message receive dispatch. This can be set on a per-subscription basis.
 * This structure is used with ::solClient_flow_topicSubscribeWithDispatch and ::solClient_flow_topicUnsubscribeWithDispatch.
 */
  typedef struct solClient_flow_rxMsgDispatchFuncInfo
  {
    solClient_dispatchType_t           dispatchType; /**< The type of dispatch described */
    solClient_flow_rxMsgCallbackFunc_t callback_p;   /**< An application-defined callback function; may be NULL if there is no callback */
    void                              *user_p;       /**< A user pointer to return with the callback; must be NULL if callback_p is NULL */
    void                              *rfu_p;        /**< Reserved for future use; must be NULL. */
  } solClient_flow_rxMsgDispatchFuncInfo_t;

/**
* @struct solClient_flow_createEventCallbackFuncInfo
*
* Callback information for Flow event callback. This is set on a per-Flow basis.
*/
  typedef struct solClient_flow_createEventCallbackFuncInfo
  {
    solClient_flow_eventCallbackFunc_t callback_p;
    void *user_p;
  } solClient_flow_createEventCallbackFuncInfo_t;


/**
* @struct solClient_session_createFuncInfo
*
* Function information for Session creation. This is set on a per-Session basis.  
*
* The application must set the eventInfo callback information. All Sessions must have an event callback registered.
*
* The application must set one, and only one, message callback information. The <i>rxInfo</i> message callback interface is 
* <b>deprecated</b> and should be set to NULL. All applications should prefer to use the <i>rxMsgInfo</i> callback interface.
* The application has available to it a ::solClient_opaqueMsg_pt, which can be kept for later processing and provides a
* structured interface for accessing elements of the received message. The application callback routine then has the signature
* (see ::solClient_session_rxMsgCallbackFunc_t) :
*
* <code style="font-size:90%">
  solClient_rxMsgCallback_returnCode_t
  applicationRxMsgCallback (solClient_opaqueSession_pt  opaqueSession_p, 
                            solClient_opaqueMsg_pt      msg_p, 
                            void                       *user_p);
* </code>
*
*/
  typedef struct solClient_session_createFuncInfo
  {
    solClient_session_createRxCallbackFuncInfo_t    rxInfo;
    solClient_session_createEventCallbackFuncInfo_t eventInfo;
    solClient_session_createRxMsgCallbackFuncInfo_t rxMsgInfo;
  } solClient_session_createFuncInfo_t;

/**
* @struct solClient_flow_createFuncInfo
*
* Function information for Flow creation. This is set on a per-Flow basis.
* The application must set the eventInfo callback information. All Flows must have an event callback registered.
*
* The application must set one, and only one, message callback information. The other message callback interface must be
* set to NULL. If the application uses the <i>rxInfo</i> callback interface, then the application callback will have the
* interface (see ::solClient_flow_rxCallbackFunc_t):
*
* <code style="font-size:90%">
  void 
  applicationRxDataCallback (solClient_opaqueFlow_pt            opaqueFlow_p, 
                             solClient_bufInfo_pt               bufInfo_p,
                             solClient_flow_rxCallbackInfo_pt   rxInfo_p,
                             void                              *user_p);
* </code>
*
* Typically, applications will prefer to use the <i>rxMsgInfo</i> callback interface. The application has available to it a
* ::solClient_opaqueMsg_pt, which can be kept for later processing and provides a structured interface for accessing elements of the 
* received message. The application callback routine then has the signature (see ::solClient_flow_rxMsgCallbackFunc_t):
*
* <code style="font-size:90%">
  solClient_rxMsgCallback_returnCode_t
  applicationRxMsgCallback (solClient_opaqueFlow_pt     opaqueFlow_p,
                            solClient_opaqueMsg_pt      msg_p,
                            void                       *user_p);
* </code>
*
*/
  typedef struct solClient_flow_createFuncInfo
  {
    solClient_flow_createRxCallbackFuncInfo_t rxInfo;
    solClient_flow_createEventCallbackFuncInfo_t eventInfo;
    solClient_flow_createRxMsgCallbackFuncInfo_t rxMsgInfo;
  } solClient_flow_createFuncInfo_t;

#define SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER {{NULL,NULL},{NULL,NULL},{NULL,NULL}}
#define SOLCLIENT_FLOW_CREATEFUNC_INITIALIZER {{NULL,NULL},{NULL,NULL},{NULL,NULL}}
#define SOLCLIENT_SESSION_DISPATCHFUNC_INITIALIZER(type) {type, NULL, NULL, NULL}
#define SOLCLIENT_FLOW_DISPATCHFUNC_INITIALIZER(type) {type, NULL, NULL, NULL}

/**
* @struct solClient_version_info
*
* A structure returned from solClient_version_get that contains version information.
* All the returned pointers are never NULL when returned from solClient_version_get.
*
* @see solClient_version_get()
*/
  typedef struct solClient_version_info
  {
    const char *version_p;   /**< A pointer to string with version, for example, "1.1.1". */
    const char *dateTime_p;  /**< Date and time. */
    const char *variant_p;   /**< A pointer to string with variant information (for example, type of operating system and compiler). */
  } solClient_version_info_t, *solClient_version_info_pt; /**< A pointer to ::solClient_version_info structure holding version information. */

/**
* This function must be called before any other API interface call is made, with the exception of
* solClient_log_setCallback(), which can be called first to intercept all logs, and
* solClient_log_setFile(), which can be used to change the log destination when the
* log callback is not in use.  Typically solClient_initialize() is called once during
* program initialization.
* Only configuration property names starting with \ref globalProps "GLOBAL_" are processed; other property names
* are ignored. Any values not supplied are set to default values.
* This function takes care of any required global initialization.
* Note that the property values are stored internally in the API, and the caller does not have to maintain
* the props array or the strings that are pointed to after this call completes. Also, the API does not modify any of
* the strings pointed to by props when processing the property list.
* @param initialLogLevel The initial log level for logs for all log categories. This value
*                        can later be changed using solClient_log_setFilterLevel. If the
*                        default log level is desired, use ::SOLCLIENT_LOG_DEFAULT_FILTER.
*                        However, it is preferred that the application supply the log level
*                        from its own command line (or some other form of configuration) so
*                        that the log level can quickly be changed without recompiling
*                        the application.
* @param props An array of name and value string pairs for \ref globalProps "global configuration properties", or NULL, if there are no global properties to set.
* @returns ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_initialize (solClient_log_level_t initialLogLevel,
                          solClient_propertyArray_pt props);

/**
* This function may be called after use of the API is completed.  
* This function takes care of any global clean-up that might be required.
* It automatically frees all allocated resources, including those for Contexts and
* Sessions that have been previously created but are not destroyed.
* Note that if solClient_cleanup() is called while Sessions are connected, any messages
* buffered for transmission are discarded and not sent. See solClient_session_disconnect()
* for further information.
* In most applications, it is not necessary to call solClient_cleanup() as the resources in 
* use are automatically recovered when the application exits. solClient_cleanup() exists for 
* rare applications that may want recover all API resources and continue operating without using
* the API further.
* @returns ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t solClient_cleanup (void);

/**
 * Returns a string representation of the return code passed in.
 * @param returnCode The return code to convert to a string representation.
 * @returns A pointer to a constant character string. This pointer is never NULL.
 */
  solClient_dllExport const char
    *solClient_returnCodeToString (solClient_returnCode_t returnCode);                                            

/**
 * Returns a string representation of the subcode passed in.
 * @param subCode The subcode to convert to a string representation.
 * @returns A pointer to a constant character string. This pointer is never NULL.
 */
  solClient_dllExport const char
    *solClient_subCodeToString (solClient_subCode_t subCode);                                            

/**
* Returns a string representation of the transmit statistic name for the given 
* statistic constant passed in.
*
* @param        txStat transmit statistic constant (::solClient_stats_tx_t).
* @returns       A pointer to a constant char string. This pointer is never NULL.
*/
  solClient_dllExport const char 
    *solClient_txStatToString (solClient_stats_tx_t txStat);

/**
* Returns a string representation of the receive statistic name for the given 
* statistic constant passed in.
*
* @param        rxStat receive statistic constant (::solClient_stats_rx_t).
* @returns       A pointer to a constant char string. This pointer is never NULL.
*/
  solClient_dllExport const char 
    *solClient_rxStatToString (solClient_stats_rx_t rxStat);

/**
* Returns a pointer to a ::solClient_errorInfo structure, which contains the last captured
* error information for the calling thread. This information is captured on a per-thread
* basis. The returned structure is only valid until the thread makes the next API call,
* so if the calling thread wants to keep any of the structure fields, it must make a
* local copy of the information of interest.
*
* Any API call that returns ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_READY also updates the per-thread
* error information.  Applications that wish extra information on the error, should retrieve the 
* ::solClient_errorInfo structure. 
*
* The API always sets the ::solClient_errorInfo information before invoking any application callback. 
* Therefore application may always call solClient_getLastErrorInfo() while handling event callbacks.
*
* @returns A pointer to a ::solClient_errorInfo structure containing error information. This pointer is never NULL.
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_errorInfo_pt
    solClient_getLastErrorInfo (void);

/**
* Clears the last error info, which is recorded on a per-thread basis. The error information
* is reset such that the sub code is ::SOLCLIENT_SUBCODE_OK, the error strings are empty strings,
* etc.
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport void solClient_resetLastErrorInfo (void);

/**
* Returns version information for the API. The caller is given a pointer to an
* internal version structure of type solClient_version_info_t containing the
* version information.
* @param version_p A pointer to a structure to hold the version information.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_version_get (solClient_version_info_pt * version_p);

/**
* Returns a Universally Unique Identifier (UUID) as a 128-bit value, as per IETF RFC 4122.
* Only a version 4 UUID (generated from random or pseudo-random numbers) is supported. 
* @param uuid_p A pointer to UUID value to be filled in.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_generateUUID(solClient_uuid_t *uuid_p);

/**
* Returns a Universally Unique Identifier (UUID) as a NULL-terminated string, as per IETF RFC 4122.
* Only a version 4 UUID (generated from random or pseudo-random numbers) is supported.
* The size of the buffer provided must be at least ::SOLCLIENT_UUID_STRING_BUFFER_SIZE bytes in
* size to hold the NULL-terminated UUID string value.
* An example output string is: 4e112068-7836-4e2f-af14-82682569bdfe (NULL-terminated).
* @param uuid_p A pointer to string buffer to fill in.
* @param bufferSize The size (in bytes) of provided buffer; must be at least ::SOLCLIENT_UUID_STRING_BUFFER_SIZE.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_generateUUIDString(char *uuid_p, size_t bufferSize);

/**
* Appends (after a '/' delimeter') a Universally Unique Identifier (UUID) as a NULL-terminated string to a
* given input string.
* Only a version 4 UUID (generated from random or pseudo-random numbers) is supported.
* The size of the buffer provided must be at least ::SOLCLIENT_UUID_STRING_BUFFER_SIZE bytes in
* size to hold the NULL-terminated UUID string value.
* An example output string is: 4e112068-7836-4e2f-af14-82682569bdfe (NULL-terminated).
* @param inputStr A pointer to a null terminated string.
* @param uuid_p A pointer to string buffer to fill in.
* @param bufferSize The size (in bytes) of provided buffer; must be at least (strlen(inputStr) + ::SOLCLIENT_UUID_STRING_BUFFER_SIZE).
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_appendUUIDString(const char *inputStr, char *uuid_p, size_t bufferSize);

/**
* Allows the log level filter to be set. Any logs of lower severity
* than the filter level specified are not emitted by the API. For example, if the
* filter level is set to solClient_LOG_ERROR, then only logs of this severity or
* higher (for example, solClient_LOG_CRITICAL) are emitted. Less severe logs are filtered
* out. The log filter level is applied globally to ALL API Sessions.
* If this function is not called, then the default log filter level of
* ::SOLCLIENT_LOG_DEFAULT_FILTER is in force.
* @param category The log category to which the new log level applies.
* @param level The new log level at which logs are emitted.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_log_setFilterLevel (solClient_log_category_t category,
                                  solClient_log_level_t level);

/**
* Allows for the registration of an optional log callback, which is called
* each time a new, non-filtered, log message is emitted by the API. If a 
* callback is not registered, then, by default, the API prints non-filtered 
* logs to stderr, although the destination file can be changed through 
* solClient_log_setFile(). Setting the callback routine to NULL reverts to 
* the default log output behavior.
* Note that solClient_log_setCallback() can be called before 
* solClient_initialize() in order to intercept all logs.
* @param callback_p The routine to call for each non-filtered log.
* @param user_p     A pointer to opaque user data that is passed back to the log callback.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @see solClient_log_setFilterLevel()
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_log_setCallback (solClient_log_callbackFunc_t callback_p,
                               void *user_p);

/**
* Reset the log callback function to default log out behavior. All 
* non-filtered logs are printed to stderr after this function is called.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_log_unsetCallback (void);
    
/**
* Allows the log file, which defaults to stderr, to be changed to a file specified
* by the caller. This file is only used if a log callback has not been set through
* solClient_log_setCallback(). Setting the file name to NULL or a zero-length string
* reverts the file back to stderr. If an error is encountered when
* writing a log message to the specified file, the log file is automatically reverted
* back to stderr.
* Note that solClient_log_setFile() can be called before solClient_initialize().
* @param logFileName_p The new file name to use, or use the default (stderr) if NULL or zero length.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_log_setFile (const char *logFileName_p);

/**
* Returns a string representation (for example, "API") of the log category passed in.
* @param category The log category to convert to a string representation.
* @return A pointer to a constant character string. This pointer is never NULL.
*/
  solClient_dllExport const char
    *solClient_log_categoryToString (solClient_log_category_t category);

/**
* Returns a string representation (for example, "ERROR") of the log level passed in.
* @param level The log level to convert to a string representation.
* @return A pointer to a constant character string. This pointer is never NULL.
*/
  solClient_dllExport const char
    *solClient_log_levelToString (solClient_log_level_t level);

#ifdef PROVIDE_LOG_UTILITIES
/** @name Utilities to raise logs
*/
/*@{*/
/**
* Allows the application to raise a log using the format of logs
* generated internally by the API. An application can use this if it does not
* have its own logging facility. A log is only raised if the specified log level
* exceeds the log filter level in force for the application log category. 
* Note that if an application has registered a log callback using 
* solClient_SetLogCallback, then when solClient_Log is called, 
* the log callback is invoked.
*
* The format of this call is similar to printf, with a format string 
* followed by a variable list of parameters.
*
* solClient_log is only available if the applications are compiled with 
* PROVIDE_LOG_UTILITIES defined (-DPROVIDE_LOG_UTILITIES).
*
* example: solClient_log(SOLCLIENT_LOG_NOTICE, "Value is %d", value);
*
* @see solClient_log_setFilterLevel()
* @see solClient_log_setCallback()
* @param level    The log level for the log being raised.
*/
#define solClient_log(level, ...) \
    {if (level <= _solClient_log_appFilterLevel_g) { \
         _solClient_log_output_detail(SOLCLIENT_LOG_CATEGORY_APP, level, \
         "/" __FILE__, __LINE__, __VA_ARGS__);}}

/**
* Allows the application to raise a log that is similar to solClient_log. 
* This routine accepts a va_list to hold the variable list of arguments that 
* comes after format_p. This form is useful if a wrapper routine in 
* the application has already received a
* variable list of arguments and has converted them to a va_list.
*
* solClient_log_va_list is only available if the applications are compiled with 
* PROVIDE_LOG_UTILITIES defined (-DPROVIDE_LOG_UTILITIES).
*
* @see solClient_log_setFilterLevel()
* @see solClient_log_setCallback()
* @see solClient_log()
* @param level    The log level for the log being raised.
* @param format_p A pointer to a format string, in the printf style.
* @param ap A va_list holding the remaining variable parameters.
*/
#define solClient_log_va_list(level, format_p, ap) \
    {if (level <= _solClient_log_appFilterLevel_g) { \
         _solClient_log_output_detail_va_list(SOLCLIENT_LOG_CATEGORY_APP, level, \
         "/" __FILE__, __LINE__, format_p, ap);}}
/*@}*/
#endif

/**
* Creates a new Context. The @ref ContextProps "context properties" are supplied as an array of
* name/value pointer pairs, where the name and value are both strings. Only configuration 
* property names starting with "CONTEXT_" are processed; other property names
* are ignored. Any values not supplied are set to default values.
* Passing in NULL causes all Context
* properties to be set to their default values.
*
* It is highly recommended that applications should always use an automatically created internal context 
* thread. Passing in ::SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD
* causes the Context to have the context thread created automatically and all other Context properties at
* their default value. Alternatively, an internal thread can be created by enabling ::SOLCLIENT_CONTEXT_PROP_CREATE_THREAD.
*
* If an external context thread  (the default for historical reasons) is needed under special circumstances, 
* the application must supply a thread to do the work on
* behalf of a Context. The application does this by having a Context thread call solClient_context_processEvents(). In some rare cases, application can also use an optional file descriptor event service.
*  Applications can register
* application file descriptors for events using solClient_context_registerForFdEvents(). The file descriptor event callback routine that has
* been registered is invoked by the Context thread when the event occurs.
*
* As an alternative, the application can provide the file descriptor event registration mechanism itself, which then
* results in the API using the application-provided events. To activate this mode, the application must provide file descriptor event
* register and deregister function at the time of Context creation (see ::solClient_context_createRegisterFdFuncInfo). If the register/deregister function
* pointers provided are NULL, then the internal API event mechanism is used. When the application is providing events to the API,
* solClient_context_processEvents() must not be called. Instead solClient_context_timerTick() must be called on regular basis as
* API timers are dependent on this function which is usually invoked during solClient_context_processEvents().
* 
*
* When the Context is created, an opaque Context pointer is returned to the caller, and this value 
* is then used for any Context-level operations, such as creating a Session.
*
* Note that the property values are stored internally in the API, and the caller does not have to maintain
* the props array or the strings that are pointed to after this call completes. The API also does not modify any of
* the strings pointed to by props when processing the property list.
*
* @param props           An array of name/value string pair pointers to configure @ref ContextProps "context properties".
* @param opaqueContext_p An opaque Context pointer is returned that refers to the created Context.
* @param funcInfo_p      A pointer to a structure that provides information on optional file descriptor event functions.
* @param funcInfoSize    The size of the passed-in funcInfo structure (in bytes) to allow the structure to grow in the future.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_context_create (solClient_propertyArray_pt props,
                              solClient_opaqueContext_pt * opaqueContext_p,
                              solClient_context_createFuncInfo_t * funcInfo_p,
                              size_t funcInfoSize);

/**
* Destroys a previously created Context. On return, the opaque Context pointer
* is set to NULL. This operation <b>must not</b> be performed in a Context callback
* for the Context being destroyed. This includes all Sessions on the Context, timers on
* the Context, and application-supplied register file descriptor (see 
* ::solClient_context_createFuncInfo) functions.
*
* @param opaqueContext_p An opaque Context returned when Context was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_context_destroy (solClient_opaqueContext_pt * opaqueContext_p);

/**
* Allows the application to optionally register an application file descriptor for events (for example,
* ::SOLCLIENT_FD_EVENT_READ) for an application file descriptor. The events parameter is an OR of
* the events requested (for example, if read and write events are requested, events is
* ::SOLCLIENT_FD_EVENT_READ | ::SOLCLIENT_FD_EVENT_WRITE).
* When the specified event(s) occur, the specified callback routine is invoked (in the context of the
* Context thread that called solClient_ProcessEvents), and the opaque user value is provided.
* When an event occurs, the event stays registered for the file descriptor unless the deregister function for the
* event is called, or if register is called again and the event is not specified.
* \n
* NOTE: When this function is called for a given file descriptor, the specified events replace any previously
*       specified events. Therefore all events for a given file descriptor share the same callback routine and user data.
* \n
*
* @param opaqueContext_p The opaque Context that was returned when Context was created.
* @param fd              The file descriptor being registered.
* @param events          The events being requested (multiple events can be OR'ed together).
* @param callback_p      The callback routine to be invoked.
* @param user_p          The opaque user data pointer (can be NULL) that is returned in the callback.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @li ::SOLCLIENT_SUBCODE_OUT_OF_RESOURCES - The number of file descriptors exceeds the operating system limit (refer to \ref feature-limitations "Feature Limitations").
* @see ::solClient_subCode for a description of all subcodes.
 */
    solClient_dllExport solClient_returnCode_t
    solClient_context_registerForFdEvents (solClient_opaqueContext_pt opaqueContext_p,
                                           solClient_fd_t fd,
                                           solClient_fdEvent_t events,
                                           solClient_context_fdCallbackFunc_t
                                           callback_p, void *user_p);

/**
* Allows the application to optionally deregister an application file descriptor for events (for example,
* ::SOLCLIENT_FD_EVENT_READ) for an application file descriptor. The events is an OR of the events no longer
* needed. If a file descriptor is called that has not been registered, or if an event is specified for a file descriptor that
* was not registered, ::SOLCLIENT_OK is returned. For example, in clean-up code, this function can be blindly
* called with ::SOLCLIENT_FD_EVENT_ALL for a given file descriptor with no ill effects.
* @param opaqueContext_p The opaque Context that was returned when Context was created.
* @param fd              The file descriptor that events are being deregistered for.
* @param events          The events no longer being requested (multiple events can be ORed together).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_context_unregisterForFdEvents (solClient_opaqueContext_pt
                                             opaqueContext_p,
                                             solClient_fd_t fd,
                                             solClient_fdEvent_t events);

/**
* Must be called by the Context thread for a given Context to have events processed. If messages
* arrive on a Session, the Session callback function is invoked in the Context of the calling Context
* thread. If application file descriptors have been registered and requested events have occurred on those file descriptors, the
* registered file descriptor event callback function is invoked in the Context of the calling Context thread. The
* Session event callback routine can also be invoked. This routine must be called as often as possible
* by the Context thread. Also, the application must avoid long processing times in the called
* messsage receive or application file descriptor event routines that can be invoked by this function.
*
* Note that if the application has taken over responsibility of generating file descriptor
* events, then this routine must not be called by the application. Instead solClient_context_timerTick() 
* must be called on regular basis as API timers are dependent on this function which is usually invoked 
* during solClient_context_processEvents().
*
* @param opaqueContext_p The opaque Context that was returned when Context was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_context_processEvents (solClient_opaqueContext_pt
                                     opaqueContext_p);

/**
* This function must be called by the Context thread for a given Context to have events 
* processed. This function also takes a second parameter 'wait'. 
* If non-zero, solClient_context_processEventsWait will wait for an event (timeout or received message) and
* is identical to simply calling solClient_context_processEvents().
*
* When wait is zero, solClient_context_processEventsWait() always returns 
* immediately, and if no received events occurred, this function returns 
* ::SOLCLIENT_NOEVENT.
*
* See also solClient_context_processEvents() for a more information.
*
* @param opaqueContext_p Opaque Context that was returned when Context was created.
* @param wait When 1, processEvents waits for an event; when 0, it returns 
*              immediately with ::SOLCLIENT_NOEVENT if there is nothing pending.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOEVENT
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_context_processEventsWait 
                     (solClient_opaqueContext_pt opaqueContext_p,
                      solClient_bool_t wait);

/**
* Allows the application to start a timer. When the timer expires, the
* specified timer callback routine is invoked with the user pointer that was specified
* at timer start time. The timer duration is subject to the timer resolution specified
* by the Context property ::SOLCLIENT_CONTEXT_PROP_TIME_RES_MS. The timer duration is rounded
* up to the nearest timer resolution interval. For example, if the Context timer resolution
* is 50 ms, and a timer is started with a duration of 60 ms, then the timer duration is
* rounded up to 100 ms (2 ticks), and then one further tick is added for a total timer
* duration of 150 ms. This extra tick accounts for the fact that the current tick might
* be about to advance when the timer is started. Therefore, the actual timeout in this
* example is in the range of 100 ms to 150 ms.
* The timer callback routine is invoked in the cntext of the
* Context thread, and timeouts occur as a result of calling solClient_context_processEvents().
* The timer durations are approximate; the actual duration can be affected by the
* processing time spent in other callbacks by the Context thread, such as for Session events,
* received messages, and file descriptor events. This timer service should only be used for applications
* that require a coarse timer service (for example, a guard timer, or other timer uses that do
* not require a highly accurate timer service).
*
* When a one-shot timer expires, the timer is automatically cancelled. When a repeat
* timer expires, the timer is automatically rescheduled for the same duration, and
* continues to run until stopped using solClient_context_stopTimer.
*
* The invoked timer callback routine is allowed to start and stop timers.
* \n
*
* @param opaqueContext_p The opaque Context that was returned when Context was created.
* @param timerMode       The type of timer to start (one-shot or repeat).
* @param durationMs      The requested timer duration (in milliseconds).
* @param callback_p      The routine to invoke when the timer expires.
* @param user_p          A pointer to opaque user data that is provided to the callback routine.
* @param timerId_p       A pointer to a returned timer ID which is used to stop a timer.
* @see solClient_context_stopTimer()
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @li ::SOLCLIENT_SUBCODE_OUT_OF_RESOURCES - The maximum number of timers started for context\n
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_context_startTimer (solClient_opaqueContext_pt opaqueContext_p,
                                  solClient_context_timerMode_t timerMode,
                                  solClient_uint32_t durationMs,
                                  solClient_context_timerCallbackFunc_t
                                  callback_p, void *user_p,
                                  solClient_context_timerId_t * timerId_p);

/**
* Allows the application to stop a previously started timer. Timers that expire
* and have their callback routine invoked are not required to be stopped by the application.
* @param opaqueContext_p The opaque Context that was returned when Context was created.
* @param timerId_p       A pointer to the identifier of the timer to be stopped; previously
*                        returned from solClient_context_startTimer(). The timer id is set to
*                        ::SOLCLIENT_CONTEXT_TIMER_ID_INVALID when returned.
* @see solClient_context_startTimer()
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_context_stopTimer (solClient_opaqueContext_pt opaqueContext_p,
                                 solClient_context_timerId_t * timerId_p);

/**
* When the application takes over all file descriptor event generation, the application
* must also call this routine at regular intervals (specified by the Context property
* ::SOLCLIENT_CONTEXT_PROP_DEFAULT_TIME_RES_MS) to provide a timer tick for timer
* processing. When the application calls solClient_context_processEvents() for the Context,
* which is the normal mode of operation, then the application must not call this routine because
* solClient_context_processEvents() takes care of generating timer ticks.
* When this routine is invoked, it might result in the invocation of timer expiry callbacks.
*
* If the time interval between calls to solClient_context_timerTick() exceeds 60000 ms (60 seconds)
* the API assumes the system clock has changed, resets its internal concept of time, and ignores
* the call to solClient_context_timerTick().  Consequently, if the application attempts to use a
* timer resolution greater than 60 seconds, timers will never expire and any resolution greater
* than 30 seconds can cause timer ticks to be missed. 
*
* @param opaqueContext_p The opaque Context that was returned when the Context was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_context_timerTick (solClient_opaqueContext_pt opaqueContext_p);

/**
* Creates a new Session within a specified Context. The @ref SessionProps "session properties"
* are supplied as an array of name/value pointer pairs, where the name and value are both strings.
* Only configuration property names starting with "SESSION_" are processed; other property names
* are ignored. Any values not supplied are set to default values.
* When the Session is created, an opaque Session pointer is returned to the caller, and this value 
* is then used for any Session-level operations (for example, sending a message).
* The passed-in structure functInfo_p provides information on the message receive callback
* function and the Session event function which the application has provided for this Session.
* Both of these callbacks are mandatory. The message receive callback is invoked for each
* received message on this Session. The Session event callback is invoked when Session events
* occur, such as the Session going up or down. Both callbacks are invoked in the context
* of the Context thread to which this Session belongs.
* Note that the property values are stored internally in the API and the caller does not have to maintain
* the props array or the strings that are pointed to after this call completes. When processing the property list, the API 
* will not modify any of the strings pointed to by props.
* \n 
*
* @param props           An array of name/value string pair pointers to configure @ref SessionProps "session properties".
* @param opaqueContext_p The Context in which the Session is to be created.
* @param opaqueSession_p An opaque Session pointer is returned that refers to the created Session.
* @param funcInfo_p      A pointer to a structure that provides information on callback functions for events and received messages.
* @param funcInfoSize    The size (in bytes) of the passed-in funcInfo structure to allow the structure to grow in the future.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @li ::SOLCLIENT_SUBCODE_OUT_OF_RESOURCES - The maximum number of Sessions already created for Context (refer to ::SOLCLIENT_CONTEXT_PROP_MAX_SESSIONS).
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_create (solClient_propertyArray_pt props,
                              solClient_opaqueContext_pt opaqueContext_p,
                              solClient_opaqueSession_pt * opaqueSession_p,
                              solClient_session_createFuncInfo_t * funcInfo_p,
                              size_t funcInfoSize);

/**
* Destroys a previously created Session. Upon return, the opaque Session pointer
* is set to NULL. If the Session being destroyed is still in a connected state,
* any buffered messages which have not been sent yet are
* discarded. If the application wants to ensure that any buffered messages are 
* first sent, solClient_session_disconnect() must be 
* called before solClient_sesssion_destroy().
*
* This operation <b>must not</b> be performed in a Session callback
* for the Session being destroyed. This includes all Flows on the Session,
* as well as the application supplied event and data callback functions (see 
* ::solClient_session_createFuncInfo) functions.
* @param opaqueSession_p An opaque Session that was returned when the Session was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_destroy (solClient_opaqueSession_pt * opaqueSession_p);

/**
 * Returns the Context associated with the specified Session. Sessions are created
 * within a Context (see ::solClient_session_create) and this routine can be used
 * if the application has a Session pointer and needs to determine the associated
 * Context. For example, in a Session event callback in which a Session pointer is
 * provided, if the application wishes to start a timer, it needs the Context
 * pointer, and this routine can be used to get the Context pointer from the 
 * Session pointer.
 * @param opaqueSession_p An opaque Session that was returned when the Session was created.
 * @param opaqueContext_p An opaque Context associated with the Session that is returned.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
 solClient_dllExport solClient_returnCode_t 
    solClient_session_getContext(solClient_opaqueSession_pt  opaqueSession_p,
                                 solClient_opaqueContext_pt *opaqueContext_p);

/**
* Allows certain properties of a Session to be modified after the Session has been created.
* Currently, only the following Session properties can be modified; attempting to specify other Session properties will result in 
* ::SOLCLIENT_FAIL being returned:
*        @li ::SOLCLIENT_SESSION_PROP_APPLICATION_DESCRIPTION (Deprecated -- see Note below)
*        @li ::SOLCLIENT_SESSION_PROP_CLIENT_NAME (Deprecated -- see Note below)
*        @li ::SOLCLIENT_SESSION_PROP_HOST (may only be modified when Session is disconnected)
*        @li ::SOLCLIENT_SESSION_PROP_PORT (may only be modified when Session is disconnected)
*
* Note: Applications shall use ::solClient_session_modifyClientInfo() to modify the following Session properties:
*        @li ::SOLCLIENT_SESSION_PROP_APPLICATION_DESCRIPTION
*        @li ::SOLCLIENT_SESSION_PROP_CLIENT_NAME
*
* Note that the property values are stored internally in the API, and the caller does not have to maintain
* the props array or the strings that are pointed to after this call completes. The API also will not modify any of
* the strings pointed to by props when processing the property list.
* \n 
*
* @param opaqueSession_p The opaque Session that was returned when Session was created.
* @param props           An array of name/value string pair pointers to modify Session properties.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
*  @li ::SOLCLIENT_SUBCODE_CANNOT_MODIFY_WHILE_NOT_IDLE
*  @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_modifyProperties (solClient_opaqueSession_pt
                                        opaqueSession_p, solClient_propertyArray_pt props);

/**
* Allows the following client name and description properties of a Session to be modified after the Session has been created.
*        @li ::SOLCLIENT_SESSION_PROP_APPLICATION_DESCRIPTION
*        @li ::SOLCLIENT_SESSION_PROP_CLIENT_NAME
*
* The property modifications can be  carried out in a blocking or non-blocking mode, depending on the flag.
* Attempting to specify other Session properties will result in ::SOLCLIENT_FAIL being returned. 
*
* Note that only one outstanding client info modification request is allowed.
*
* Note that changing client name property would trigger the P2P topic changes. To avoid message loss, it is recommended that this API 
* is used only at the initialization time. 
*
* Note that the property values are stored internally in the API, and the caller does not have to maintain
* the props array or the strings that are pointed to after this call completes. The API also will not modify any of
* the strings pointed to by props when processing the property list.  
* \n 
*
* @param opaqueSession_p The opaque Session that was returned when Session was created.
* @param props   An array of name/value string pair pointers to modify Session properties.
* @param flag    A flag to control the operation. Valid flag for this operation is:
*  @li ::SOLCLIENT_MODIFYPROP_FLAGS_WAITFORCONFIRM
* 
* @param correlation_p  A valid correlationTag pointer for non-blocking mode that will be returned in an asynchronous confirmation (::SOLCLIENT_SESSION_EVENT_MODIFYPROP_OK) or failure (::SOLCLIENT_SESSION_EVENT_MODIFYPROP_FAIL).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS ::SOLCLIENT_NOT_READY
* @subcodes
 * @li ::SOLCLIENT_SUBCODE_CLIENT_NAME_ALREADY_IN_USE - The client name is in use by another client in the same VPN. Applications need to call ::solClient_session_connect after client name correction.
 * @li ::SOLCLIENT_SUBCODE_CLIENT_NAME_INVALID - The client name chosen has been rejected as invalid by the appliance.  Applications need to call ::solClient_session_connect after client name correction.
 * @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_modifyClientInfo(solClient_opaqueSession_pt  opaqueSession_p, 
                                       solClient_propertyArray_pt  props,
                                       solClient_modifyPropFlags_t flag,
                                       void                        *correlation_p);

/**
* Gets the value of the specified Session property for the Session. The 
* property value is copied out to buffer provided by the caller. The returned
* value is a NULL-terminated string.
*
* @param opaqueSession_p An opaque Session returned when the Session was created.
* @param propertyName_p  The name of the Session property for which the value is to be returned.
* @param buf_p           A pointer to the buffer provided by the caller in which to place the NULL-terminated property value string.
* @param bufSize         The size (in bytes) of the buffer provided by the caller.
* @returns               ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
solClient_dllExport solClient_returnCode_t 
solClient_session_getProperty( 
    solClient_opaqueSession_pt  opaqueSession_p,
    const char                 *propertyName_p,
    char                       *buf_p,
    size_t                      bufSize
);

/**
* Gets the value of the specified capability for the Session. The caller
* provides a solClient_field_t structure that is set on success with the proper type and value
* for the property requested.
*
* @param opaqueSession_p  The opaque Session returned when the Session was created.
* @param capabilityName_p The name of the \ref sessioncapabilities "Session capability" the value is to be returned for.
* @param field_p          A pointer to the solClient_field_t provided by the caller in which to place the capability value.
* @param fieldSize        The size (in bytes) of the solClient_field_t provided by the caller.
* @returns               ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/

solClient_dllExport solClient_returnCode_t 
solClient_session_getCapability(
    solClient_opaqueSession_pt  opaqueSession_p,
    const char                 *capabilityName_p,
    solClient_field_t          *field_p,
    size_t                      fieldSize
);

/**
* Checks if the specified capability is set on the currently connected Session. Returns true if the Session has the capability requested.
*
* @param opaqueSession_p  The opaque Session returned when the Session was created.
* @param capabilityName_p The name of the \ref sessioncapabilities "session capability" the value is to be returned for.
* @returns                True or False.
*/

solClient_dllExport solClient_bool_t 
solClient_session_isCapable(
    solClient_opaqueSession_pt  opaqueSession_p,
    const char                 *capabilityName_p
);

/**
* Connects the specified Session. A Session connection can be carried out in a blocking or
* non-blocking mode, depending upon the Session property
* ::SOLCLIENT_SESSION_PROP_CONNECT_BLOCKING.
* In blocking mode, the calling thread is blocked until either the Session connection attempt
* succeeds or is determined to have failed. If the connection succeeds, ::SOLCLIENT_OK is
* returned. If the Session could not connect, ::SOLCLIENT_NOT_READY is returned.
* In non-blocking mode, ::SOLCLIENT_IN_PROGRESS is returned upon a successful Session connect
* request, and the connection attempt proceeds in the background.
* In both non-blocking and blocking mode, a Session event is generated for the Session:
* ::SOLCLIENT_SESSION_EVENT_UP_NOTICE, if the Session was connected successfully; or
* ::SOLCLIENT_SESSION_EVENT_CONNECT_FAILED_ERROR, if the Session failed to connect.
* For blocking mode, the Session event is issued before the call to
* solClient_session_connect() returns. For non-blocking mode, the timing is undefined (that is, 
* it could occur before or after the call returns, but it will typically be after).
* A Session connection timer, controlled by the Session property
* ::SOLCLIENT_SESSION_PROP_CONNECT_TIMEOUT_MS, controls the maximum amount of
* time a Session connect attempt lasts for. If this amount time is exceeded,
* a ::SOLCLIENT_SESSION_EVENT_CONNECT_FAILED_ERROR event is issued for the Session.
* If there is an error when solClient_session_connect() is invoked, ::SOLCLIENT_FAIL
* is returned, and a Session event is not subsequently issued. Therefore, the caller must
* check for a return code of ::SOLCLIENT_FAIL if it has logic that depends upon a subsequent
* Session event to be issued.
* For a non-blocking Session connect invocation, if the Session connect attempt eventually
* fails, the last error information to indicate the reason for the failure cannot be
* determined by the calling thread, rather it must be discovered through the Session event
* callback (and solClient_getLastErrorInfo can be called in the Session event callback
* to get further information).
* For a blocking Session connect invocation, if the Session connect attempt does not
* return ::SOLCLIENT_OK, then the calling thread can determine the failure reason by immediately
* calling solClient_getLastErrorInfo.
* \n 
*
* @param opaqueSession_p The opaque Session that was returned when Session was created.
*  @return ::SOLCLIENT_OK (blocking mode only), ::SOLCLIENT_NOT_READY (blocking mode only), ::SOLCLIENT_IN_PROGRESS (non-blocking mode only) or ::SOLCLIENT_FAIL.
* @subcodes
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* 
* The following subcodes can occur when a blocking connection operation is used (refer to ::SOLCLIENT_SESSION_PROP_CONNECT_BLOCKING). 
* Otherwise, such errors are reported when a ::SOLCLIENT_SESSION_EVENT_CONNECT_FAILED_ERROR Session event is received.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed out trying to connect the Session.
* @li ::SOLCLIENT_SUBCODE_LOGIN_FAILURE
* @li ::SOLCLIENT_SUBCODE_MSG_VPN_NOT_ALLOWED
* @li ::SOLCLIENT_SUBCODE_MSG_VPN_UNAVAILABLE
* @li ::SOLCLIENT_SUBCODE_CLIENT_USERNAME_IS_SHUTDOWN
* @li ::SOLCLIENT_SUBCODE_DYNAMIC_CLIENTS_NOT_ALLOWED
* @li ::SOLCLIENT_SUBCODE_CLIENT_NAME_ALREADY_IN_USE
* @li ::SOLCLIENT_SUBCODE_INVALID_VIRTUAL_ADDRESS
* @li ::SOLCLIENT_SUBCODE_CLIENT_DELETE_IN_PROGRESS
* @li ::SOLCLIENT_SUBCODE_TOO_MANY_CLIENTS
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_FLOW_NAME
* @li ::SOLCLIENT_SUBCODE_CONTROL_OTHER
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_connect (solClient_opaqueSession_pt opaqueSession_p);

/**
* Disconnects the specified Session. Once disconnected, topics/subscriptions
* can no longer be added or removed from the Session, messages can no longer be received for
* the Session, and messages cannot be sent to the Session. The Session definition remains,
* and the Session can be connected again (using solClient_session_connect()).
* When solClient_session_disconnect() is called, if there are buffered messages waiting to
* be transmitted for the Session (for example, because the send socket is full), the caller is
* blocked until all buffered data has been written to the send socket. Note the following: 1) This
* is done regardless of whether the Session has been configured for a blocking or non-blocking
* send operation (see ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING).
* 2) A call to solClient_session_destroy(), solClient_context_destroy(), or solClient_cleanup() 
* while a Session is connected (without first disconnecting the Session explicitly through a 
* call to solClient_session_disconnect()) discards any buffered messages.
* @param opaqueSession_p The opaque Session returned when Session was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_disconnect (solClient_opaqueSession_pt opaqueSession_p);

/**
* This is a deprecated function. It used to be required to call this function to start the 
* Guaranteed Delivery protocol before Guaranteed messages could be sent.
* This is now done automatically as part of solClient_session_connect().
* New applications should not call this function, and they should not rely on 
* the associated Session events ::SOLCLIENT_SESSION_EVENT_ASSURED_PUBLISHING_UP
* and ::SOLCLIENT_SESSION_EVENT_ASSURED_CONNECT_FAILED. To maintain backwards
* compatability with existing applications, if 
* solClient_session_startAssuredPublishing() is called
* (which is only allowed when the Session is in an established state), the
* application gets one of the following events: ::SOLCLIENT_SESSION_EVENT_ASSURED_PUBLISHING_UP,
* if sending of Guaranteed messages is allowed on the Session; or 
* ::SOLCLIENT_SESSION_EVENT_ASSURED_CONNECT_FAILED.
*
* A new application can call 
* solClient_session_send to send a Persistent or Non-Persistent
* message. A failure is returned if sending Guaranteed
* messages is not allowed on the Session (that is, if the Session is connected to an appliance
* that does not support Guaranteed Messaging).
* \n 
*
* @param opaqueSession_p The opaque Session returned when Session was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - Thrown when an attempt to start Guaranteed Messaging
* is made on a Session that does not support it.\n
* @li ::SOLCLIENT_SUBCODE_ASSURED_MESSAGING_STATE_ERROR - Thrown when an attempt to start Guaranteed Messaging is made on a Session that is not established. \n
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_startAssuredPublishing (solClient_opaqueSession_pt
                                              opaqueSession_p);

/**
* Sends a message on the specified Session. The message is composed of a number of optional
* components that are specified by the msg_p. The application should first 
* allocate a solClient_msg, then use the methods defined in solClientMsg.h to
* build the message to send.
*
* solClient_session_sendMsg() returns SOLCLIENT_OK when the message has been successfully 
* copied to the transmit buffer or underlying transport, this does not guarantee successful
* delivery to the Solace messaging appliance. When sending Guaranteed messages (persistent or non-persistent),
* the application will receive a subsequent ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT event for all
* messages successfully delivered to the Solace messaging appliance.  For Guaranteed messages, notifications of
* quota, permission, or other delivery problems will be indicated in a ::SOLCLIENT_SESSION_EVENT_REJECTED_MSG_ERROR
* event.
*
* <b>Special Buffering of Guaranteed Messages</b>\n
* Guaranteed messages (::SOLCLIENT_DELIVERY_MODE_PERSISTENT or ::SOLCLIENT_DELIVERY_MODE_NONPERSISTENT) are 
* assured by the protocol between the client and the Solace message router.  To make developers' task easier, 
* guaranteed messages are queued for delivery in many instances:
* @li While transport (TCP) flow controlled.
* @li While message router flow controlled.
* @li While sessions are connecting or reconnecting.
* @li While sessions are disconnected or down.
*
* The C-SDK will buffer up to a publishers window size (::SOLCLIENT_SESSION_PROP_PUB_WINDOW_SIZE) of guaranteed messages before 
* solClient_session_sendMsg() will either block (when ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING is enabled) or return ::SOLCLIENT_WOULD_BLOCK
* (on active sessions) or return ::SOLCLIENT_NOT_READY (on disconnected sessions).
*
* For the most part this is desired behavior. Transient sessions failures do not require special handling in applications. When 
* ::SOLCLIENT_SESSION_PROP_RECONNECT_RETRIES is non-zero, the underlying transport will automatically reconnect and the publishing 
* application does not need to concern itself with special handling for the transient reconnecting state.
*
* @param opaqueSession_p The opaque Session returned when the Session was created.
* @param msg_p           The opaque message created by solClient_msg_alloc.
* @see @ref adConsiderations
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to send a direct (non-guaranteed) message on a non-established
* Session. Guaranteed messages are queued for later transmission on non-established Sessions.
* @li ::SOLCLIENT_SUBCODE_TOPIC_TOO_LARGE
* @li ::SOLCLIENT_SUBCODE_TOPIC_MISSING
* @li ::SOLCLIENT_SUBCODE_USER_DATA_TOO_LARGE
* @li ::SOLCLIENT_SUBCODE_QUEUENAME_TOO_LARGE
* @li ::SOLCLIENT_SUBCODE_QUEUENAME_INVALID_MODE
* @li ::SOLCLIENT_SUBCODE_QUEUENAME_TOPIC_CONFLICT
* @li ::SOLCLIENT_SUBCODE_MAX_TOTAL_MSGSIZE_EXCEEDED
* @li ::SOLCLIENT_SUBCODE_DELIVER_TO_ONE_INVALID
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - An attempt was made to send a Guaranteed message to a Session that does not support Guaranteed Messaging.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write a message to socket,
*     waiting for Session to be established (only for blocking sends; refer to ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING), 
*     or waiting for an open publisher window (only when using ::SOLCLIENT_DELIVERY_MODE_PERSISTENT or 
*     ::SOLCLIENT_DELIVERY_MODE_NONPERSISTENT with blocking send operation).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_sendMsg (solClient_opaqueSession_pt opaqueSession_p,
                            solClient_opaqueMsg_pt msg_p);

/**
* Sends the given Solace Message Format (SMF) message on the specified Session. 
*
* solClient_session_sendSmf() returns SOLCLIENT_OK when the message has be successfully 
* copied to the transmit buffer or underlying transport, this does not guarantee successful
* delivery to the Solace messaging appliance. 
*
* @param opaqueSession_p The opaque Session returned when the Session was created.
* @param smfBufInfo_p    A pointer to the bufInfo describing a validly formatted SMF direct message.
*
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to send a Direct (that is, a non-Guaranteed) message on a non-established
* Session.
* @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The message was too large to be buffered.
* @li ::SOLCLIENT_SUBCODE_INVALID_SMF - The buffer does not contain a validly formatted SMF message.
* @li ::SOLCLIENT_SUBCODE_INVALID_SMF_MESSAGE - The buffer contains a valid SMF message, but it is not a Direct message. Only Direct messages can be sent.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write a message to socket,
*     waiting for Session to be established (only for blocking sends; refer to ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING) 
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_sendSmf (solClient_opaqueSession_pt opaqueSession_p,
                               solClient_bufInfo_pt       smfBufInfo_p);

/**
 * Sends multiple messages on the specified Session. For direct messages, using this function is more efficient than multiple calls to
 * solClient_session_send(). This routine is suitable when an application is able to construct multiple
 * messages at once from a single stimulus.
 * 
 * Note that the number of messages which can be sent through a single call to solClient_session_sendMultipleMsg() 
 * is limited to ::SOLCLIENT_SESSION_SEND_MULTIPLE_LIMIT.
 *
 * For Sessions in which solClient_session_sendMultipleMsg() is used, it is recommended that the Session
 * property ::SOLCLIENT_SESSION_PROP_TCP_NODELAY be enabled (it is enabled by default).
 * With solClient_session_sendMultipleMsg() multiple messages are sent at once onto a TCP connection, and therefore
 * there is no need to have the operating system carry out the TCP delay algorithm to cause fuller IP packets.
 * 
 * Guaranteed Delivery messages may be specified. It is recommended that all messages to be sent in a batch  have
 * the same delivery mode.
 *
 * solClient_session_sendMultipleMsg() returns SOLCLIENT_OK when the messages have been successfully 
 * copied to the transmit buffer or underlying transport, this does not guarantee successful
 * delivery to the Solace messaging appliance. When sending Guaranteed messages (persistent or non-persistent),
 * the application will receive a subsequent ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT event for all
 * messages successfully delivered to the Solace messaging appliance.  For Guaranteed messages, notifications of
 * quota, permission, or other delivery problems will be indicated in a ::SOLCLIENT_SESSION_EVENT_REJECTED_MSG_ERROR
 * event.
 *
 * solClient_session_sendMultipleMsg() takes as an argument a location to store the number of 
 * messages written (<i>numberOfMessagesWritten</i>). This value will always be equal to the requested
 * <i>numberOfMessages</i> when ::SOLCLIENT_OK is returned.   However on all other return values, it may be
 * possible that part of the message array (<i>msgArray_p</i>) has been written and this value <b>must</b> be
 * checked by applications that need to know which messages have been sent and which have not.
 *
 * @param opaqueSession_p         The opaque Session returned when the Session was created.
 * @param msgArray_p              A pointer to an array of solClient_opaqueMsg_pt pointers. Each entry describes one message to be sent.
 * @param numberOfMessages        The number of messages provided in the message array.
 * @param numberOfMessagesWritten A pointer to the variable to receive the returned number of messages accepted by SolClient.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
 */
  solClient_dllExport solClient_returnCode_t
    solClient_session_sendMultipleMsg( solClient_opaqueSession_pt opaqueSession_p,
                                       solClient_opaqueMsg_pt    *msgArray_p,
                                       solClient_uint32_t         numberOfMessages,
                                       solClient_uint32_t        *numberOfMessagesWritten);

/**
 * Sends multiple Solace Message Format (SMF) messages on the specified Session. 
 *
 * @param opaqueSession_p The opaque Session returned when the Session was created.
 * @param smfBufInfo_p    A pointer to the bufInfo array describing validly formatted SMF Direct message.
 * @param numberOfMessages The number of messages in the smfBufInfo_p array.
 *
 * solClient_session_sendMultipleSmf() returns SOLCLIENT_OK when the message has be successfully 
 * copied to the transmit buffer or underlying transport, this does not guarantee successful
 * delivery to the Solace messaging appliance. 
 *
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to send a Direct (that is, non-Guaranteed) message on a non-established
 * Session.
 * @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The message was too large to be buffered.
 * @li ::SOLCLIENT_SUBCODE_INVALID_SMF - The buffer does not contain a validly formatted SMF message.
 * @li ::SOLCLIENT_SUBCODE_INVALID_SMF_MESSAGE - The buffer contains a valid SMF message, but is not a direct (reliable) message suitable for replay.
 * @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write a message to socket,
 *     waiting for Session to be established (only for blocking sends; refer to ::SOLCLIENT_SESSION_PROP_SEND_BLOCKING). 
 * @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
 * @see ::solClient_subCode for a description of all subcodes.
 */
  solClient_dllExport solClient_returnCode_t
    solClient_session_sendMultipleSmf (solClient_opaqueSession_pt opaqueSession_p,
                                       solClient_bufInfo_pt       smfBufInfo_p,
                                       solClient_uint32_t         numberOfMessages);
/**
* Adds a Topic subscription to a Session. Messages matching the subscription
* are delivered to the Session's message receive callback. When Topic dispatching is in use,
* messages matching this subscription are only delivered to the Session callback if they 
* are not delivered to a more specific callback.
* @see @ref subscription-syntax
* @see @ref topic-dispatch
*
* @param opaqueSession_p An opaque Session returned when the Session was created.
* @param topicSubscription_p The Topic subscription string (a NULL-terminated UTF-8 string).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed out trying to write subscribe request to socket, 
*     waiting for Session to be established, or waiting for internal resources.
*     (Only for blocking subscription operations. Refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING)
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_topicSubscribe (solClient_opaqueSession_pt opaqueSession_p,
                                      const char *topicSubscription_p);

/**
* Adds a Topic subscription to a Session. This extended version of the subscribe function
* allows for more control of the operation through flags. Messages matching the subscription
* are delivered to the Session's message receive callback. When Topic dispatching is in use,
* messages matching this subscription are only delivered to the Session callback if they 
* are not delivered to a more specific callback.
* @see @ref subscription-syntax
* @see @ref topic-dispatch
*
* @param opaqueSession_p The opaque Session returned when Session was created.
* @param flags \ref subscribeflags "Flags" to control the operation. Valid flags for this operation are:
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE
*
* @param topicSubscription_p The Topic subscription string (a NULL-terminated UTF-8 string).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write a subscribe request to socket, 
*     waiting for a Session to be established, or waiting for internal resources.
*     (Only for blocking subscription operations. Refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING);
*     or timed-out waiting for a confirm (only when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
*
* The following subcodes can occur when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. Otherwise, such errors are reported
* when a ::SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR Session event is received.
* @li ::SOLCLIENT_SUBCODE_OUT_OF_RESOURCES - The appliance cannot accept any more Topic subscriptions. (This subcode only occurs when using the Topic Routing Blade.)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ALREADY_PRESENT (see ::SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_TOO_MANY
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_topicSubscribeExt (
                                      solClient_opaqueSession_pt opaqueSession_p,
                                      solClient_subscribeFlags_t flags,
                                      const char *topicSubscription_p);
                                      
/**
* Adds a Topic subscription to a Session like ::solClient_session_topicSubscribeExt(), 
* but this function also allows a different message receive callback and dispatchUser_p to be specified.
* Specifying a NULL funcInfo_p or if funcInfo_p references a NULL  dispatchCallback_p and a NULL dispatchUser_p makes this function
* act the same as ::solClient_session_topicSubscribeExt(). Used in this manner, an application can set the correlationTag, which appears in asynchronouus confirmations (::SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_OK). Setting correlationTag is not available when using 
* ::solClient_session_topicSubscribeExt().
* 
* Usually this API is used to provide a separate callback and user pointer for messages received on the given topic.
* The Session property ::SOLCLIENT_SESSION_PROP_TOPIC_DISPATCH must be enabled for a non-NULL callback to be
* specified. When funcInfo_p is non-NULL and a dispatchCallback_p is specified, the callback pointer and dispatchUser_p are stored
* in an internal callback table. funcInfo_p is <b>not</b> saved by the API.
*
* @see @ref subscription-syntax
* @see @ref topic-dispatch
*
* @param opaqueSession_p The opaque Session returned when Session was created.
* @param flags \ref subscribeflags "Flags" to control the operation. Valid flags for this operation are:
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY
*
* @param topicSubscription_p The Topic subscription string (a NULL-terminated UTF-8 string).
* @param funcInfo_p         The message receive callback information. See struct solClient_session_rxMsgDispatchFuncInfo
* @param correlationTag     A correlationTag pointer that is returned as is in the confirm or fail sessionEvent for the
*                           subscription. This is only used if SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM is set and
*                           SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM is not set.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS
* 
* A successful call has a return code of ::SOLCLIENT_OK, except when using ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM without 
* ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. In that case, the return code will be ::SOLCLIENT_IN_PROGRESS because the call returns without
* waiting for the operation to complete.
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write a subscribe request to socket, 
*     waiting for a Session to be established, or waiting for internal resources.
*     This subcode only occurs for blocking subscription operations (refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING),
*     or the operation timed-out waiting for a confirm (only when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @li ::SOLCLIENT_SUBCODE_PARAM_CONFLICT - If subscribe flags ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM or ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
*     are used with ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY.
*
* The following subcodes can occur when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. Otherwise, such errors are reported
* when a ::SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR Session event is received.
* @li ::SOLCLIENT_SUBCODE_OUT_OF_RESOURCES - The appliance cannot accept any more Topic subscriptions. (This subcode only occurs using the Topic Routing Blade.)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ALREADY_PRESENT (see ::SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_TOO_MANY
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION
* @see ::solClient_subCode for a description of all subcodes.
*
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_topicSubscribeWithDispatch(
                         solClient_opaqueSession_pt                 opaqueSession_p,
                         solClient_subscribeFlags_t                 flags,
                         const char                                *topicSubscription_p,
                         solClient_session_rxMsgDispatchFuncInfo_t *funcInfo_p,
                         void                                      *correlationTag);

/**
* Removes a Topic subscription from a Session.
* @see @ref subscription-syntax
* @see @ref topic-dispatch
*
* @param opaqueSession_p The opaque Session returned when Session was created.
* @param topicSubscription_p The Topic subscription string (a NULL-terminated UTF-8 string).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed out trying to write unsubscribe request to socket,  
*     waiting for Session to be established, or waiting for internal resources.
*      This subcode only occurs for blocking subscription operations (refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_topicUnsubscribe (solClient_opaqueSession_pt
                                        opaqueSession_p,
                                        const char *topicSubscription_p);
/**
* Removes a Topic subscription from a Session. This extended version of the Topic unsubscribe function
* allows for more control of the operation through flags.
* @see @ref subscription-syntax
* @see @ref topic-dispatch
*
* @param opaqueSession_p The opaque Session returned when Session was created.
* @param flags \ref subscribeflags "Flags" to control the operation. Valid flags for this operation are:
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE
*
* @param topicSubscription_p The Topic subscription string (a NULL-terminated UTF-8 string).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed out trying to write unsubscribe request to socket, 
*     waiting for Session to be established, waiting for internal resources.
*      (This subcode only occurs only for blocking subscription operations; refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING),
*     or waiting for confirm (only when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed
*
* The following subcodes can occur when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. Otherwise, such errors are reported
* when a ::SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR Session event is received.
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_NOT_FOUND (see ::SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_topicUnsubscribeExt (solClient_opaqueSession_pt
                                        opaqueSession_p,
                                        solClient_subscribeFlags_t flags,
                                        const char *topicSubscription_p);

/**
* Removes a Topic subscription from a Session like ::solClient_session_topicUnsubscribeExt(), 
* but this function also allows a message receive callback and dispatchUser_p to be specified.
* Specifying a NULL funcInfo_p or if funcInfo_p references a NULL  dispatchCallback_p and a NULL dispatchUser_p makes this function
* act the same as ::solClient_session_topicUnsubscribeExt(). Used in this manner, an application can set the correlationTag which appears in asynchronouus confirmations (::SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_OK). Setting correlationTag is not available when using 
* ::solClient_session_topicUnsubscribeExt().
* 
* Usually this API is used to provide a separate callback and user pointer for messages received on the given topic.
* The Session property ::SOLCLIENT_SESSION_PROP_TOPIC_DISPATCH must be enabled for a non-NULL callback to be
* specified. When funcInfo_p is non-NULL and a dispatchCallback_p is specified, the callback pointer and dispatchUser_p are removed
* from an internal callback table. funcInfo_p does not have to match the funcInfo_p used in ::solClient_session_topicSubscribeWithDispatch(). However,
* the contents referenced in funcInfo_p must match an entry found in the callback table.
*
* @see @ref subscription-syntax
* @see @ref topic-dispatch
*
* @param opaqueSession_p The opaque Session returned when Session was created.
* @param flags \ref subscribeflags "Flags" to control the operation. Valid flags for this operation are:
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY
*
* @param topicSubscription_p The Topic subscription string (a NULL-terminated UTF-8 string).
* @param funcInfo_p         The message receive callback information. See struct solClient_session_rxMsgDispatchFuncInfo.
* @param correlationTag     A correlationTag pointer that is returned as is in the confirm or fail sessionEvent for the
*                           subscription. This is only used if SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM is set and
*                           SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM is not set.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS
* 
* A successful call has a return code of ::SOLCLIENT_OK, except when using ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM without 
* ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM, then the return code is ::SOLCLIENT_IN_PROGRESS because the call returns without
* waiting for the operation to complete.
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - An attempt was made to operate on a non-established Session.
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write unsubscribe request to socket, 
*     waiting for Session to be established, waiting for internal resources.
*      This subcode only occurs for blocking subscription operations (refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING);
*     or the operation timed-out waiting for a confirmation (only when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @li ::SOLCLIENT_SUBCODE_PARAM_CONFLICT - If subscribe flags ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM or ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
*     are used with ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY.
*
* The following subcodes can occur when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. Otherwise, such errors are reported
* when a ::SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR Session event is received.
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_NOT_FOUND (see ::SOLCLIENT_SESSION_PROP_IGNORE_DUP_SUBSCRIPTION_ERROR)
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_OTHER
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION
* @see ::solClient_subCode for a description of all subcodes.
*
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_topicUnsubscribeWithDispatch(
                          solClient_opaqueSession_pt                 opaqueSession_p,
                          solClient_subscribeFlags_t                 flags,
                          const char                                *topicSubscription_p,
                          solClient_session_rxMsgDispatchFuncInfo_t *funcInfo_p,
                          void                                      *correlationTag);

/**
* Check a Topic string against Topic encoding rules for the appliance. This
* routine is intended for use with solClient_session_sendMsg, and the Topic 
* set in the \link ::solClient_opaqueMsg_pt opaque message pointer\endlink. This
* function should not be used with \link ::solClient_session_topicSubscribe 
* solClient_session_topicSubscribe.\endlink  solClient_session_topicSubscribe 
* does its
* own Topic validation, which includes accepting wildcards. For performance
* reasons, solClient_session_sendMsg does not validate topics, and this function
* is provided as a convenience to developers to ensure a Topic is valid
* before sending a message that could be rejected by the appliance. It is
* expected then, that the same Topic be used for sending many messages.
*
* @param opaqueSession_p The opaque Session returned when Session was created.
* @param topicString_p The Topic string (a NULL-terminated UTF-8 string).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @see ::solClient_subCode for a description of all subcodes.
*/

  solClient_dllExport solClient_returnCode_t 
  solClient_session_validateTopic (solClient_opaqueSession_pt opaqueSession_p,
                                   const char *topicString_p);
/**
 * Sends a Topic Endpoint unsubscribe command to the appliance. This is 
 * only valid if no subscribers are bound to the Topic Endpoint. 
 * The application can specify a correlation tag to match up responses. The 
 * correlation tag is a void pointer with no significance to the API. When the appliance responds to the 
 * unsubscribe command, the correlation tag is returned in the eventInfo structure of the callback event.
 * If this command succeeds there is a later event callback of either 
 * ::SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_OK or 
 * ::SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_ERROR.
 *  
 *
 * @param opaqueSession_p An opaque Session pointer returned when Session was created.
 * @param teName_p       A string containing the Topic Endpoint name.
 * @param correlationTag  Correlation tag returned in the resulting Session event.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, or ::SOLCLIENT_FAIL
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - Attempted to operate on a non-established Session.
 * @li ::SOLCLIENT_SUBCODE_TIMEOUT - Timed-out trying to write a subscribe request to socket,  
 *     waiting for a Session to be established, or waiting for internal resources
 *     (only for blocking subscription operations; refer to ::SOLCLIENT_SESSION_PROP_SUBSCRIBE_BLOCKING);
 *     or timed-out waiting for a confirm (only when using ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM).
 * @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
 * @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - The Session does not support Guaranteed messaging.
 * @see ::solClient_subCode for a description of all subcodes.
 */
  solClient_dllExport solClient_returnCode_t
    solClient_session_dteUnsubscribe (solClient_opaqueSession_pt opaqueSession_p, 
                                      const char *teName_p,
                                      void       *correlationTag);

/**
* Returns an array of Session receive statistics.
* If the array is smaller than the number of defined receive statistics, only the first N
* defined statistics are returned.
* If the array is larger than the number of defined receive statistics, only the defined
* entries are filled in. The other entries are not touched.
* @param opaqueSession_p An opaque Session returned when the Session was created.
* @param rxStats_p       A pointer to an array of statistic values of type solClient_stats_t.
* @param arraySize       The number of entries in the passed in array (not the number of bytes).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_getRxStats (solClient_opaqueSession_pt opaqueSession_p,
                                  solClient_stats_pt rxStats_p,
                                  solClient_uint32_t arraySize);

/**
* Returns an individual receive statistic.
* If multiple receive statistics are needed, it is more efficient to use
* solClient_session_getRxStats rather than to call this routine multiple times for
* different statistics.
* @param opaqueSession_p The opaque Session returned when the Session was created.
* @param rxStatType      The type of receive statistic to return.
* @param rxStat_p        A pointer to a variable to hold the returned statistic.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @see solClient_session_getRxStats()
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_getRxStat (solClient_opaqueSession_pt opaqueSession_p,
                                 solClient_stats_rx_t rxStatType,
                                 solClient_stats_pt rxStat_p);

/**
* Returns an array of Session transmit statistics.
* If the array is smaller than the number of defined transmit statistics, only the first N
* defined statistics are returned.
* If the array is larger than the number of defined transmit statistics, only the defined
* entries are filled in. The other entries are not touched.
* @param opaqueSession_p The opaque Session returned when the Session was created.
* @param txStats_p       A pointer to an array of statistic values of type solClient_Stats_t.
* @param arraySize       The number of entries in the array passed in (NOT the number of bytes).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_getTxStats (solClient_opaqueSession_pt opaqueSession_p,
                                  solClient_stats_pt txStats_p,
                                  solClient_uint32_t arraySize);

/**
* Returns an individual transmit statistic.
* If multiple transmit statistics are needed, it is more efficient to use
* solClient_session_getTxStats rather than to call this routine multiple times for
* different statistics.
* @param opaqueSession_p An opaque Session returned when Session was created.
* @param txStatType      The type of transmit statistic to return.
* @param txStat_p        A pointer to a variable to hold the returned statistic.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @see solClient_session_getTxStats()
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_getTxStat (solClient_opaqueSession_pt opaqueSession_p,
                                 solClient_stats_tx_t txStatType,
                                 solClient_stats_pt txStat_p);

/**
* Clears all of the receive and transmit statistics for the specified Session. All previous
* Session statistics are lost when this is called.
* @param opaqueSession_p The opaque Session returned when Session was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_clearStats (solClient_opaqueSession_pt opaqueSession_p);

/**
* Outputs a log at the specified log level at this moment containing the value of all
* receive and transmit statistics for the specified Session. This routine is useful for
* application debugging because it allows an application to easily output all available
* Session statistics.
* @param opaqueSession_p The opaque Session returned when the Session was created.
* @param level The log level used to output the statistics log.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_logStats (solClient_opaqueSession_pt opaqueSession_p,
                                solClient_log_level_t level);


/**
 * Create a temporary Topic string. This string may be passed as the Topic to
 * ::solClient_session_createFlow() when connecting with a non-durable Topic 
 * Endpoint. It may also be used in a ::solClient_destination_t that is sent
 * to a peer application in the ReplyTo field of the Solace Header map or in a 
 * structured data field.
 *
 * @param opaqueSession_p The opaque Session returned when the Session was created.
 * @param topic_p         A pointer to a string location where the string is returned.
 * @param length          The maximum string length to return.
 * @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The generated Topic would be longer than the 
 * available length.
 * @li ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED - The Session must be established before 
 * temporary Topic strings can be generated.
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_session_createTemporaryTopicName(solClient_opaqueSession_pt opaqueSession_p,
                                       char                      *topic_p,
                                       size_t                     length);

/**
* Outputs a log at the specified log level at this moment containing the Flow state information
* for all Flows at this moment. It also dumps a summary of the number of active
* Flows, as well as the allocated but unbound Flows.
* @param opaqueSession_p The opaque Session returned when Session was created.
* @param level The log level used to output the statistics log.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
solClient_dllExport solClient_returnCode_t
solClient_session_logFlowInfo(solClient_opaqueSession_pt opaqueSession_p,
                             solClient_log_level_t level );


/**
 * Send a Topic Request message. The application expects an end-to-end reply
 * from the client that receives the message. 
 * 
 * If the Reply-To destination in the Solace header map is not set, it is set to the default Session
 * replyTo destination. Leaving the replyTo destination unset and allowing the API to
 * set the default replyTo destination is the easiest way to set a valid replyTo destination.
 *
 * When the application needs to do a non-blocking request (that is, the timeout parameter is zero), the application
 * may set any replyTo topic destination.
 *
 * When the application needs to do a blocking request (that is, the timeout parameter is non-zero),
 * the replyTo destination must be a topic that the application has subscribed to for Direct messages.
 * If the replyTo destination is set to an unsubscribed topic, a call to solClient_session_sendRequest()
 * will block until the amount of time set for the timeout parameter expires and then return
 * ::SOLCLIENT_INCOMPLETE with subcode ::SOLCLIENT_SUBCODE_TIMEOUT.
 *
 * If the timeout parameter is zero, this function returns immediately with ::SOLCLIENT_IN_PROGRESS upon
 * successful buffering of the message for transmission. Any response generated by the destination client
 * is delivered to the replyTo destination as a receive message callback with the response attribute set -
 * solClient_msg_isReplyMsg() returns true. It is entirely within the responsibility of the
 * application to manage asynchronous responses.
 *
 * When the timeout parameter is non-zero, this function waits for the amount of time specified by the timeout parameter for a
 * response before returning ::SOLCLIENT_INCOMPLETE, otherwise this function returns ::SOLCLIENT_OK.
 * If replyMsg_p is non-NULL, this functions returns an opaque message pointer (::solClient_opaqueMsg_pt) in the location
 * referenced by replyMsg_p. This message is allocated by the API and contains the received reply. This function allocates
 * the message on behalf of the application and the application <b>must</b> later release the message by calling
 * solClient_msg_free(replyMsg_p).
 *
 * The API only allows one blocking request (timeout non-zero) at a time. If a multiple threaded application calls 
 * solClient_session_sendRequest() from multiple threads at the same time, the first processed request is sent and
 * all other requests block until the first response is received, then the next request is sent. This will consume
 * some of the timeout period for each request.
 *
 * If this function does not return ::SOLCLIENT_OK, and replyMsg_p is non-NULL, then the location referenced by
 * replyMsg_p is set to NULL.
 *
 * @param opaqueSession_p The opaque Session pointer that is returned when the Session was
 *                        created.
 * @param msg_p          A pointer to a solClient_msgBuffer that contains the 
 *                       message to be sent.
 * @param replyMsg_p     A reference to a solClient_msgBuffer pointer that will 
 *                       receive the reply message pointer. If NULL, then 
 *                       only status is returned. If non-NULL the application must
 *                       call solClient_msg_free() for the replyMsg when it is 
 *                       no longer needed.
 * @param timeout        The maximum time (in milliseconds) to wait for reply.
 *                       If timeout is set to zero then the function will return
 *                       immediately ::SOLCLIENT_IN_PROGRESS after buffering of
 *                       the message for transmission.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_IN_PROGRESS, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK,
 *         ::SOLCLIENT_INCOMPLETE
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_session_sendRequest (solClient_opaqueSession_pt opaqueSession_p,
                            solClient_opaqueMsg_pt msg_p,
                            solClient_opaqueMsg_pt *replyMsg_p,
                            solClient_uint32_t     timeout);

/**
* Send a Reply Message. This function constructs a Solace binary 
* message header based on the received message and sends a reply to 
* the correct destination. If rxmsg_p is NULL, the application is responsible for setting 
* a destination and correlationId string in the replyMsg. Otherwise the following fields
* from the rxmsg are used in the replyMsg:
*
* @li ReplyTo If replyMsg has no destination, the rxmsg ReplyTo destination is used.
* @li CorrelationId If replyMsg has no correlationId string, the rxmsg correlationId is copied.
*
* If replyMsg is null, the API creates a replyMsg that contains only correlationId string.
*
* @param opaqueSession_p The opaque Session pointer returned when the Session was
*                        created.
* @param rxmsg_p        A pointer to a solClient_msgBuffer that contains the 
*                       message to reply to. (optional)
* @param replyMsg_p     A pointer to a solClient_msgBuffer that contains the 
*                       message to be sent. (optional)
* @returns ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL
* @subcodes
* @li ::SOLCLIENT_SUBCODE_MISSING_REPLY_TO - the rxmsg_p (if not NULL) does not have a reply-to and so a reply cannot be sent.
* @see ::solClient_subCode for a description of all subcodes.
*/
solClient_dllExport solClient_returnCode_t
solClient_session_sendReply (solClient_opaqueSession_pt opaqueSession_p,
                            solClient_opaqueMsg_pt rxmsg_p,
                            solClient_opaqueMsg_pt replyMsg_p);

/**
* Returns a string representation of the Session event passed in.
* @param sessionEvent The Session event to convert to a string representation.
* @return A pointer to a constant character string. This pointer is never NULL.
*/
  solClient_dllExport const char
    *solClient_session_eventToString (solClient_session_event_t sessionEvent);


/**
* Provision, on the appliance, a durable Queue or Topic Endpoint using the specified Session. 
* ::SOLCLIENT_ENDPOINT_PROP_ID must be set to either ::SOLCLIENT_ENDPOINT_PROP_QUEUE or ::SOLCLIENT_ENDPOINT_PROP_TE
* in this interface. Only durable (::SOLCLIENT_ENDPOINT_PROP_DURABLE is enabled) endpoints may be provisioned. A non-durable
* endpoint is created when a Flow is bound to it with solClient_session_createFlow().
*
* Endpoint creation can be carried out in a blocking or non-blocking mode, depending upon the \ref provisionflags "provisionFlags".
* If SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM is set in provisionFlags,
* the calling thread is blocked until the endpoint creation attempt either
* succeeds or is determined to have failed. If the endpoint is created, ::SOLCLIENT_OK is
* returned.
* When SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM is not set, ::SOLCLIENT_IN_PROGRESS is returned when the endpoint
* provision request is successfully sent, and the creation attempt proceeds in the background.
* An endpoint creation timer, controlled by the property
* ::SOLCLIENT_SESSION_PROP_PROVISION_TIMEOUT_MS, controls the maximum amount of
* time a creation attempt lasts for. Upon expiry of this time,
* a ::SOLCLIENT_SESSION_EVENT_PROVISION_ERROR event is issued for the Session.
* If there is an error when solClient_session_endpointProvision() is invoked, then ::SOLCLIENT_FAIL
* is returned, and a provision event will not be subsequently issued. Thus, the caller must
* check for a return code of ::SOLCLIENT_FAIL if it has logic that depends upon a subsequent
* provision event to be issued.
* For a non-blocking endpoint provision, if the creation attempt eventually
* fails, the error information that indicates the reason for the failure cannot be
* determined by the calling thread, rather it must be discovered through the Session event
* callback (and solClient_getLastErrorInfo() can be called in the Session event callback
* to get further information).
* For a blocking endpoint creation invocation, if the creation attempt does not
* return ::SOLCLIENT_OK, then the calling thread can determine the failure reason by immediately
* calling solClient_getLastErrorInfo. For a blocking endpoint creation, ::SOLCLIENT_NOT_READY is returned
* if the create failed due to the timeout expiring (see ::SOLCLIENT_SESSION_PROP_PROVISION_TIMEOUT_MS).
* 
* @param props           The provision \ref endpointProps "properties" used to define the endpoint.
* @param opaqueSession_p The Session which is used to create the endpoint.
* @param provisionFlags  \ref provisionflags "Flags" to control provision operation.
* @param correlationTag  A correlation tag returned in the resulting Session event.
* @param queueNetworkName    This parameter is deprecated but still supported for backwards compatibility.
*                         It is recommended to pass NULL for this parameter.  When a non-null pointer is passed,
*                         it is used as a pointer to the location in which the network name of the created Queue
*                         Network Name is returned. This can be used to set the destination for published
*                         messages. An empty string is returned when the created endpoint is a Topic Endpoint.
*                         For publishing to a queue, the current recommended practice is to use
*                         solClient_destination_t where the destType is set to SOLCLIENT_QUEUE_DESTINATION and
*                         dest is set to the queue name.
* @param qnnSize          As with queueNetworkName, this is a deprecated paramter.  When passing NULL as the
*                         queueNetworkName, pass 0 as qnnSize.  When queueNetworkName is not null, qnnSize is the
*                         maximum length of the Queue Network Name string that can be returned.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_IN_PROGRESS, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - The operation timed out trying to provision (only for blocking provision operations; refer to ::SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_NAME_FOR_TE
* @li ::SOLCLIENT_SUBCODE_ENDPOINT_ALREADY_EXISTS
* @li ::SOLCLIENT_SUBCODE_ENDPOINT_PROPERTY_MISMATCH
* @li ::SOLCLIENT_SUBCODE_NO_MORE_NON_DURABLE_QUEUE_OR_TE
* @li ::SOLCLIENT_SUBCODE_PERMISSION_NOT_ALLOWED
* @li ::SOLCLIENT_SUBCODE_QUOTA_OUT_OF_RANGE
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_endpointProvision(solClient_propertyArray_pt       props,
                                        solClient_opaqueSession_pt       opaqueSession_p, 
                                        solClient_uint32_t               provisionFlags,
                                        void                            *correlationTag,
                                        char                            *queueNetworkName,
                                        size_t                           qnnSize); 

/** 
 * Remove an endpoint from the appliance. An application can only remove an endpoint that has previously
 * been provisioned with solClient_session_provistionEndpoint(). The appliance will reject with an error any
 * attempt to remove a temporary endpoint (provisioned by solClient_session_createFlow()), or a permanent endpoint
 * provisioned by the administrator through the CLI or solAdmin. 
 *
* @param props           The provision properties used to identify the endpoint.
* @param opaqueSession_p The Session which is used to delete the endpoint.
* @param provisionFlags  \ref provisionflags "Flags" to control delete operation.
* @param correlationTag  The correlation tag returned in the resulting Session event.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_IN_PROGRESS, ::SOLCLIENT_WOULD_BLOCK
* @subcodes
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - The operation timed out trying to provision (only for blocking provision operations; refer to ::SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_QUEUE_NAME
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_TE_NAME
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_endpointDeprovision( solClient_propertyArray_pt  props,
                                      solClient_opaqueSession_pt       opaqueSession_p, 
                                      solClient_uint32_t               provisionFlags,
                                      void                            *correlationTag);

/** 
 * Add a Topic subscription to the endpoint defined by the endpoint properties if the operation is supported 
 * on the endpoint. Topic subscriptionscan be added to Queue endpoints (::SOLCLIENT_ENDPOINT_PROP_QUEUE) and
 * to the special endpoint for each Session on appliances running SolOS-TR (::SOLCLIENT_ENDPOINT_PROP_CLIENT_NAME). 
 *
 * <b>Adding subscriptions to Queues</b>
 *
 * Guaranteed messages are published to a Queue when it is set as the destination of the message. However, 
 * you can also add one or more Topic subscriptions to a durable or temporary Queue so that 
 * Guaranteed messages published to those topics are also delivered to the Queue.
 * @li Only a Queue (::SOLCLIENT_ENDPOINT_PROP_QUEUE) Endpoint can be used.
 * @li Only a Topic subscription can be used.
 * @li Must have modify-topic permission on the Endpoint.
 *
 * <b>Adding subscriptions to remote clients</b>
 *
 * A subscription manager client can add subscriptions on behalf of other clients. These Topic 
 * subscriptions are not retained in the subscription cache and are not be reapplied upon reconnection.
 * @li Only supported in SolOS-TR mode. 
 * @li Only a ClientName (::SOLCLIENT_ENDPOINT_PROP_CLIENT_NAME) Endpoint can be used. 
 * @li Only a Topic subscription can be used.
 * @li Must have subscription-manager permissions on the client username.
 *
 * It is an error to add subscriptions to a Topic Endpoint (::SOLCLIENT_ENDPOINT_PROP_TE)  with this interface. 
 * A subscription can only be added to a Topic Endpoint when a Flow is bound to it. The subscription is part of the Flow properties.
 *
* @param endpointProps   The \ref endpointProps "provision properties" used to identify the endpoint.
* @param opaqueSession_p The Session which is used to perform the subscription request. If the Session also supports
*                        ::SOLCLIENT_SESSION_PROP_REAPPLY_SUBSCRIPTIONS, then the subscriptions are be remembered
*                        and automatically reapplied should the Session fail and reconnect automatically.
* @param flags           \ref subscribeflags "Flags" to control the operation. Valid flags for this operation are:
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE
* @param topicSubscription_p Topic Subscription to apply as a NULL-terminated UTF-8 string.
* @param correlationTag  Correlation tag returned in the resulting Session event.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - The operation timed out trying to provision (only for blocking provision operations; refer to ::SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_QUEUE_NAME
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_CLIENT_NAME
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_endpointTopicSubscribe( solClient_propertyArray_pt endpointProps,
                                              solClient_opaqueSession_pt opaqueSession_p,
                                              solClient_subscribeFlags_t flags,
                                              const char              *topicSubscription_p,
                                              void                    *correlationTag);

/** 
 * Remove a Topic subscription from the endpoint defined by the endpoint properties if the operation is supported 
 * on the endpoint. Used to remove subscriptions added by solClient_session_endpointTopicSubscribe() . 
 * Refer to that function's documentation for operational parameters. This method cannot remove subscriptions added through the CLI.
 * 
* @param endpointProps   The \ref endpointProps "provision properties" used to identify the endpoint.
* @param opaqueSession_p The Session which is used to perform the remove subscription request. 
* @param flags           \ref subscribeflags "Flags" to control the operation. Valid flags for this operation are:
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE
* @param topicSubscription_p Topic Subscription to remove as a NULL-terminated UTF-8 string.
* @param correlationTag  Correlation tag returned in the resulting Session event.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - The operation timed out trying to provision (only for blocking provision operations; refer to ::SOLCLIENT_PROVISION_FLAGS_WAITFORCONFIRM).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_QUEUE_NAME
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_CLIENT_NAME
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_endpointTopicUnsubscribe( solClient_propertyArray_pt endpointProps,
                                      solClient_opaqueSession_pt       opaqueSession_p,
                                      solClient_subscribeFlags_t       flags,
                                      const char                      *topicSubscription_p,
                                      void                            *correlationTag);

/**
* Creates a new Flow within a specified Session. Flow characteristics and behavior are defined by Flow properties. The Flow properties
* are supplied as an array of name/value pointer pairs, where the name and value are both strings.
* \ref flowProps "FLOW" and \ref endpointProps "ENDPOINT" configuration property names are processed; other property names
* are ignored. If the Flow creation specifies a non-durable endpoint, ENDPOINT properties can be used to change the default 
* properties on the non-durable endpoint. Any values not supplied are set to default values.
*
* When the Flow is created, an opaque Flow pointer is returned to the caller, and this value is then used for any
* Flow-level operations (for example, starting/stopping a Flow, getting statistics, sending an acknowledgment).
* The passed-in structure functInfo_p provides information on the message receive callback
* function and the Flow event function which the application has provided for this Flow.
* Both of these callbacks are mandatory. The message receive callback is invoked for each
* received message on this Flow. The Flow event callback is invoked when Flow events
* occur, such as the Flow going up or down. Both callbacks are invoked in the context
* of the Context thread to which the controlling Session belongs.
*
* Flow creation can be carried out in a blocking or
* non-blocking mode, depending upon the Flow property
* ::SOLCLIENT_FLOW_PROP_BIND_BLOCKING.
* In blocking mode, the calling thread is blocked until the Flow connection attempt either
* succeeds or is determined to have failed. If the connection succeeds, ::SOLCLIENT_OK is
* returned, and if the Flow could not be connected, ::SOLCLIENT_NOT_READY is returned.
* In non-blocking mode, ::SOLCLIENT_IN_PROGRESS is returned upon a successful Flow create
* request, and the connection attempt proceeds in the background.
* In both a non-blocking and blocking mode, a Flow event is generated for the Session:
* ::SOLCLIENT_FLOW_EVENT_UP_NOTICE, if the Flow was connected successfully; or
* ::SOLCLIENT_FLOW_EVENT_BIND_FAILED_ERROR, if the Flow failed to connect.
* For blocking mode, the Flow event is issued before the call to
* solClient_session_createFlow() returns. For non-blocking mode, the timing is undefined (that is,
* it could occur before or after the call returns, but it will typically be after).
* A Flow connection timer, controlled by the Flow property
* ::SOLCLIENT_SESSION_PROP_BIND_TIMEOUT_MS, controls the maximum amount of
* time a Flow connect attempt lasts for. Upon expiry of this time,
* a ::SOLCLIENT_FLOW_EVENT_BIND_FAILED_ERROR event is issued for the Session.
* If there is an error when solClient_session_createFlow() is invoked, then ::SOLCLIENT_FAIL
* is returned, and a Flow event is not subsequently issued. Thus, the caller must
* check for a return code of ::SOLCLIENT_FAIL if it has logic that depends upon a subsequent
* Flow event to be issued.
* For a non-blocking Flow create invocation, if the Flow create attempt eventually
* fails, the error information that indicates the reason for the failure cannot be
* determined by the calling thread. It must be discovered through the Flow event
* callback (and solClient_getLastErrorInfo can be called in the Flow event callback
* to get further information).
* For a blocking Flow create invocation, if the Flow create attempt does not
* return ::SOLCLIENT_OK, then the calling thread can determine the failure reason by immediately
* calling solClient_getLastErrorInfo. For a blocking Flow creation, ::SOLCLIENT_NOT_READY is returned
* if the created failed due to the bind timeout expiring (see ::SOLCLIENT_FLOW_PROP_BIND_TIMEOUT_MS).
* Note that the property values are stored internally in the API and the caller does not have to maintain
* the props array or the strings that are pointed to after this call completes. The API does not modify any of
* the strings pointed to by props when processing the property list.
*
* If the flow property SOLCLIENT_FLOW_PROP_BIND_ENTITY_ID is set to ::SOLCLIENT_FLOW_PROP_BIND_ENTITY_TE, 
* the flow Topic property ::SOLCLIENT_FLOW_PROP_TOPIC <b>must</b> be set, which will replace any existing
* topic on the topic-endpoint.
*
* <b>WARNING:</b> By default the ::SOLCLIENT_FLOW_PROP_ACKMODE is set to ::SOLCLIENT_FLOW_PROP_ACKMODE_AUTO,
* which automatically acknowledges all received messages. Function ::solClient_flow_sendAck returns SOLCLIENT_OK
* in the mode ::SOLCLIENT_FLOW_PROP_ACKMODE_AUTO, but with a warning that solClient_flow_sendAck
* is ignored as flow is in auto-ack mode.
*
* \n
*
* @param props           An array of name/value string pair pointers to configure Flow properties.
* @param opaqueSession_p The Session in which the Flow is to be created.
* @param opaqueFlow_p    The returned opaque Flow pointer that refers to the created Flow.
* @param funcInfo_p      A pointer to a structure that provides information on callback functions for events and received messages.
* @param funcInfoSize    The size of the passed-in funcInfo structure (in bytes) to allow the structure to grow in the future.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - The operation timed out trying to create a Flow (only for blocking Flow create operations; refer to ::SOLCLIENT_FLOW_PROP_BIND_BLOCKING).
* @li ::SOLCLIENT_SUBCODE_COMMUNICATION_ERROR - The underlying connection failed.
* @li ::SOLCLIENT_SUBCODE_OUT_OF_RESOURCES - The API was unable to allocate the memory necessary to create the Flow.
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - The Session does not support flow creation.
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_NAME_FOR_TE
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_QUEUE_NAME
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_TE_NAME
* @li ::SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_QUEUE
* @li ::SOLCLIENT_SUBCODE_MAX_CLIENTS_FOR_TE
* @li ::SOLCLIENT_SUBCODE_QUEUE_NOT_FOUND
* @li ::SOLCLIENT_SUBCODE_QUEUE_SHUTDOWN
* @li ::SOLCLIENT_SUBCODE_TE_SHUTDOWN
* @li ::SOLCLIENT_SUBCODE_NO_MORE_NON_DURABLE_QUEUE_OR_TE
* @li ::SOLCLIENT_SUBCODE_SUBSCRIPTION_ACL_DENIED
* @li ::SOLCLIENT_SUBCODE_REPLAY_NOT_SUPPORTED
* @li ::SOLCLIENT_SUBCODE_REPLAY_DISABLED
* @li ::SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_NON_EXCLUSIVE_NOT_ALLOWED
* @li ::SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_INACTIVE_FLOW_NOT_ALLOWED
* @li ::SOLCLIENT_SUBCODE_CLIENT_INITIATED_REPLAY_BROWSER_FLOW_NOT_ALLOWED 
* @li ::SOLCLIENT_SUBCODE_REPLAY_TEMPORARY_NOT_SUPPORTED
* @li ::SOLCLIENT_SUBCODE_UNKNOWN_START_LOCATION_TYPE
* @li ::SOLCLIENT_SUBCODE_REPLAY_START_TIME_NOT_AVAILABLE
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_session_createFlow (solClient_propertyArray_pt props,
                                  solClient_opaqueSession_pt opaqueSession_p,
                                  solClient_opaqueFlow_pt * opaqueFlow_p,
                                  solClient_flow_createFuncInfo_t *
                                  funcInfo_p, size_t funcInfoSize);

/**
* Destroys a previously created Flow. Upon return, the opaque Flow pointer
* is set to NULL.
* This operation <b>must not</b> be performed in a Flow callback
* for the Flow being destroyed.  
*
* @param opaqueFlow_p A pointer to the opaque Flow pointer that was returned when 
* the Session was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @li ::SOLCLIENT_SUBCODE_TIMEOUT - The operation timed-out trying to destroy the Flow.
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_destroy (solClient_opaqueFlow_pt * opaqueFlow_p);


/**
 * Returns the Session associated with the specified Flow. When flows are created
 * within a Session (see ::solClient_session_createFlow), then this routine can be used
 * if the application has a Flow pointer and needs to determine the associated
 * Session. For example, in a Flow event callback in which a Flow pointer is
 * provided, an application can use this routine to determine the associated Session. Note that from the Session the associated Context
 * can be determined through  ::solClient_session_getContext.
 * @param opaqueFlow_p An opaque Flow that was returned when the Flow was created.
 * @param opaqueSession_p An opaque Session associated with the Flow that is returned.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
 solClient_dllExport solClient_returnCode_t 
    solClient_flow_getSession(solClient_opaqueFlow_pt     opaqueFlow_p,
                              solClient_opaqueSession_pt *opaqueSession_p);


/**
 * Sends an acknowledgment on the specified Flow. This instructs the API to consider
 * the specified msgID acknowledged at the application layer. The library 
 * does not send acknowledgments immediately. It stores the state for 
 * acknowledged messages internally and acknowledges messages, in bulk, when a
 * threshold or timer is reached.
 *
 * Applications <b>must</b> only acknowledge a message on the Flow on which
 * it is received. Using the <i>msgId</i> received on one Flow when acknowledging
 * on another may result in no message being removed from the message-spool or the wrong
 * message being removed from the message-spool.
 * 
 * The exact behavior of solClient_flow_sendAck() is controlled by Flow property 
 * ::SOLCLIENT_FLOW_PROP_ACKMODE:
 * @li SOLCLIENT_FLOW_PROP_ACKMODE_AUTO - messages are acknowledged automatically by C API
 * and calling this function has no effect.
 * @li SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT - every message received must be acknowledged by the 
 * application through individual calls to solClient_flow_sendAck().
 *
 * <b>WARNING:</b> If ::SOLCLIENT_FLOW_PROP_ACKMODE is set to ::SOLCLIENT_FLOW_PROP_ACKMODE_AUTO 
 * (the default behavior), the function returns ::SOLCLIENT_OK, but with a warning that 
 * solClient_flow_sendAck is ignored as the flow is in auto-ack mode.
 * 
 * @param opaqueFlow_p    The opaque Flow that is returned when the Flow was created.
 * @param msgId           The 64-bit messageId for the acknowledged message.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
  solClient_dllExport solClient_returnCode_t
    solClient_flow_sendAck (solClient_opaqueFlow_pt opaqueFlow_p,
                            solClient_msgId_t msgId);

/**
* Closes the receiver on the specified Flow. This method will close the Flow 
* window to the appliance so further messages will not be received until 
* solClient_flow_start() is called. Messages in transit when this method is 
* called will still be delivered to the application. So the application must
* expect that the receive message callback can be called even after calling 
* solClient_flow_stop(). The maximum number of messages that may be 
* in transit at any one time is controlled by ::SOLCLIENT_FLOW_PROP_WINDOWSIZE 
* and ::SOLCLIENT_FLOW_PROP_MAX_UNACKED_MESSAGES (see solClient_flow_setMaxUnAcked()).
*
* A Flow can be created with the window closed by setting the Flow property ::SOLCLIENT_FLOW_PROP_START_STATE 
* to ::SOLCLIENT_PROP_DISABLE_VAL. When a Flow is created in this way, messages will not be received
* on the Flow until after ::solClient_flow_start() is called.
* 
*
* @param opaqueFlow_p    The opaque Flow returned when the Flow was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_stop (solClient_opaqueFlow_pt opaqueFlow_p);

/**
* Opens the receiver on the specified Flow. This method opens the Flow window
* to the appliance so further messages can be received. For browser flows (::SOLCLIENT_FLOW_PROP_BROWSER), applications have to call the function to get more messages.
*
* A Flow may be created with the window closed by setting the Flow property ::SOLCLIENT_FLOW_PROP_START_STATE 
* to ::SOLCLIENT_PROP_DISABLE_VAL. When a Flow is created in this way, messages will not be received
* on the Flow until after ::solClient_flow_start() is called.
* 
*
* @param opaqueFlow_p    The opaque Flow returned when the Flow was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_start (solClient_opaqueFlow_pt opaqueFlow_p);

/** 
 * Changes the maximum number of unacknowledged messages that may be received
 * on the specified Flow. This function may only be used when the Flow has been 
 * created with ::SOLCLIENT_FLOW_PROP_ACKMODE set to ::SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT.
 * When the maximum number of unacknowledged messages is reduced, messages in transit will still be received even if that causes
 * the application to exceed the new limit of maximum number of unacknowledged messages. 
 * The maximum number of messages that may be in transit at any one time is the lesser of 
 * the current allowable unacknowledged messages and the Flow property ::SOLCLIENT_FLOW_PROP_WINDOWSIZE.
 * The current allowable unacknowledged messages is simply the difference between the current
 * (that is, before calling this function) maximum number of acknowledged messages and the number of messages 
 * received that have not been acknowledged.
 *
 * @param opaqueFlow_p    The opaque Flow returned when the Flow was created.
 * @param maxUnacked      The new value for maximum number of acknowledged messages to allow 
 *                        on the Flow. If set to -1, there is no limit to the maximum number of 
 *                        acknowledged messages other than the appliance defined limit in the endpoint.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_INVALID_FLOW_OPERATION - Changing maximum unacknowledged messages is 
 *              only valid when the Flow acknowldgement mode (::SOLCLIENT_FLOW_PROP_ACKMODE) is set
 *              to ::SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT.
 * @see ::solClient_subCode for a description of all subcodes.
 */
  solClient_dllExport solClient_returnCode_t
    solClient_flow_setMaxUnacked (solClient_opaqueFlow_pt opaqueFlow_p,
                                  solClient_int32_t       maxUnacked);


/**
* Returns an array of Flow receive statistics.
* If the array is smaller than the number of defined receive statistics, only the first N
* defined statistics are returned.
* If the array is larger than the number of defined receive statistics, only the defined
* entries are filled in. The other entries are not touched.
* @param opaqueFlow_p The opaque Flow returned when the Flow was created.
* @param rxStats_p       A pointer to an array of statistic values of type solClient_Stats_t.
* @param arraySize       The number of entries in the array passed in (NOT the number of bytes).
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_getRxStats (solClient_opaqueFlow_pt opaqueFlow_p,
                               solClient_stats_pt rxStats_p,
                               solClient_uint32_t arraySize);

/**
* Returns an individual receive statistic.
* If multiple receive statistics are needed, it is more efficient to use
* solClient_flow_getRxStats rather than to call this routine multiple times for
* different statistics.
* @param opaqueFlow_p The opaque Flow returned when Flow was created.
* @param rxStatType      The type of receive statistic to return.
* @param rxStat_p        A pointer to a variable to hold the returned statistic.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @see solClient_flow_getRxStats()
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_getRxStat (solClient_opaqueFlow_pt opaqueFlow_p,
                              solClient_stats_rx_t rxStatType,
                              solClient_stats_pt rxStat_p);

/**
 * Retrieve the destination for the Flow. The destination returned can 
 * be used to set the ReplyTo field in a message, or otherwise communicated
 * to partners that need to send messages to this Flow. This is especially useful 
 * for temporary endpoints (Queues and Topic Endpoints), as the destination 
 * is unknown before the endpoint is created.
 * @param opaqueFlow_p The opaque Flow returned when the Flow was created.
 * @param dest_p       A pointer to a solClient_destination_t.
 * @param destSize     The size of (solClient_destination_t). This parameter is used for backwards binary compatibility if solClient_destination_t changes in the future.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
  solClient_dllExport solClient_returnCode_t 
    solClient_flow_getDestination(solClient_opaqueFlow_pt       opaqueFlow_p,
                                  solClient_destination_t      *dest_p,
                                  size_t                        destSize);

/**
* Clears all the statistics (rx and tx) for the specified Flow. All previous
* Flow statistics are lost when this is called.
* @param opaqueFlow_p The opaque Flow returned when Flow was created.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_clearStats (solClient_opaqueFlow_pt opaqueFlow_p);

/**
* Outputs a log at a specified log level that contains the value at this moment of all
* receive and transmit statistics for the specified Flow. This routine is useful for
* debugging of applications because it enables an application to easily output all available
* Flow statistics.
* @param opaqueFlow_p The opaque Flow returned when Flow was created.
* @param level The log level used to output the statistics log.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_logStats (solClient_opaqueFlow_pt opaqueFlow_p,
                             solClient_log_level_t level);


/**
* Allows topics to be dispatched to different message receive callbacks and with different
* dispatchUser_p for received messages on an endpoint Flow. If the endpoint supports adding topics
* (Queue endpoints), then this function will also add the Topic subscription to the endpoint unless 
* SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY is set. SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY is
* implied for all other endpoints.
*
* If the dispatch function info (funcinfo_p) is NULL, the Topic subscription is only added to the endpoint and
* no local dispatch entry is created. This operation is then identical to solClient_session_endpointTopicSubscribe().

* SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY can only be set when funcinfo_p 
* is not NULL. Consequently funcinfo_p may not be NULL for non-Queue endpoints. 
*
* The Session property ::SOLCLIENT_SESSION_PROP_TOPIC_DISPATCH must be enabled for a non-NULL funcinfo_p
* to be specified.
*
* When funcinfo_p is not NULL, the received messages on the Topic Endpoint Flow are further demultiplexed based on the received
* topic.
* 
* @see @ref subscription-syntax
* @see @ref topic-dispatch
* @see @ref flow-topic-dispatch
*
* @param opaqueFlow_p The opaque Flow that is returned when the Flow was created.
* @param flags \ref subscribeflags "Flags" to control the operation. Valid flags for this operation are:
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_RX_ALL_DELIVER_TO_ONE
* @li ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY
* @param topicSubscription_p The Topic subscription string (a NULL-terminated UTF-8 string).
* @param funcInfo_p         The message receive callback information. See structure solClient_flow_rxMsgDispatchFuncInfo.
* @param correlationTag     A correlationTag pointer that is returned in the confirm or fail sessionEvent for the
*                           subscription. This is only used if SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM is set and
*                           SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM is not set.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - Topic dispatch is not enabled for the Session .
* @li ::SOLCLIENT_SUBCODE_INVALID_FLOW_OPERATION - Topic dispatch is not supported for the Transacted Flow.
* @see ::solClient_subCode for a description of all subcodes.
*
* A successful call has a return code of ::SOLCLIENT_OK except when using ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM without 
* ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. In that case, the return code will be ::SOLCLIENT_IN_PROGRESS because the call returns without
* waiting for the operation to complete.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_topicSubscribeWithDispatch(
                         solClient_opaqueFlow_pt opaqueFlow_p,
                         solClient_subscribeFlags_t flags,
                         const char *topicSubscription_p,
                         solClient_flow_rxMsgDispatchFuncInfo_t *funcInfo_p,
                         void  *correlationTag);
                         

/**
* Removes a topic dispatch from an endpoint Flow. When the Flow is connected to a Queue endpoint, this function also removes the Topic 
* subscription from the Queue unless ::SOLCLIENT_SUBSCRIBE_FLAGS_LOCAL_DISPATCH_ONLY is set.
* 
* @see @ref subscription-syntax
* @see @ref topic-dispatch
* @see @ref flow-topic-dispatch
*
* @param opaqueFlow_p The opaque Flow returned when the Flow was created.
* @param flags \ref subscribeflags "Flags" to control the operation. Not currently used. Set to zero to ensure future compatibility.
* @param topicSubscription_p The Topic subscription string (a NULL-terminated UTF-8 string).
* @param funcInfo_p         The message receive callback information. See structure solClient_flow_rxMsgDispatchFuncInfo.
* @param correlationTag     A correlationTag pointer that is returned as is in the confirm or fail sessionEvent for the
*                           subscription. This is only used if SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM is set and
*                           SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM is not set.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL, ::SOLCLIENT_WOULD_BLOCK, ::SOLCLIENT_IN_PROGRESS
* @subcodes
* @li ::SOLCLIENT_SUBCODE_INVALID_TOPIC_SYNTAX
* @li ::SOLCLIENT_SUBCODE_INVALID_SESSION_OPERATION - Topic dispatch is not enabled for the Session
* @li ::SOLCLIENT_SUBCODE_INVALID_FLOW_OPERATION - Topic dispatch is not supported for the Transacted Flow.
* @see ::solClient_subCode for a description of all subcodes.
*
* A successful call has a return code of ::SOLCLIENT_OK except when using ::SOLCLIENT_SUBSCRIBE_FLAGS_REQUEST_CONFIRM without 
* ::SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM. In that case, the return code will be ::SOLCLIENT_IN_PROGRESS because the call returns without
* waiting for the operation to complete.
*/
  solClient_dllExport solClient_returnCode_t
    solClient_flow_topicUnsubscribeWithDispatch(
                         solClient_opaqueFlow_pt opaqueFlow_p,
                         solClient_subscribeFlags_t flags,
                         const char *topicSubscription_p,
                         solClient_flow_rxMsgDispatchFuncInfo_t *funcInfo_p,
                         void  *correlationTag);

/**
 * Gets the value of the specified Flow property for the Flow. The 
 * property value is copied out to buffer provided by the caller. The returned
 * value is a NULL-terminated UTF-8 string.  
 *
 * @param opaqueFlow_p    The opaque Flow returned when the Flow was created.
 * @param propertyName_p  The name of the Flow property for which the value is to be returned.
 * @param buf_p           A pointer to the buffer provided by the caller in which to place the NULL-terminated property value string.
 * @param bufSize         The size of the buffer provided by the caller (in bytes).
 * @returns               ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_flow_getProperty( 
    solClient_opaqueFlow_pt opaqueFlow_p,
    const char                 *propertyName_p,
    char                       *buf_p,
    size_t                      bufSize
);

/**
* Outputs a log at the specified log level that contains the Flow state information
* for the given Flow.
* @param opaqueFlow_p The opaque Flow returned when Flow was created.
* @param level The log level used to output the statistics log.
* @return ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
* @subcodes
* @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_flow_logFlowInfo(solClient_opaqueFlow_pt opaqueFlow_p,
                             solClient_log_level_t level );

/**
* Returns a string representation of the Flow event passed in.
* @param flowEvent The Flow event to convert to a string representation.
* @return A pointer to a constant character string. This pointer is never NULL.
*/
  solClient_dllExport const char
    *solClient_flow_eventToString (solClient_flow_event_t flowEvent);                                            

/**
 * Creates a Transacted Session object that is used in subsequent transactions.
 * It is recommended that a client application use a default Context-bound Message Dispatcher
 * for its asynchronous message delivery within a Transacted Session.
 *
 * By enabling Transacted Session property ::SOLCLIENT_TRANSACTEDSESSION_PROP_CREATE_MESSAGE_DISPATCHER,
 * a TransactedSession-bound Message Dispatcher is created as well when a Transacted Session
 * is created. The lifecycle of a Message Dispatcher is bounded to the lifecycle of the
 * corresponding API object.
 * @see @ref transacted-session
 * 
* @param props     An array of name/value string pair pointers to configure Transacted Session  properties.
 * @param session_p           Session in which a Transacted Session is to be created.
 * @param transactedSession_p  Pointer to the location which contains the opaque Transacted Session pointer on return.
 * @param rfu_p               Reserved for future use; must be NULL.
 * @returns                     ::SOLCLIENT_OK on success. ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_READY
 * @subcodes
 * @see ::solClient_subCode for a description of subcodes.
 */
solClient_dllExport solClient_returnCode_t
   solClient_session_createTransactedSession(solClient_propertyArray_pt            props,
                                             solClient_opaqueSession_pt            session_p,
                                             solClient_opaqueTransactedSession_pt  *transactedSession_p,
                                             void                                  *rfu_p);

/**
 * Destroys a previous created Transacted Session.
 * This operation must not be performed from a callback for associated with the Transacted Session
 * being destroyed. This includes all Flows on the Transacted Session.
  * @see @ref transacted-session
 *
 * @param transactedSession_p  Pointer to opaque Transacted Session pointer that was returned
 *                              when the Transacted Session was created.
 * @returns ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of subcodes.
 */
solClient_dllExport solClient_returnCode_t
  solClient_transactedSession_destroy(solClient_opaqueTransactedSession_pt   *transactedSession_p);


/**
 * Rollback the active transaction of a specified Transacted Session.
  * @see @ref transacted-session
 *
 * @param transactedSession_p  Pointer to opaque Transacted Session pointer that was returned when 
 * the Transacted Session was created.
 * @returns ::SOLCLIENT_OK on success, ::SOLCLIENT_NOT_READY if underlying session is not active, 
 * ::SOLCLIENT_FAIL if the Transacted Session is no longer active.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_INVALID_TRANSACTED_SESSION_ID
 * @li ::SOLCLIENT_SUBCODE_INVALID_TRANSACTION_ID
 * @li ::SOLCLIENT_SUBCODE_COMMIT_OR_ROLLBACK_IN_PROGRESS
 */
solClient_dllExport solClient_returnCode_t
   solClient_transactedSession_rollback(solClient_opaqueTransactedSession_pt transactedSession_p);


/**
 * Commit the active transaction of a specified Transacted Session.
 * @see @ref transacted-session
 *
 * @param transactedSession_p  Pointer to opaque Transacted Session pointer that was returned when 
 * the Transacted Session was created.
 * @returns
 * @li ::SOLCLIENT_OK on success
 * @li ::SOLCLIENT_NOT_READY if underlying session is not active
 * @li ::SOLCLIENT_ROLLBACK if a ROLLBACK response is received and the Transacted Session is still active
 * @li ::SOLCLIENT_FAIL
 *
 * The following subcodes all apply when ::SOLCLIENT_ROLLBACK is returned.
 * @li ::SOLCLIENT_SUBCODE_INVALID_TRANSACTED_SESSION_ID
 * @li ::SOLCLIENT_SUBCODE_INVALID_TRANSACTION_ID
 * @li ::SOLCLIENT_SUBCODE_NO_TRANSACTION_STARTED
 * @li ::SOLCLIENT_SUBCODE_MESSAGE_CONSUME_FAILURE
 * @li ::SOLCLIENT_SUBCODE_MESSAGE_PUBLISH_FAILURE
 * @li ::SOLCLIENT_SUBCODE_QUEUE_SHUTDOWN
 * @li ::SOLCLIENT_SUBCODE_TE_SHUTDOWN
 * @li ::SOLCLIENT_SUBCODE_SPOOL_OVER_QUOTA
 * @li ::SOLCLIENT_SUBCODE_QUEUE_NOT_FOUND
 * @li ::SOLCLIENT_SUBCODE_TRANSACTION_FAILURE
 * @li ::SOLCLIENT_SUBCODE_ENDPOINT_MODIFIED
 * @li ::SOLCLIENT_SUBCODE_INVALID_CONNECTION_OWNER
 * @li ::SOLCLIENT_SUBCODE_COMMIT_OR_ROLLBACK_IN_PROGRESS
 * @li ::SOLCLIENT_SUBCODE_NO_SUBSCRIPTION_MATCH
 * @li ::SOLCLIENT_SUBCODE_CONTROL_OTHER
 *
 * When solClient_transactedSession_commit() returns ::SOLCLIENT_FAIL, the status of the outstanding commit request cannot be determined. The commit may have succeeded on the message-router before the response was lost. Or the commit request may have failed (roll back).  Applications that treat the failure as a rollback must allow for duplicate transactions. The following subcodes may occur:
 * @li ::SOLCLIENT_SUBCODE_COMMIT_STATUS_UNKNOWN - This can occur when a commit request is sent and before the response is received the connection failed and was
 * automatically re-established to a standby message router. The API establishes a new transacted session and processing may continue but the status of the last 
 * commit request is unknown.

 * @see ::solClient_subCode for a description of subcodes.
 */
solClient_dllExport solClient_returnCode_t
   solClient_transactedSession_commit(solClient_opaqueTransactedSession_pt transactedSession_p);

/**
* Sends an Guaranteed Delivery  message on the specified Transacted Session.
* The message is composed of a number of
* optional components that are specified by the msg_p. The application should first allocate a
* solClient_msg, then use the methods defined in solClientMsg.h to build the message to send. \n
* solClient_transactedSession_sendMsg() returns ::SOLCLIENT_OK when the message has been successfully
* copied to the transmit buffer or underlying transport. 
* A successful commit acknowledges published messages delivered to the Solace messaging appliance.
 * @see @ref transacted-session
 *
* @param transactedSession_p The opaque Transacted Session returned when the Transacted Session was created.
* @param msg_p           The opaque message created by solClient_msg_alloc.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, or ::SOLCLIENT_FAIL. 
* @subcodes
* @li ::SOLCLIENT_SUBCODE_DELIVERY_MODE_UNSUPPORTED
* @li ::SOLCLIENT_SUBCODE_COMMIT_OR_ROLLBACK_IN_PROGRESS
*/
solClient_dllExport solClient_returnCode_t
   solClient_transactedSession_sendMsg(solClient_opaqueTransactedSession_pt   transactedSession_p,
                                       solClient_opaqueMsg_pt                         msg_p);

/**
 * Create a transacted consumer Flow within a specified Transacted Session.
 * If a Rx Message Callback is provided, messages received on the flow are dispatched
 * to the callback with its usual user_p in a context of a Message Dispatcher 
 * thread. If there is no Rx Message Callback (i.e.
 * funcInfo_p-> rxMsgInfo.callback_p=NULL), messages received on the Flow are queued internally.
 * Applications must call ::solClient_flow_receiveMsg() directly to retrieve the queued messages.
 *
 * Unlike non-transacted Flows, events and messages received on a transacted Flow are delivered
 * in the context of two different threads. Events are still delivered in a context of
 * the context thread, but messages are dispatched to the Flow Rx message
 * callback with their usual user_p in the context of a Message Dispatcher thread or are retrieved by
 * calling solClient_flow_receiveMsg() from the context of an application thread.
 * @see @ref transacted-session
 *
 * The following flow properties are not supported by transacted Flows:
 *  @li ::SOLCLIENT_FLOW_PROP_ACKMODE
 *  @li ::SOLCLIENT_FLOW_PROP_AUTOACK
 *  @li ::SOLCLIENT_FLOW_PROP_BROWSER
 *  @li ::SOLCLIENT_FLOW_PROP_FORWARDING_MODE
 *  @li ::SOLCLIENT_FLOW_PROP_MAX_UNACKED_MESSAGES
 *
 * @param props           An array of name/value string pair pointers to configure Flow properties.
 * @param transactedSession_p The Transacted Session in which the transacted Flow is to be created.
 * @param flow_p    The returned opaque Flow pointer that refers to the created Flow.
 * @param funcInfo_p      A pointer to a structure that provides information on callback functions for events and received messages.
 * @param funcInfoSize    The size of the passed-in funcInfo structure (in bytes) to allow the structure to grow in the future.
 * @return ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_READY
 *  @subcodes
 *     SOLCLIENT_SUBCODE_PROP_PARAM_CONFLICT
 * @see ::solClient_subCode for a description of subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_transactedSession_createFlow (solClient_propertyArray_pt props,
                                        solClient_opaqueTransactedSession_pt transactedSession_p,
                                        solClient_opaqueFlow_pt * flow_p,
                                        solClient_flow_createFuncInfo_t *funcInfo_p,
                                        size_t funcInfoSize);

/** Retrieve Transacted Session name
 * @see @ref transacted-session
 *
 * @param transactedSession_p  Opaque Transacted Session pointer that was returned when the 
 * Transacted Session was created.
 * @param nameBuf_p   A pointer to the buffer provided by the caller in which to place the NULL-terminated transacted session name string. The maximum transacted session name length is ::SOLCLIENT_TRANSACTEDSESSION_MAX_SESSION_NAME_LENGTH.
 * @param bufSize    The size (in bytes) of the buffer provided by the caller. 
 * @returns ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of subcodes.
 */
#define SOLCLIENT_TRANSACTEDSESSION_MAX_SESSION_NAME_LENGTH 64 /**< The maximum transacted session name length. */

solClient_dllExport solClient_returnCode_t
solClient_transactedSession_getSessionName(
    solClient_opaqueTransactedSession_pt  transactedSession_p,
    char                                  *nameBuf_p,       
    size_t                                bufSize);

/**
 * Receives a message from a specified Transacted Flow.
 * It waits until timeout or a message received, no wait if timeout=0.
 * Applications must later call solClient_msg_free() to release the received message.
 * @see @ref transacted-session
 *
 * @param flow_p     Opaque flow pointer.
 * @param msg_p      Pointer to the location to contain the opaque message pointer.  If there is no message, the opaque message pointer is set to NULL.
 * @param timeout    timeout in milliseconds, 0 means no wait.
 * @returns
 *     @li  ::SOLCLIENT_OK with a valid opaque message pointer if queue is not empty,
 *     @li  ::SOLCLIENT_OK with the returned opaque message pointer set to NULL (*msg_p= NULL) when timeout or
 *              queue is empty and timeout=0. 
 *     @li   ::SOLCLIENT_FAIL with subcode ::SOLCLIENT_SUBCODE_INVALID_FLOW_OPERATION if the flow is not a Transacted Flow or ::SOLCLIENT_SUBCODE_FLOW_UNBOUND if the flow is unbound and there are no buffered messages within the API waiting for delivery.
 *     @li   ::SOLCLIENT_NOT_READY with subcode ::SOLCLIENT_SUBCODE_SESSION_NOT_ESTABLISHED or ::SOLCLIENT_SUBCODE_NO_TRANSACTION_STARTED.
 * @subcodes
 *     @li ::SOLCLIENT_SUBCODE_FLOW_UNBOUND
 *     @li ::SOLCLIENT_SUBCODE_INVALID_FLOW_OPERATION 
 *     @li ::SOLCLIENT_SUBCODE_NO_TRANSACTION_STARTED
 *     @li ::SOLCLIENT_SUBCODE_COMMIT_OR_ROLLBACK_IN_PROGRESS
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
  solClient_flow_receiveMsg(solClient_opaqueFlow_pt    flow_p,
                            solClient_opaqueMsg_pt     *msg_p,
                            solClient_int32_t          timeout);

/**
 * Returns the Transacted Session pointer with the specified Flow.
 * When flows are created within a Transacted Session (see ::solClient_transactedSession_createFlow), then this routine can be used if the application has
 * a Flow pointer and needs to determine the associated Transacted Session. For example, in a Flow event callback in which a Flow pointer is provided,
 * an application can use this routine to determine the associated Transacted Session. 
 * @see @ref transacted-session
 *
 * @param flow_p     Opaque flow pointer.
 * @param transactedSession_p  Pointer to the location which contains the opaque Transacted Session pointer on return.
* @return ::SOLCLIENT_OK, ::SOLCLIENT_NOT_READY, ::SOLCLIENT_FAIL
*/
solClient_dllExport solClient_returnCode_t
solClient_flow_getTransactedSession(solClient_opaqueFlow_pt               flow_p,
                                    solClient_opaqueTransactedSession_pt  *transactedSession_p);

#ifndef SOLCLIENT_EXCLUDE_DEPRECATED
#include "solClientDeprecated.h"
#endif /* SOLCLIENT_EXCLUDE_DEPRECATED */

/** @example Intro/HelloWorldPub.c */
/** @example Intro/HelloWorldSub.c */
/** @example Intro/HelloWorldQueuePub.c */
/** @example Intro/HelloWorldQueueSub.c */
/** @example Intro/HelloWorldWebPub.c */
/** @example Intro/HelloWorldWebSub.c */
/** @example ex/directPubSub.c */
/** @example ex/dtoPubSub.c  */
/** @example ex/queueProvision.c  */
/** @example ex/perfTest.c      */
/** @example ex/perfADSub.c     */
/** @example ex/perfADPub.c     */
/** @example ex/common.h        */
/** @example ex/os.h            */
/** @example ex/common.c        */
/** @example ex/sempGetOverMb.c */
/** @example ex/messageSelectorsOnQueue.c   */
/** @example ex/asyncCacheRequest.c         */
/** @example ex/subscribeOnBehalfOfClient.c */
/** @example ex/syncCacheRequest.c          */
/** @example ex/topicToQueueMapping.c       */
/** @example ex/topicDispatch.c             */
/** @example ex/messageTTLAndDeadMessageQueue.c     */
/** @example ex/messageReplay.c */
/** @example ex/noLocalPubSub.c */
/** @example ex/flowControlQueue.c          */
/** @example ex/eventMonitor.c  */
/** @example ex/adPubAck.c      */
/** @example ex/replication.c      */
/** @example ex/simpleBrowserFlow.c         */
/** @example ex/cutThroughFlowToQueue.c     */
/** @example ex/simpleFlowToQueue.c         */
/** @example ex/simpleFlowToTopic.c         */
/** @example ex/redirectLogs.c              */
/** @example ex/sdtPubSubMsgDep.c           */
/** @example ex/sdtPubSubMsgIndep.c         */
/** @example ex/secureSession.c          */
/** @example ex/activeFlowIndication.c     */
/** @example ex/RRcommon.h        */
/** @example ex/RRDirectReplier.c  */
/** @example ex/RRDirectRequester.c  */
/** @example ex/RRGuaranteedReplier.c  */
/** @example ex/RRGuaranteedRequester.c  */
/** @example ex/transactions.c  */
/** @example ex/ios/examples/AppReachabilityExample.m */
/** @example ex/ios/examples/AppTransitionsExample.m */
/** @example ex/ios/examples/AsyncCacheRequestExample.m */
/** @example ex/ios/examples/DirectPubSubExample.m */
/** @example ex/ios/examples/DtoPubSubExample.m */
/** @example ex/ios/examples/EventMonitorExample.m */
/** @example ex/ios/examples/MessageReplayExample.m */
/** @example ex/ios/examples/PerfTestExample.m */
/** @example ex/ios/examples/RedirectLogsExample.m */
/** @example ex/ios/examples/SdtPubSubMsgDepExample.m */
/** @example ex/ios/examples/SdtPubSubMsgIndepExample.m */
/** @example ex/ios/examples/SecureSessionExample.m */
/** @example ex/ios/examples/SempGetOverMbExample.m */
/** @example ex/ios/examples/SubscribeOnBehalfOfClientExample.m */
/** @example ex/ios/examples/SyncCacheRequestExample.m */
/** @example ex/ios/examples/TopicDispatchExample.m */
/** @example ex/ios/intro/HelloWorldPubExample.m */
/** @example ex/ios/intro/HelloWorldSubExample.m */
/** @example ex/ios/intro/HelloWorldWebPubExample.m */
/** @example ex/ios/intro/HelloWorldWebSubExample.m */
/** @example ex/ios/examples/AppReachabilityExample.h */
/** @example ex/ios/examples/AppTransitionsExample.h */
/** @example ex/ios/examples/AsyncCacheRequestExample.h */
/** @example ex/ios/examples/DirectPubSubExample.h */
/** @example ex/ios/examples/DtoPubSubExample.h */
/** @example ex/ios/examples/EventMonitorExample.h */
/** @example ex/ios/examples/MessageReplayExample.h */
/** @example ex/ios/examples/PerfTestExample.h */
/** @example ex/ios/examples/RedirectLogsExample.h */
/** @example ex/ios/examples/SdtPubSubMsgDepExample.h */
/** @example ex/ios/examples/SdtPubSubMsgIndepExample.h */
/** @example ex/ios/examples/SecureSessionExample.h */
/** @example ex/ios/examples/SempGetOverMbExample.h */
/** @example ex/ios/examples/SubscribeOnBehalfOfClientExample.h */
/** @example ex/ios/examples/SyncCacheRequestExample.h */
/** @example ex/ios/examples/TopicDispatchExample.h */
/** @example ex/ios/intro/HelloWorldPubExample.h */
/** @example ex/ios/intro/HelloWorldSubExample.h */
/** @example ex/ios/intro/HelloWorldWebPubExample.h */
/** @example ex/ios/intro/HelloWorldWebSubExample.h */						
/** @example ex/ios/AppDelegate.h */
/** @example ex/ios/AppDelegate.m */
/** @example ex/ios/EventCallbackProtocol.h*/
/** @example ex/ios/Example.h */
/** @example ex/ios/Example.m */
/** @example ex/ios/ExampleInterface.h */
/** @example ex/ios/ExampleInterface.m */
/** @example ex/ios/LogCallbackProtocol.h */
/** @example ex/ios/MessageReceiveCallbackProtocol.h */
/** @example ex/ios/Parameter.h */
/** @example ex/ios/Parameter.m */
/** @example ex/ios/ParameterInterface.h */
/** @example ex/ios/ParameterInterface.m */
/** @example ex/ios/main.m */
#if defined(__cplusplus)
}
#endif                          /* _cplusplus */

#endif                          /* SOL_CLIENT_H_ */
