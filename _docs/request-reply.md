---
layout: tutorials
title: Request/Reply
summary: Learn how to set up request/reply messaging.
icon: I_dev_R+R.svg
links:
    - label: BasicRequestor.c
      link: /blob/master/src/intro/BasicRequestor.c
    - label: BasicReplier.c
      link: /blob/master/src/intro/BasicReplier.c
---


This tutorial outlines both roles in the request-response message exchange pattern. It will show you how to act as the client by creating a request, sending it and waiting for the response. It will also show you how to act as the server by receiving incoming requests, creating a reply and sending it back to the client. It builds on the basic concepts introduced in [publish/subscribe tutorial]({{ site.baseurl }}/publish-subscribe).

## Assumptions

This tutorial assumes the following:

*   You are familiar with Solace PubSub+ [core concepts]({{ site.docs-core-concepts }}){:target="_top"}.
*   You have access to PubSub+ messaging with the following configuration details:
    *   Connectivity information for a PubSub+ message-VPN
    *   Enabled client username and password

One simple way to get access to Solace messaging quickly is to create a messaging service in Solace PubSub+ Cloud [as outlined here]({{ site.links-solaceCloud-setup}}){:target="_top"}. You can find other ways to get access to PubSub+ messaging below.

The build instructions in this tutorial assume you are using a Linux shell. If your environment differs, adapt the instructions.

## Goals

The goal of this tutorial is to understand the following:

*   On the requestor side:
    1.  How to create a request
    2.  How to receive a response
    3.  How to use the Solace PubSub+ API to correlate the request and response
*   On the replier side:
    1.  How to detect a request expecting a reply
    2.  How to generate a reply message

## Overview

Request-reply messaging is supported by the PubSub+ message router for all delivery modes. For direct messaging, the Solace PubSub+ APIs provide the Requestor object for convenience. This object makes it easy to send a request and wait for the reply message. It is a convenience object that makes use of the API provided “inbox” topic that is automatically created for each PubSub+ client and automatically correlates requests with replies using the message correlation ID. (See Message Correlation below for more details). On the reply side another convenience method enables applications to easily send replies for specific requests. Direct messaging request reply is the delivery mode that is illustrated in this sample.

It is also possible to use guaranteed messaging for request reply scenarios. In this case the replier can listen on a queue for incoming requests and the requestor can use a temporary endpoint to attract replies. The requestor and replier must manually correlate the messages. This is explained further in the [Solace PubSub+ documentation]({{ site.docs-gm-rr }}){:target="_top"} and shown in the API samples named `RRGuaranteedRequestor` and `RRGuaranteedReplier`.

### Message Correlation

For request-reply messaging to be successful it must be possible for the requestor to correlate the request with the subsequent reply. PubSub+ messages support two fields that are needed to enable request-reply correlation. The reply-to field can be used by the requestor to indicate a PubSub+ Topic or Queue where the reply should be sent. A natural choice for this is often the unique `P2PInboxInUse` topic which is an auto-generated unique topic per client which is accessible as a session property. The second requirement is to be able to detect the reply message from the stream of incoming messages. This is accomplished using the correlation-id field. This field will transit the PubSub+ messaging system unmodified. Repliers can include the same correlation-id in a reply message to allow the requestor to detect the corresponding reply. The figure below outlines this exchange.

![]({{ site.baseurl }}/assets/images/Request-Reply_diagram-1.png)

For direct messages however, this is simplified through the use of the `Requestor` object as shown in this sample.

{% include_relative assets/solaceMessaging.md %}
{% include_relative assets/solaceApi.md %}
{% include_relative assets/init.md %}

## Making a request

First let’s look at the requestor. This is the application that will send the initial request message and wait for the reply.

![]({{ site.baseurl }}/assets/images/Request-Reply_diagram-2.png)

The requestor must create a message and the topic to send the message to:

```cpp
solClient_destination_t destination;
/* Set the destination Topic for the request message. */
destination.destType = SOLCLIENT_TOPIC_DESTINATION;
destination.dest = "topic/topic1";
if ( ( rc = solClient_msg_setDestination ( msg_p, &destination, sizeof ( destination ) ) ) == SOLCLIENT_OK ) 
{
    /* Create a stream in the binary attachment part of the message. */
    solClient_msg_createBinaryAttachmentStream ( msg_p, &stream_p, 100 );
    
    //Go ahead and add data into the binary stream.
}
```

Now the request can be sent. This example demonstrates a blocking call where the method will wait for the response message to be received.

```cpp
solClient_opaqueMsg_pt replyMsg_p;
int timeout = 5000; //5 sec

if (solClient_session_sendRequest ( opaqueSession_p,msg_p, &replyMsg_p, timeout) == SOLCLIENT_OK)
{
    //proceed to extract reply result
}
```

If the call request was executed successfully then the returned return code is `SOLCLIENT_OK`.

If the timeout is set to zero then the `solClient_session_sendRequest` call becomes non-blocking and it returns immediately.

## Replying to a request

Now it is time to receive the request and generate an appropriate reply.

![Request-Reply_diagram-3]({{ site.baseurl }}/assets/images/Request-Reply_diagram-3.png)

Just as with previous tutorials, you still need to connect a session and subscribe to the topics that requests are sent on. The following is an example of such reply.

```cpp

static solClient_rxMsgCallback_returnCode_t;
requestMsgReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    solClient_returnCode_t rc = SOLCLIENT_OK;
    solClient_opaqueMsg_pt replyMsg_p;
    solClient_opaqueContainer_pt stream_p;
    solClient_opaqueContainer_pt replyStream_p;
    double result;
    if ( ( rc = solClient_msg_getBinaryAttachmentStream ( msg_p, &stream_p ) ) == SOLCLIENT_OK ) 
    {
        //extract binary data from stream
    }

    //Proceed to create a reply
    solClient_msg_alloc ( &replyMsg_p );
    if ( ( rc = solClient_msg_createBinaryAttachmentStream ( replyMsg_p, &replyStream_p, 32 ) ) == SOLCLIENT_OK ) 
    {
        //insert binary data as reply

        //Send reply
        solClient_session_sendReply ( opaqueSession_p, msg_p, replyMsg_p );
    }
}
```

The `requestMsgReceiveCallback` function in this case replace the implementation of the `sessionMessageReceiveCallback`, which is passed as parameter in `solClient_session_create`.

## Receiving the Reply Message

All that’s left is to receive and process the reply message as it is received at the requestor. If you now update your requestor code to match the following you will see each reply printed to the console.

```cpp
if (solClient_session_sendRequest ( opaqueSession_p,msg_p, &replyMsg_p, timeout) == SOLCLIENT_OK)
{
    //proceed to extract reply result
    if ( solClient_msg_getBinaryAttachmentStream ( replyMsg_p, &replyStream_p ) == SOLCLIENT_OK ) 
    {
        //extract data from binary stream
    }    
}
solClient_msg_free ( &replyMsg_p );
```

## Summarizing

The full source code for this example is available in [GitHub]({{ site.repository }}){:target="_blank"}. If you combine the example source code shown above results in the following source:

<ul>
{% for item in page.links %}
<li><a href="{{ site.repository }}{{ item.link }}" target="_blank">{{ item.label }}</a></li>
{% endfor %}
</ul>

### Building

{% include_relative assets/building.md %}

### Running the Samples

First start the `BasicReplier` so that it is up and listening for requests. Then you can use the `BasicRequestor` sample to send requests and receive replies. Pass your PubSub+ messaging router connection properties as parameters.

```
bin$ . ./setenv.sh
bin$ ./BasicReplier -u <client-username>@<message-vpn> -c <protocol>://<msg_backbone_ip>:<port> -p <password> -t <topic>
Sending request for 9 PLUS 5
Received reply message, result = 14.000000
Sending request for 9 MINUS 5
Received reply message, result = 4.000000
Sending request for 9 TIMES 5
Received reply message, result = 45.000000
Sending request for 9 DIVIDED_BY 5
Received reply message, result = 1.800000
```

```
bin$ ./BasicRequestor -u <client-username>@<message-vpn> -c <protocol>://<msg_backbone_ip>:<port> -p <password> -t <topic>
Sending request for 9 PLUS 5
Received reply message, result = 14.000000
Sending request for 9 MINUS 5
Received reply message, result = 4.000000
Sending request for 9 TIMES 5
Received reply message, result = 45.000000
Sending request for 9 DIVIDED_BY 5
Received reply message, result = 1.800000
```

With that you now know how to successfully implement the request-reply message exchange pattern using Direct messages.

If you have any issues sending and receiving a message, check the [Solace community]({{ site.links-community }}){:target="_top"} for answers to common issues.
