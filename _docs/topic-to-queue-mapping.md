---
layout: tutorials
title: Topic to Queue Mapping
summary: Learn how to map topics to PubSub+ queues.
icon: I_dev_topic2q.svg
links:
    - label: TopicToQueueMapping.c
      link: /blob/master/src/intro/TopicToQueueMapping.c
    - label: common.c
      link: /blob/master/src/intro/common.c
---

This tutorial builds on the basic concepts introduced in the [Persistence with Queues tutorial]({{ site.baseurl }}/persistence-with-queues) and will show you how to make use of one of PubSub+’s advanced queueing features called “Topic to Queue Mapping.”

In addition to spooling messages published directly to the queue, it is possible to add one or more topic subscriptions to a durable queue so that messages published to those topics are also delivered to and spooled by the queue. This is a powerful feature that enables queues to participate equally in point to point and publish / subscribe messaging models. More details about the [“Topic to Queue Mapping” feature here]({{ site.docs-topic-queue}}){:target="_top"}.

The following diagram illustrates this feature.

<img src="{{ site.baseurl }}/assets/images/topic-to-queue-mapping-detail.png" alt="C Queue Mapping" width="500" height="206" />

If you have a durable queue named `Q`, it will receive messages published directly to the queue destination named `Q`. However, it is also possible to add subscriptions to this queue in the form of topics. This example adds topics `A` and `B`. Once these subscriptions are added, the queue will start receiving messages published to the topic destinations `A` and `B`. When you combine this with the wildcard support provided by PubSub+ topics this opens up a number of interesting use cases.

## Assumptions

This tutorial assumes the following:

*   You have access to PubSub+ messaging with the following configuration details:
    *   Connectivity information for a PubSub+ message-VPN configured for guaranteed messaging support
    *   Enabled client username and password
    *   Client-profile enabled with guaranteed messaging permissions.

One simple way to get access to PubSub+ messaging quickly is to create a messaging service in PubSub+ Cloud [as outlined here]({{ site.links-solaceCloud-setup}}){:target="_top"}. You can find other ways to get access to PubSub+ messaging below.

## Goals

The goal of this tutorial is to understand the following:

*   How to add topic subscriptions to a queue. Two ways are shown in this case.
*   How to interrogate the PubSub+ message router to confirm capabilities.
*   How to delete a queue.


{% include_relative assets/solaceMessaging.md %}
{% include_relative assets/solaceApi.md %}
{% include_relative assets/init.md %}

## Review: Receiving message from a queue

The [Persistence with Queues tutorial]({{ site.baseurl }}/persistence-with-queues) demonstrated how to publish and receive messages from a queue. This sample will do so in the same way. This sample will also depend on the endpoint being provisioned by through the API as was done in the previous tutorial. For clarity, this code is not repeated in the discussion but is included in the [full source available in GitHub]({{ site.repository }}/blob/master/src/TopicToQueueMapping/TopicToQueueMapping.cs){:target="_blank"}.

## Adding a Subscription to a Queue

In order to enable a queue to participate in publish/subscribe messaging, you need to add topic subscriptions to the queue to attract messages. In this sample, adding a subscription is demonstrated in 2 ways:
* Through the session using `solClient_session_endpointTopicSubscribe` 
* Through a created flow to the queue using `solClient_flow_topicSubscribeWithDispatch` 
  
```cpp
    // Create a session 

    // Subscribe through the Session
    const char     *props[40] = {0, };
    propIndex = 0;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_ID;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_QUEUE;
    props[propIndex++] = SOLCLIENT_ENDPOINT_PROP_NAME;
    props[propIndex++] = "Q";
    char* pTopic1 = "A";
    char* pTopic2 = "B";
    
    solClient_session_endpointTopicSubscribe ( (char**)props, session_p,
                                                SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                                pTopic1 );
    // Create a flow to the queue

    // Subscribe through a flow
    solClient_flow_topicSubscribeWithDispatch ( flow_p,
                                                SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
                                                pTopic2,
                                                NULL,       /* no dispatch functions */
                                                0           /* correlation tag pointer */
                                                );

```

## Publish – Subscribe using a Queue

Once the subscription is added to the queue, all that is left to do in this tutorial is to send some messages to your topic and validate that they arrive on the queue. 

```cpp
    char* pTopic = "A" // or B
    solClient_opaqueMsg_pt msg_p = NULL;
    solClient_destination_t destination;

    // Allocate memory for the message to be sent. 
    solClient_msg_alloc ( &msg_p );

    // Set the message delivery mode. 
    solClient_msg_setDeliveryMode ( msg_p, SOLCLIENT_DELIVERY_MODE_PERSISTENT );

    // Set the destination. */
    destination.destType = SOLCLIENT_TOPIC_DESTINATION;
    destination.dest = pTopic;
    if ( (solClient_msg_setDestination ( msg_p, &destination, sizeof ( destination ) ) ) != SOLCLIENT_OK ) {
        solClient_session_sendMsg ( session_p, msg_p );
    }

    solClient_msg_free ( &msg_p );

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

* Start two `TopicSubscriber` sample in 2 separate terminals subscribing to topic `my/sample/topic/1` and `my/sample/topic/2` respectively.
* Start `QueueSubscriber` in another terminal subscribing to queue `my_sample_queue`.
* Start `TopicToQueueMapping` in another terminal:
```
bin$ . ./setenv.sh
bin$ ./TopicToQueueMapping -u <client-username>@<message-vpn> -c <protocol>://<msg_backbone_ip>:<port> -p <password>
```
You will then see the queue and topic subscriber outputs as below:

On `QueueSubscriber` subscribing to `my_sample_queue`
```
Connected.
Waiting for messages......
Received message:
Destination:                            Topic 'my/sample/topic/1'
SenderId:                               ubuntu-xenial/XXXXX.......
SequenceNumber:                         1
SenderTimestamp:                        1547621987332 (Wed Jan 16 2019 06:59:47)
Class Of Service:                       COS_1
DeliveryMode:                           PERSISTENT
Message Id:                             1
Binary Attachment:                      len=16
  6d 79 20 61 74 74 61 63  68 65 64 20 64 61 74 61      my attac   hed data

Acknowledging message Id: 1.
Received message:
Destination:                            Topic 'my/sample/topic/2'
SenderId:                               ubuntu-xenial/XXXXX......
SequenceNumber:                         2
SenderTimestamp:                        1547621987334 (Wed Jan 16 2019 06:59:47)
Class Of Service:                       COS_1
DeliveryMode:                           PERSISTENT
Message Id:                             2
Binary Attachment:                      len=16
  6d 79 20 61 74 74 61 63  68 65 64 20 64 61 74 61      my attac   hed data

Acknowledging message Id: 2.
Exiting.
```
On `TopicSubscriber` subswcribing to `my/sample/topic/1`
```
TopicSubscriber initializing...
Connected.
Waiting for message......
Received message:
Destination:                            Topic 'my/sample/topic/1'
SenderId:                               ubuntu-xenial/31886/#00000001/PsW7c5FyP1
SequenceNumber:                         1
SenderTimestamp:                        1547621987332 (Wed Jan 16 2019 06:59:47)
Class Of Service:                       COS_1
DeliveryMode:                           DIRECT
Message Id:                             1
Binary Attachment:                      len=16
  6d 79 20 61 74 74 61 63  68 65 64 20 64 61 74 61      my attac   hed data
```
On `TopicSubscriber` subswcribing to `my/sample/topic/2`
```
opicSubscriber initializing...
Connected.
Waiting for message......
Received message:
Destination:                            Topic 'my/sample/topic/2'
SenderId:                               ubuntu-xenial/31886/#00000001/PsW7c5FyP1
SequenceNumber:                         2
SenderTimestamp:                        1547621987334 (Wed Jan 16 2019 06:59:47)
Class Of Service:                       COS_1
DeliveryMode:                           DIRECT
Message Id:                             2
Binary Attachment:                      len=16
  6d 79 20 61 74 74 61 63  68 65 64 20 64 61 74 61      my attac   hed data
```
You have now added a topic subscription to a queue and successfully published persistent messages to the topic and had them arrive on your Queue endpoint.

If you have any problems with this tutorial, check the [Solace community]({{ site.links-community }}){:target="_top"} for answers to common issues.
