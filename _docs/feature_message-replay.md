---
layout: features
title: Message Replay
summary: Learn how to make use of Message Replay via the Solace Java client library
links:
    - label: MessageReplay.java
      link: /blob/master/src/main/java/com/solace/samples/features/MessageReplay.java
---

In this introduction we show you how a client can initiate and process the replay of previously published messages, as well as deal with an externally initiated replay.

## Feature Overview

During normal publishing, guaranteed messages will be removed from the message broker's [queue or topic endpoint]({{ site.docs-endpoints }}) once the consumer acknowledges their receipt or successful processing. When Message Replay is initiated for an endpoint, the message broker will re-publish a requested subset of previously published and logged messages, which enables the client to process these messages again.

Message Replay can be used if a client needs to catch up with missed messages as well as for several other [use cases]({{ site.docs-replay-use-cases }}).

Message Replay for an endpoint can be initiated programmatically from an API client connected to an exclusive endpoint, or administratively from the message broker. After the replay is done, the connected client will keep getting live messages delivered.

It's important to note that when initiating replay, the message broker will disconnect all connected client flows, active or not. A new flow needs to be started for a client wishing to receive replayed and subsequent messages. The only exception is that in the client initiated case the flow initiating the replay will not be disconnected.

## Prerequisite

A replay log must be created on the message broker for the Message VPN using [Message Replay CLI configuration]({{ site.docs-replay-cli-config }}) or using [Solace PubSub+ Manager]({{ site.docs-psplus-manager }}) administration console. Another option for configuration is to use the [SEMP API]({{ site.docs-semp-api }}).

NOTE: Message Replay is supported on Solace PubSub+ 3530 and 3560 appliances running release 9.1 and greater, and on the Solace PubSub+ software message broker running release 9.1 and greater. Solace Java API version 10.5 or later is required.

![alt text]({{ site.baseurl }}/assets/images/config-replay-log.png "Configuring Replay Log using Solace PubSub+ Manager")
<br>

## Code

### Checking for Message Replay capability

Message Replay must be supported on the message broker, so this should be the first thing the code checks:

```cpp
if ( !solClient_session_isCapable ( session_p, SOLCLIENT_SESSION_CAPABILITY_MESSAGE_REPLAY ) ) {

    printf ( "Message replay not supported on this message broker.\n" );
    return -1;
}
```

### Initiating replay

First, a "replay start location" needs to be configured in the flow properties to specify the desired subset of messages in the replay log.

There are two options:
* use `SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION_BEGINNING` to replay all logged messages
* use `DATE:date` to replay all logged messages received starting from a specified `date`. Note the different possible formats in the example including how to specify the time zone.

Note: The `date` can't be earlier than the date the replay log was created, otherwise replay will fail.

```cpp
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
```

Replay is requested by setting a non-NULL `SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION` property and non-NULL location value in the flow properties, which is then passed to `solClient_session_createFlow()` as a parameter.

The target endpoint for replay is also set in the flow properties below, which is the normal way of setting an endpoint for a consumer flow.

```cpp
/* Flow Properties */
const char     *flowProps[20] = {NULL};
int             provIndex;
:
flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_ACKMODE;
flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_ACKMODE_CLIENT;
:
flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_BIND_NAME;
flowProps[propIndex++] = argv[5];         // queue name taken from input param
:
flowProps[propIndex++] = SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION;
flowProps[propIndex] = SOLCLIENT_FLOW_PROP_REPLAY_START_LOCATION_BEGINNING;
:
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
printf ( "Waiting for 10 messages......\n" );
``` 

### Replay-related events

`SOLCLIENT_FLOW_EVENT_DOWN_ERROR` is the replay-related event that has several Subcodes defined corresponding to various conditions, which can be processed in the `flowEventCallback` event handler.

Note that in the Java API, the event handler is called on the main reactor thread, and manipulating the `session` from here isn't allowed because it can lead to deadlock. There will also be a related exception raised where the `session` can be manipulated in the exception handler, see the description in the next section. 

Some of the important Subcodes:
* REPLAY_STARTED - a replay has been administratively started from the message broker; the consumer flow is being disconnected.
* REPLAY_START_TIME_NOT_AVAILABLE - the requested replay start date is before when the replay log was created, which is not allowed - see above section, "Initiating replay"
* REPLAY_FAILED - indicates that an unexpected error has happened during replay

For the definition of additional replay-related Subcodes refer to the `JCSMPErrorResponseSubcodeEx` class in the [Java API Reference]({{ site.docs-api-errorresponse-subcode-ex }}).

Here we will define the `ReplayFlowEventHandler` to process events with some more example Subcodes. The event handler will set `replayErrorResponseSubcode`, which will be used in the exception handler.

```java
private volatile int replayErrorResponseSubcode = JCSMPErrorResponseSubcodeEx.UNKNOWN;
class ReplayFlowEventHandler implements FlowEventHandler {
    @Override
    public void handleEvent(Object source, FlowEventArgs event) {
        System.out.println("Consumer received flow event: " + event);
        if (event.getEvent() == FlowEvent.FLOW_DOWN) {
            if (event.getException() instanceof JCSMPErrorResponseException) {
                JCSMPErrorResponseException ex = (JCSMPErrorResponseException) event.getException();
                // Store the subcode for the exception handler
                replayErrorResponseSubcode = ex.getSubcodeEx();
                // Placeholder for additional event handling
                // Do not manipulate the session from here
                // onException() is the correct place for that
                switch (replayErrorResponseSubcode) {
                    case JCSMPErrorResponseSubcodeEx.REPLAY_STARTED:
                    case JCSMPErrorResponseSubcodeEx.REPLAY_FAILED:
                    case JCSMPErrorResponseSubcodeEx.REPLAY_CANCELLED:
                    case JCSMPErrorResponseSubcodeEx.REPLAY_LOG_MODIFIED:
                    case JCSMPErrorResponseSubcodeEx.REPLAY_START_TIME_NOT_AVAILABLE:
                    case JCSMPErrorResponseSubcodeEx.REPLAY_MESSAGE_UNAVAILABLE:
                    case JCSMPErrorResponseSubcodeEx.REPLAYED_MESSAGE_REJECTED:
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
:
private ReplayFlowEventHandler consumerEventHandler = null;
:
consumerEventHandler = new ReplayFlowEventHandler();
:
/*
 * Create and start a consumer flow
 */
consumer = session.createFlow(this, consumerFlowProps, null, consumerEventHandler);
```

### Replay-related Exceptions

If a replay-related event occurs, the flow is disconnected with `JCSMPFlowTransportUnsolicitedUnbindException`, and the exception handler is called if the `onException` method is overridden in the `XMLMessageListener` object that was passed to `createFlow()`.

In this sample the `MessageReplay` class implements `XMLMessageListener`, hence `this` is passed as the first parameter to `createFlow()`.

Here is the overridden `onException` method. In this example `JCSMPFlowTransportUnsolicitedUnbindException` is handled depending on the `replayErrorResponseSubcode`, which was set by the event handler (see previous section).
* REPLAY_STARTED is handled by creating a new flow. 
* REPLAY_START_TIME_NOT_AVAILABLE is handled by adjusting `ReplayStartLocation` to replay all logged messages. 

```
@Override
public void onException(JCSMPException exception) {
    if (exception instanceof JCSMPFlowTransportUnsolicitedUnbindException) {
        try {
            if (exception instanceof JCSMPFlowTransportUnsolicitedUnbindException) {
                switch (replayErrorResponseSubcode) {
                    case JCSMPErrorResponseSubcodeEx.REPLAY_STARTED:
                        System.out.println("Sample handling of an unsolicited unbind for replay initiated by recreating the flow");
                        if (consumerFlowProps.getReplayStartLocation() != null) {
                            consumerFlowProps.setReplayStartLocation(null);
                        }
                        consumer = session.createFlow(this, consumerFlowProps, null, consumerEventHandler);
                        consumer.start();
                        break;
                    case JCSMPErrorResponseSubcodeEx.REPLAY_START_TIME_NOT_AVAILABLE:
                        System.out.println("Start date was before the log creation date, initiating replay for all messages instead");
                        consumerFlowProps.setReplayStartLocation(JCSMPFactory.onlyInstance().createReplayStartLocationBeginning());
                        consumer = session.createFlow(this, consumerFlowProps, null, consumerEventHandler);
                        consumer.start();
                        break;
                    default:
                        break;
                }
                replayErrorResponseSubcode = JCSMPErrorResponseSubcodeEx.UNKNOWN; // reset after handling
            }
        }
        catch (JCSMPException e) {
            e.printStackTrace();
        }
    } else {
        exception.printStackTrace();
    }
}
```

## Running the Sample

Follow the instructions to [build the samples]({{ site.repository }}/blob/master/README.md#build-the-samples ).

Before running this sample, be sure that Message Replay is enabled in the Message VPN. Also, messages must have been published to the replay log for the queue that is used. The "QueueProducer" sample can be used to create and publish messages to the queue. The "QueueConsumer" sample can be used to drain the queue so that replay is performed on an empty queue and observed by this sample. Both samples are from the [Persistence with Queues]({{ site.baseurl }}/persistence-with-queues) tutorial.

```
$ ./build/staged/bin/queueProducer <host:port> <client-username>@<message-vpn> [<client-password>]
$ ./build/staged/bin/queueConsumer <host:port> <client-username>@<message-vpn> [<client-password>]
```

At this point the replay log has one message.

You can now run this sample and observe the following, particularly the "messageId"s listed:

1. First, a client initiated replay is started when the flow connects. All messages are requested and replayed from the replay log.
```
$ ./build/staged/bin/featureMessageReplay -h <host:port> -u <client-username>@<message-vpn> \
                                          -q Q/tutorial [-w <client-passWord>]
```
2. After replay the application is able to receive live messages. Try it by publishing a new message using the "QueueProducer" sample from another terminal. Note that this message will also be added to the replay log.
```
$ ./build/staged/bin/queueProducer <host:port> <client-username>@<message-vpn> [client-password]
```
3. Now start a replay from the message broker. The flow event handler monitors for a replay start event. When the message broker initiates a replay, the flow will see a DOWN_ERROR event with cause 'Replay Started'. This means an administrator has initiated a replay, and the application must destroy and re-create the flow to receive the replayed messages.
This will replay all logged messages including the live one published in step 2.

![alt text]({{ site.baseurl }}/assets/images/initiate-replay.png "Initiating Replay using Solace PubSub+ Manager")
<br>

## Learn More

<ul>
{% for item in page.links %}
<li>Related Source Code: <a href="{{ site.repository }}{{ item.link }}" target="_blank">{{ item.label }}</a></li>
{% endfor %}
<li><a href="https://docs.solace.com/Features/Message-Replay.htm" target="_blank">Solace Feature Documentation</a></li>
</ul>

