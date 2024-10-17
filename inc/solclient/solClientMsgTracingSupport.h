
#ifndef SOLCLIENTMSGTRACINGSUPPORT_H
#define SOLCLIENTMSGTRACINGSUPPORT_H

#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * Trace context type
 */
typedef enum solClient_msg_tracing_context_type
{
    TRANSPORT_CONTEXT = 0,
    CREATION_CONTEXT = 1
} solClient_msg_tracing_context_type_t;

/**
 * Injection Standard type
 */
typedef enum solClient_msg_tracing_injection_standard_type
{
    SOLCLIENT_INJECTION_STANDARD_SMF = 0,
    SOLCLIENT_INJECTION_STANDARD_W3C = 1
} solClient_msg_tracing_injection_standard_type_t;

/**
 * Copies the value of trace identifier associated with this message as
 * byte array from a specified context into the given pointer.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param contextType The type of context to be used for retrieval; one of ::solClient_msg_tracing_context_type.
 * @param traceId_p   An address of a byte array large  enough to hold the traceId
 * @param size        A maximum length of traceId byte array, usual size of a trace id byte array is 16.
 *
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_getTraceIdByte (solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        solClient_uint8_t          *traceId_p,
        size_t                      size);



/**
 *  Copies the value of span identifier from a specified context associated with this message as
 *  a byte array into the given pointer.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *  @param contextType  The type of context to be used for retrieval; one of ::solClient_msg_tracing_context_type.
 *  @param spanId_p   An address of a byte array large  enough to hold the spanId
 *  @param size         A maximum length of spanId byte array, usual size of a span id byte array is 8.
 *
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_getSpanIdByte (solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        solClient_uint8_t          *spanId_p,
        size_t                      size);


/**
 *  Copies the value of a sampled flag from a specified context associated with this message into the given pointer.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *  @param contextType  The type of context to be used for retrieval; one of ::solClient_msg_tracing_context_type.
 *  @param value   True if trace should be sampled, only sampled traces will apper on a backend
 *
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_isSampled (solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        solClient_bool_t      *value);





/**
 * Sets the value of trace identifier into a message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *  @param contextType The context to update; one of ::solClient_msg_tracing_context_type.
 *  @param traceId_p    A pointer to a byte array with traceId.
 *  @param size         A length of traceId byte array, usual size of a trace id byte array is 16.
 *
 *  @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_setTraceIdByte    (solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        const solClient_uint8_t             *traceId_p,
        size_t                               size);



/**
 *  Sets the value of span identifier into a message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *  @param contextType The context to update; one of ::solClient_msg_tracing_context_type.
 *  @param spanId_p   A pointer to a byte array with spanId.
 *  @param size     A length of traceId byte array, usual size of a span id byte array is 8.
 *  @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_setSpanIdByte (solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        const solClient_uint8_t              *spanId_p,
        size_t                                size);


/**
 *  Sets the value of a sampled flag into a message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *  @param contextType The context to update; one of ::solClient_msg_tracing_context_type.
 *  @param value   A sampled flag value to copy from into the message.
 *
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_setSampled (solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        const solClient_bool_t               value);

/**
 * Pointer-based getter variant for the TraceState.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param contextType The context to access; one of ::solClient_msg_tracing_context_type.
 * @param traceState_pp  Pointer to traceState within message (output param) It is expected that byte sequence is UTF8 encoded but not null terminated.
 * @param size_p         The length of traceState string (output param)
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 */

solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_getTraceStatePtr(solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        const char      **traceState_pp,
        size_t    *size_p);

/**
 * Given a msg_p, set the object used for carrying over of the distributed tracing
 * message trace state information. This function
 * copies in from the given trace state object associated with a given pointer. Changes to it after this function is called
 * will not be propagated to this message. The message will be encoded suitable for reading by any other Solace Messaging API.
 * Distributed tracing information can be extracted when Solace API supports such extraction.
 * If any trace state previously existed in the message it is first removed before the new data is copied in.
 *
 * Do <b>not</b> call this method with a baggage that is already in the message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive message callback.
 * @param contextType The context to access; one of ::solClient_msg_tracing_context_type.
 * @param traceState_p  A pointer to a null terminated trace state UTF8 encoded string.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid or memory is not available.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */     
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_setTraceState(solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
	const char *traceState_p);


/**
 * Given a msg_p, set the object used for carrying over of the distributed tracing
 * message trace state information. This function
 * copies in from the given trace state object associated with a given pointer. Changes to it after this function is called
 * will not be propagated to this message. The message will be encoded suitable for reading by any other Solace Messaging API.
 * Distributed tracing information can be extracted when Solace API supports such extraction.
 * If any trace state previously existed in the message it is first removed before the new data is copied in.
 *
 * Do <b>not</b> call this method with a baggage that is already in the message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive message callback.
 * @param contextType The context to access; one of ::solClient_msg_tracing_context_type.
 * @param traceState_p  A pointer to the character array containing the trace state with UTF8 encoding but no null termination.
 * @param size The size of the trace state character sequence.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid or memory is not available.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */     
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_setTraceStatePtr(solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
	const char *traceState_p, size_t size);

/**
 * Given a msg_p, retrieves the object used for carrying over of the distributed tracing
 * baggage information
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param baggage_p    Baggage string pointer on return. It is expected that string is UTF8 encoded.
 * @param size         A maximum length of baggage string
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 */         
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_getBaggage(solClient_opaqueMsg_pt msg_p,
        char      *baggage_p,
        size_t                      size);

/**
 * Pointer variant of the distributed tracing baggage getter.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param baggage_pp    Baggage string pointer (output param). It is expected that string is UTF8 encoded.
 * @param size_p        The length of baggage string (output param)
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 */         
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_getBaggagePtr(solClient_opaqueMsg_pt msg_p,
        const char      **baggage_pp,
        size_t     *size_p);


/**
 * Given a msg_p, set the object used for carrying over of the distributed tracing
 * message baggage information, as referenced by the given solClient_opaqueTraceContext_pt. This function
 * copies in from the given baggage object. Changes to it after this function is called
 * will not be propagated to this message. The message will be encoded suitable for reading by any other Solace Messaging API.
 * Distributed tracing information can be extracted when Solace API supports such extraction.
 * If any baggage previously existed in the message it is first removed before the new data is copied in.
 *
 * Do <b>not</b> call this method with a baggage that is already in the message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive message callback.
 * @param baggage_p  A pointer to a null terminated baggage UTF8 encoded string.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid or memory is not available.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_setBaggage(solClient_opaqueMsg_pt msg_p,
        const char      *baggage_p); 


/**
 * Removes a tracing context from a message. All data (fields) set on that context is lost.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param contextType The type of context to delete; one of ::solClient_msg_tracing_context_type.
 *
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_deleteContext(solClient_opaqueMsg_pt opaqueMsg_p,
        solClient_msg_tracing_context_type_t contextType);

/**
 * Removes tracing baggage from a message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_deleteBaggage(solClient_opaqueMsg_pt opaqueMsg_p);


/**
 * Copies the value of the injection standard field in this message as
 * byte from a specified context into the given pointer.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param contextType The type of context to be used for retrieval; one of ::solClient_msg_tracing_context_type.
 * @param injectionStandard_p   An address of a byte to hold the injection standard
 *
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_getInjectionStandardByte (solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        solClient_msg_tracing_injection_standard_type_t          *injectionStandard_p);

/**
 * Updates the value of the injection standard field in the specified context of this message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param contextType The type of context to update; one of ::solClient_msg_tracing_context_type.
 * @param injectionStandard  The byte to set as the injection standard.
 *
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_tracing_setInjectionStandardByte (solClient_opaqueMsg_pt msg_p,
        solClient_msg_tracing_context_type_t contextType,
        solClient_msg_tracing_injection_standard_type_t          injectionStandard);


#if defined(__cplusplus)
} // extern C
#endif

#endif
