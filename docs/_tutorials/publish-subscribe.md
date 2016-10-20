---
layout: tutorials
title: Publish/Subscribe
summary: Learn how to set up pub/sub on a Solace VMR.
icon: publish-subscribe.png
---

This tutorial will introduce you to the fundamentals of the Solace API by connecting a client, adding a topic subscription and sending a message matching this topic subscription. This forms the basis for any publish / subscribe message exchange illustrated here:

![]({{ site.baseurl }}/images/publish-subscribe.png)

## Assumptions

This tutorial assumes the following:

*   You are familiar with Solace [core concepts]({{ site.docs-core-concepts}}){:target="_top"}.
*   You have access to a running Solace message router with the following configuration:
    *   Enabled message VPN
    *   Enabled client username

One simple way to get access to a Solace message router is to start a Solace VMR load [as outlined here]({{ site.docs-vmr-setup }}){:target="_top"}.

## Goals

The goal of this tutorial is to demonstrate the most basic messaging interaction using Solace. This tutorial will show you:

1.  How to build and send a message on a topic
2.  How to subscribe to a topic and receive a message

## Solace message router properties

In order to send or receive messages to a Solace message router, you need to know a few details of how to connect to the Solace message router. Specifically you need to know the following:

<table>
<tr>
<td>Resource</td>
<td>Value</td>
<td>Description</td>
</tr>
<tr>
<td>Host</td>
<td>String of the form <code>DNS name</code> or <code>IP:Port</code></td>
<td>This is the address clients use when connecting to the Solace message router to send and receive messages.  
For a Solace VMR this there is only a single interface so the IP is the same as the management IP address.  
For Solace message router appliances this is the host address of the message-backbone.</td>
</tr>
<tr>
<td>Message VPN</td>
<td>String</td>
<td>The Solace message router Message VPN that this client should connect to. The simplest option is to use the "default" message-vpn which is present on all Solace message routers and fully enabled for message traffic on Solace VMRs.</td>
</tr>
<tr>
<td>Client Username</td>
<td>String</td>
<td>The client username. For the Solace VMR default message VPN, authentication is disabled by default, so this can be any value.</td>
</tr>
<tr>
<td>Client Password</td>
<td>String</td>
<td>The optional client password. For the Solace VMR default message VPN, authentication is disabled by default, so this can be any value or omitted.</td>
</tr>
</table>

For the purposes of this tutorial, you will connect to the default message VPN of a Solace VMR. So the only required information to proceed is the Solace VMR host string which this tutorial accepts as an argument.

## Obtaining the Solace API

This tutorial depends on you having the C API downloaded and available. The C API library can be [downloaded here]({{ site.links-downloads }}){:target="_top"}. The C API is distributed as a gzipped tar file for the platform you're working with. The API developer documentation is downloaded a separate tar file. The instructions in this tutorial assume you have downloaded the C API library, unpacked it to a known location and are executing the provided build instructions from a sub-directory within the C API library root. If your environment differs then adjust the build instructions appropriately.

## Connecting to the Solace message router

In order to send or receive messages, an application must connect a Solace session. The Solace session is the basis for all client communication with the Solace message router.

In the Solace messaging API for C (SolClient), a few distinct steps are required to create and connect a Solace session.

*   The API must be initialized
*   Appropriate asynchronous callbacks must be declared
*   A SolClient Context is needed to control application threading
*   The SolClient session must be created

### Initializing the CCSMP API

To initialize the SolClient API, you call the initialize method with arguments that control logging.

```cpp
/* solClient needs to be initialized before any other API calls. */
solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL );
```

This call must be made prior to making any other calls to the SolClient API. It allows the API to initialize internal state and buffer pools.

### SolClient Asynchronous Callbacks

The SolClient API is predominantly an asynchronous API designed for the highest speed and lowest latency. As such most events and notifications occur through callbacks. In order to get up and running, the following basic callbacks are required at a minimum.

```cpp
static int msgCount = 0;

solClient_rxMsgCallback_returnCode_t
messageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    printf ( "Received message:\n" );
    solClient_msg_dump ( msg_p, NULL, 0 );
    printf ( "\n" );

    msgCount++;
    return SOLCLIENT_CALLBACK_OK;
}

void
eventCallback ( solClient_opaqueSession_pt opaqueSession_p,
                solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{  
    printf("Session EventCallback() called:  %s\n", solClient_session_eventToString ( eventInfo_p->sessionEvent));
}
```

The messageReceiveCallback is invoked for each Direct message received by the Session. In this sample, the message is printed to the screen.

The eventCallback is invoked for various significant session events like connection, disconnection, and other SolClient session events. In this sample, simply prints the events. See the [SolClient API documentation]({{ site.docs-api-ref }}){:target="_top"} and samples for details on the session events.

### Context Creation

As outlined in the core concepts, the context object is used to control threading that drives network I/O and message delivery and acts as containers for sessions. The easiest way to create a context is to use the context initializer with default thread creation.

```cpp
/* Context */
solClient_opaqueContext_pt context_p;
solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;

solClient_context_create ( SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                           &context_p, &contextFuncInfo, sizeof ( contextFuncInfo ) );
```

### Session Creation

Finally a session is needed to actually connect to the Solace message router. This is accomplished by creating a properties array and connecting the session.

```cpp
/* Session */
solClient_opaqueSession_pt session_p;
solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

/* Session Properties */
const char     *sessionProps[20];
int             propIndex = 0;

/* Configure the Session function information. */
sessionFuncInfo.rxMsgInfo.callback_p = messageReceiveCallback;
sessionFuncInfo.rxMsgInfo.user_p = NULL;
sessionFuncInfo.eventInfo.callback_p = eventCallback;
sessionFuncInfo.eventInfo.user_p = NULL;

/* Configure the Session properties. */
propIndex = 0;
sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
sessionProps[propIndex++] = argv[1];
sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
sessionProps[propIndex++] = "default";
sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
sessionProps[propIndex++] = "helloWorldTutorial";
sessionProps[propIndex] = NULL;

/* Create the Session. */
solClient_session_create ( ( char ** ) sessionProps,
                           context_p,
                           &session_p, &sessionFuncInfo, sizeof ( sessionFuncInfo ) );

/* Connect the Session. */
solClient_session_connect ( session_p );
printf ( "Connected.\n" );
```

When creating the session, the factory method takes the session properties, the session pointer and information about the session callback functions. The API then creates the session within the supplied context and returns a reference in the session pointer. The final call to solClient_session_connect establishes the connection to the Solace message router which makes the session ready for use.

At this point your client is connected to the Solace message router. You can use SolAdmin to view the client connection and related details.

## Receiving a message

This tutorial is uses "Direct" messages which are at most once delivery messages. So first, let's express interest in the messages by subscribing to a Solace topic. Then you can look at publishing a matching message and see it received.  

![]({{ site.baseurl }}/images/pub-sub-receiving-message-300x134.png)

With a session connected in the previous step, then you must subscribe to a topic in order to express interest in receiving messages. This tutorial uses the topics "tutorial/topic".

```cpp
solClient_session_topicSubscribeExt ( session_p,
                                    SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                      "tutorial/topic");
```

Then after the subscription is added, the consumer is started. At this point the consumer is ready to receive messages. So wait in a loop for the expected message to arrive.

```cpp
printf ( "Waiting for message......\n" );
fflush ( stdout );
while ( msgCount < 1 ) {
    sleepInSec ( 1 );
}
```

## Sending a message

Now it is time to send a message to the waiting consumer.  

![]({{ site.baseurl }}/images/pub-sub-sending-message-300x134.png)

To send a message, you must create a message and a topic destination. This tutorial will send a Solace binary message with contents "Hello world!". Then you must send the message to the Solace message router.

```cpp
/* Message */
solClient_opaqueMsg_pt msg_p = NULL;ss
solClient_destination_t destination;
const char *text_p = "Hello world!";

/* Allocate memory for the message that is to be sent. */
solClient_msg_alloc ( &msg_p );

/* Set the message delivery mode. */
solClient_msg_setDeliveryMode ( msg_p, SOLCLIENT_DELIVERY_MODE_DIRECT );

/* Set the destination. */
destination.destType = SOLCLIENT_TOPIC_DESTINATION;
destination.dest = "tutorial/topic";
solClient_msg_setDestination ( msg_p, &destination, sizeof ( destination ) );

/* Add some content to the message. */
solClient_msg_setBinaryAttachment ( msg_p, text_p, ( solClient_uint32_t ) strlen ( (char *)text_p ) );

/* Send the message. */
printf ( "About to send message '%s' to topic '%s'...\n", (char *)text_p, argv[4] );
solClient_session_sendMsg ( session_p, msg_p );

printf ( "Message sent. Exiting.\n" );
solClient_msg_free ( &msg_p );
```

In the SolClient API, messages are allocated and freed from an internal API message pool for greatest performance and efficiency. Therefore as shown, messages must be acquired by calls to solClient_msg_alloc and then later freed back to the pool by calls to solClient_msg_free.

The minimum properties required to create a SolClient message that can be sent is to set the delivery mode, queue or topic destination, and message contents as shown in the above code. Once the message is created it is sent to the Solace message router with a call to solClient_session_sendMsg.

At this point the producer has sent a message to the Solace message router and your waiting consumer will have received the message and printed its contents to the screen.

## Summarizing

Combining the example source code show above results in the following source code files:

*   [HelloWorldPub.c]({{ site.repository }}/blob/master/src/HelloWorldPubSub/HelloWorldPub.c){:target="_blank"}
*   [HelloWorldSub.c]({{ site.repository }}/blob/master/src/HelloWorldPubSub/HelloWorldSub.c){:target="_blank"}
*   [os.c]({{ site.repository }}/blob/master/src/HelloWorldPubSub/os.c){:target="_blank"}
*   [os.h]({{ site.repository }}/blob/master/src/HelloWorldPubSub/os.h){:target="_blank"}

The OS source code simply provides platform abstraction. The subscriber sample makes use of this for the sleep in the main loop.

### Building

Building these examples is simple. The following provides an example using Linux. For ideas on how to build on other platforms you can consult the README of the C API library.

```
gcc -g -Wall -I ../include -L ../lib -lsolclient HelloWorldPub.c -o HelloWorldPub
gcc -g -Wall -I ../include -L ../lib -lsolclient os.c HelloWorldSub.c -o HelloWorldSub
```

Referencing the downloaded SolClient library include and lib file is required. For more advanced build control, consider adapting the makefile found in the "Intro" directory of the SolClient package. The above samples very closely mirror the samples found there.

If you start the HelloWoldSub with a single argument for the Solace message router host address it will connect and wait for a message.

```
$ LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH ./HelloWorldSub <<HOST_ADDRESS>>
HelloWorldSub initializing...
Session EventCallback() called:  Session up
Connected. Awaiting message...
```

Then you can send a message using the HelloWorldPub again using a single argument to specify the Solace message router host address. If successful, the output for the producer will look like the following:

```
$ LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH ./HelloWorldPub <<HOST_ADDRESS>>
HelloWorldPub initializing...
Session EventCallback() called:  Session up
Connected. About to send message 'Hello world!' to topic 'tutorial/topic'...
Message sent. Exiting.
```

With the message delivered the subscriber output will look like the following:

```
Received message:
Destination:         Topic 'tutorial/topic'
Class Of Service:    COS_1
DeliveryMode:        DIRECT
Binary Attachment:   len=12
48 65 6c 6c 6f 20 77 6f 72 6c 64 21           Hello world!
Exiting.
```

The received message is printed to the screen. The message contents was "Hello world!" as expected and shown in the contents of the message dump along with additional information about the Solace message that was received.

If you have any issues sending and receiving a message, check the [Solace community]({{ site.links-community }}){:target="_top"} for answers to common issues seen.

You have now successfully connected a client, subscribed to a topic and exchanged messages using this topic.