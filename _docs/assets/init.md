## Connecting to the Solace PubSub+ message router

In order to send or receive messages, an application must connect a PubSub+ session. The PubSub+ session is the basis for all client communication with the PubSub+ message router.

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
sessionMessageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    printf ( "Received message:\n" );
    solClient_msg_dump ( msg_p, NULL, 0 );
    printf ( "\n" );

    msgCount++;
    return SOLCLIENT_CALLBACK_OK;
}

void
sessionEventCallback ( solClient_opaqueSession_pt opaqueSession_p,
                solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{  
    printf("Session EventCallback() called:  %s\n", solClient_session_eventToString ( eventInfo_p->sessionEvent));
}
```

The messageReceiveCallback is invoked for each message received by the Session. In this sample, the message is printed to the screen.

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

Finally a session is needed to actually connect to the PubSub+ message router. This is accomplished by creating a properties array and connecting the session.

```cpp
/* Session */
solClient_opaqueSession_pt session_p;
solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

/* Session Properties */
const char     *sessionProps[50] = {0, };
int             propIndex = 0;
char *username,*password,*vpnname,*host;

/* Configure the Session function information. */
sessionFuncInfo.rxMsgInfo.callback_p = sessionMessageReceiveCallback;
sessionFuncInfo.rxMsgInfo.user_p = NULL;
sessionFuncInfo.eventInfo.callback_p = sessionEventCallback;
sessionFuncInfo.eventInfo.user_p = NULL;

/* Configure the Session properties. */
propIndex = 0;
host = argv[1];
vpnname = argv[2];
username = strsep(&vpnname,"@");
password = argv[3];

sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
sessionProps[propIndex++] = host;

sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
sessionProps[propIndex++] = vpnname;

sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
sessionProps[propIndex++] = username;

sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_PASSWORD;
sessionProps[propIndex] = password;

/* Create the Session. */
solClient_session_create ( ( char ** ) sessionProps,
                           context_p,
                           &session_p, &sessionFuncInfo, sizeof ( sessionFuncInfo ) );

/* Connect the Session. */
solClient_session_connect ( session_p );
printf ( "Connected.\n" );
```

When creating the session, the factory method takes the session properties, the session pointer and information about the session callback functions. The API then creates the session within the supplied context and returns a reference in the session pointer. The final call to solClient_session_connect establishes the connection to the PubSub+ message router which makes the session ready for use.

At this point your client is connected to the PubSub+ message router. You can use SolAdmin to view the client connection and related details.
