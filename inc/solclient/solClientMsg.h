/**
* @file solClientMsg.h include file for Solace Corporation Messaging Buffer Management
*
* Copyright 2008-2024 Solace Corporation. All rights reserved.
*/
/** @page msgbuffer Solace Message Buffers
* 
* @section solclientmsg Solace Message Buffers
*
* Solace Message Buffers provide clients with
* controlled buffer management. Applications that use
* Solace Message Buffers must first allocate a message buffer with 
* \link ::solClient_msg_alloc solClient_msg_alloc()\endlink. This returns an
* opaque pointer to a Solace message buffer that must be released by the
* application when it is finished with the message. Message buffers are 
* released by calling \link ::solClient_msg_free solClient_msg_free().\endlink
* 
* Applications are responsible for releasing all message buffers they allocate by
* solClient_msg_alloc() or solClient_msg_dup(). Message buffers received by
* callback are owned by the API and <b>must not</b> be released. However the
* application may also take ownership of these message buffers by returning 
* ::SOLCLIENT_CALLBACK_TAKE_MSG on return from the receive message callback
* function. If the application returns ::SOLCLIENT_CALLBACK_TAKE_MSG, it
* <b>must</b> call solClient_msg_free() to release the message when it is
* finished with the message buffer.
* 
* Message buffers 
*
*
* The Message Buffer API provides functions to manipulate the common Solace
* message header fields that are optionally sent in the binary metadata 
* portion of the Solace message. 
* Applications can also use the structured data API to add containers 
* (maps or streams) and their fields to the binary payload or to the User Property
* map contained within the binary metadata.
*
* This does not prevent applications from ignoring these 
* functions and treating these payloads as an opaque binary field for
* end-to-end communications.
*
* The Solace common binary header fields that may be set/read by this message buffer API are:
*
* @li ReplyTo - a ReplyTo destination.
* @li SenderId - a Sender Identification. This field can be automatically generated when 
*                the session property 
*                ::SOLCLIENT_SESSION_PROP_GENERATE_SENDER_ID is enabled. When
*                this property is enabled, and the SenderId is not explicitly
*                set, the session ClientName is used as the SenderId.<br>
*                The SenderID is user-defined, carried end-to-end, and can also be matched
*                in a selector, but otherwise is not relevant to the event broker.
* @li Application MsgType - a string for an application specific MsgType.
*                 The ApplicationMessageType is user-defined,
*                 carried end-to-end, and can also be matched in a selector, but otherwise is not relevant
*                 to the event broker.<br>
*                 In JMS applications, this field is carried as the JMSType Message Header Field.
* @li Application MessageId - a string for an application-specific Message Identifier.
*                 The ApplicationMessageId is user-defined,
*                 carried end-to-end, and can also be matched in a selector, but otherwise is not relevant
*                 to the event broker.<br>
*                 In JMS applications, this field is carried as the JMSMessageID Message Header Field.
* @li Sequence Number  - a 64-bit application Sequence Number, set by Publisher.
*                This field may be automatically generated when the session property
*                ::SOLCLIENT_SESSION_PROP_GENERATE_SEQUENCE_NUMBER is 
*                enabled. When this property is enabled, and the Sequence 
*                Number is not explicitly set, a unique and increasing sequence
*                number is generated for each sent message.<br>
*                The SequenceNo is user-defined, carried end-to-end, and can also be matched
*                in a selector, but otherwise is not relevant to the event broker.
* @li CorrelationId - a string for an application-specific correlation tag.
*                 The CorrelationId is user-defined,
*                 carried end-to-end, and can also be matched in a selector, but otherwise is not relevant
*                 to the event broker.  The CorrelationId may be used for peer-to-peer message synchronization.<br>
*                 In JMS applications, this field is carried as the JMSCorrelationID Message Header Field.
* @li Send Timestamp - a 64-bit field set by the Publisher. This field may be
*                automatically generated when the session property
*                ::SOLCLIENT_SESSION_PROP_GENERATE_SEND_TIMESTAMPS 
*                is enabled in the sending API.
* @li Receive Timestamp - a read-only field inserted by the receiving API when the
*                session property ::SOLCLIENT_SESSION_PROP_GENERATE_RCV_TIMESTAMPS 
*                is enabled in the receiving API.
*
* @section encoding-overhead Encoding Overhead of Data Types in Streams and Maps
* The overhead of a stream or a map structure is 5 bytes, plus the 
* contents of the stream or map.
* The list below shows the overhead of various data types when added to a stream or map.
* When adding to a map, the overhead is the overhead of the type to be stored, plus the
* overhead of the string field name. The overhead of the string field name is the same
* as the overhead shown below for a string value.
* @li null - 2 bytes
* @li ::solClient_bool_t - 3 bytes
* @li char - 4 bytes
* @li ::solClient_wchar_t - 4 bytes
* @li ::solClient_int8_t or ::solClient_uint8_t - 3 bytes
* @li ::solClient_int16_t or ::solClient_uint16_t - 4 bytes
* @li solClient_int32_t or solClient_uint32_t - 6 bytes
* @li ::solClient_int64_t or ::solClient_uint64_t - 10 bytes
* @li float - 6 bytes
* @li double - 10 bytes
* @li byte array - number of bytes in byte array plus: 
*                  2 if # bytes <= 253; 
*                  3 if # bytes <= 65552; 
*                  4 if # bytes <= 16777211,
*                  5 otherwise
* @li string - string length plus: 
*              3 if string length <= 252; 
*              4 if string length <= 65551; 
*              5 if string length <= 16777210,
*              6 otherwise
* @li ::solClient_destination_t - string length of destination plus:
*                                 4 if string length <= 251; 
*                                 5 otherwise
* @li SMF message - same as byte array
*
* If a stream is added to the binary payload of a message, and the 
* stream contains a ::solClient_int16_t and a ::solClient_int64_t, the stream
* size will be 19 bytes (5 bytes for the stream overhead, 4 bytes for the 
* ::solClient_int16_t, and 10 bytes for the ::solClient_int64_t).
*
* If a map is added to the binary payload of a message, and the map
* contains an entry of type ::solClient_int16_t with a field name
* of "FirstFieldName" and an entry of type ::solClient_int8_t with a field name of 
* "SecondFieldName", the map will be 47 bytes (5 bytes for the map overhead,
* 17 bytes for the first field name string, 4 bytes for the ::solClient_int16_t value,
* 18 bytes for the second field name string, and 3 bytes for the ::solClient_int8_t
* value).
 *
 * @section msgMaps Solace Structured Data Maps
 *
 * Solace structured data maps (::solClient_msg_createBinaryAttachmentMap(), 
 * ::solClient_container_createMap(), solClient_msg_createUserPropertyMap())
 * provide the programmer with a structure whose members can be directly 
 * accessed by name. Programmers familiar with a hashMap will find the same 
 * concepts in a Solace structured data map. There is one notable exception -
 * Solace structured data maps do not detect duplicate key names on 'add'. If a 
 * field is added to the map with a key name that already exist, both fields will 
 * be present in the map. Only one field can be retrieved by that key name and it 
 * is not defined which field will be retrieved.
 *
 * Solace maps have been designed for efficient construction and transmission.
 * The maps can be read fastest by iterating over the map using 
 * solClient_container_getNextField(). If each field in a map is retrieved 
 * by name instead, the read time for the whole map is order independent.
 *
 * The key name field in Solace maps is case sensitive.  For example, adding 
 * a field with the name key "field_1" and a field with name key "FIELD_1" will 
 * create two distinct fields that can both be retrieved.
 *
 * @section adConsiderations Using Solace Message Buffers for Guaranteed messages
 * Solace messages buffers can be used to hold Guaranteed messages. You can call the 
 * accessor ::solClient_msg_setDeliveryMode() to set one of the following delivery
 * modes:
 * @li ::SOLCLIENT_DELIVERY_MODE_DIRECT
 * @li ::SOLCLIENT_DELIVERY_MODE_PERSISTENT
 * @li ::SOLCLIENT_DELIVERY_MODE_NONPERSISTENT
 *
 * The default mode of any message is ::SOLCLIENT_DELIVERY_MODE_DIRECT for the
 * fast reliable delivery (not Guaranteed), known as Direct Delivery. 
 *
 * To provide the best possible latency and throughput, the C API does 
 * <b>NOT</b> copy the contents of the message.
 * The message contents are maintained internally,
 * exactly as presented to ::solClient_session_sendMsg(), until the message 
 * is acknowledged by the Solace Messaging Appliance. In the event that a message needs to
 * be recovered by retransmission, the contents are seamlessly redelivered.
 *
 * This means the application must ensure that the message buffer's
 * contents remain untouched until notified by the session event callback 
 * that the message has been acknowledged. When sending a Guaranteed
 * message, the application must not modify any data referenced by the message,
 * until the ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT is received. This is 
 * essential for applications that set message buffer parts by pointer 
 * (::solClient_msg_setBinaryAttachmentPtr, ::solClient_msg_setUserDataPtr, 
 * ::solClient_msg_setXmlPtr, ::solClient_msg_setTopicPtr, and
 * ::solClient_msg_setQueueNamePtr). When the application memory is used to 
 * hold message parts it <b>must not</b> be modified before the message is 
 * acknowledged. If the C API manages all memory in the message, it may be 
 * modified but the application will pay a performance penalty to do so.
 *
 * The message correlation tag is provided as a convenient location for an 
 * application to store stateful information. A pointer to the data with the 
 * correlation tag field is returned to the application with the 
 * ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT event. A typical application can
 * set the correlation tag pointer to its own memory for later retrieval of the 
 * message pointer to free. For example:
 *
 * @code
 *      struct myData {
 *          solClient_msg_pt    msg_p;
 *      } *myData_p;
 *      solClient_msg_alloc(&myData_p->msg_p);
 *      solClient_msg_setDeliveryMode(msg_p, SOLCLIENT_DELIVERY_MODE_PERSISTENT);
 *
 *      // set up message attachments, etc. here
 *
 *      solClient_msg_setCorrelationTagPtr(msg_p, myData_p, sizeof(*myData_p));
 *      solClient_msg_sendMsg(msg_p);
 *
 *      //  ....
 *      // session event callback handler for 
 *      // SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT
 *      
 *      struct myData *myData_p = (struct myData *) eventInfo_p->info_p;
 *      solClient_msg_free(&myData_p->msg_p);
 * @endcode
 *
 * Of course it is expected that your application will appropriately check the
 * ::solClient_returnCode_t for each C API call and handle exceptions.
 *
 */

#ifndef SOLCLIENTMSG_H
#define SOLCLIENTMSG_H

#include "solClient.h"

#if defined(__cplusplus)
extern "C"
{
#endif


/**
 * @enum solClient_cacheStatus
 *
 * Cache Status of received message, returned by solClient_msg_isCacheMsg()
 */
typedef enum solClient_cacheStatus
{
    SOLCLIENT_CACHE_INVALID_MESSAGE     = -1,
    SOLCLIENT_CACHE_LIVE_MESSAGE        = 0,
    SOLCLIENT_CACHE_MESSAGE             = 1,
    SOLCLIENT_CACHE_SUSPECT_MESSAGE     = 2
} solClient_cacheStatus_t;

/** @name Number of Message Size Quanta
* The number of message size quanta available.
* @see ::SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_0 through ::SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_4
*/
/*@{*/
#define SOLCLIENT_MSG_NUMDBQUANTA               (5)  /**< The number of message size quanta available */
/*@}*/

/**
* @enum solClient_msg_stats
* Statistics associated with message memory management.
* These statistics can be retrieved through ::solClient_msg_getStat().
*/
  typedef enum solClient_msg_stats
  {
    SOLCLIENT_MSG_STATS_TOTAL_MEMORY      =  0,  /**< Approximate amount of total memory consumed for message pool (allocated and on free list) */
    SOLCLIENT_MSG_STATS_ALLOC_MEMORY      =  1,  /**< Approximate amount of allocated memory (in-use; not on free list) */
    SOLCLIENT_MSG_STATS_MSG_ALLOCS        =  2,  /**< The number of message allocations performed -- calls to ::solClient_msg_alloc() */
    SOLCLIENT_MSG_STATS_MSG_FREES         =  3,  /**< The number of message frees performed -- calls to ::solClient_msg_free() */
    SOLCLIENT_MSG_STATS_MSG_DUPS          =  4,  /**< The number of message dups performed -- calls to ::solClient_msg_dup() */
    SOLCLIENT_MSG_STATS_MSG_REALLOCS      =  5,  /**< The number of message re-allocations performed (growth in a message buffer) */
    SOLCLIENT_MSG_STATS_FREE_MSGS         =  6,  /**< The number of messages on message free list */
    SOLCLIENT_MSG_STATS_ALLOC_MSGS        =  7,  /**< The number of currently allocated messages */
    SOLCLIENT_MSG_STATS_FREE_CONTAINERS   =  8,  /**< The number of containers (streams or maps) on container free list.*/
    SOLCLIENT_MSG_STATS_ALLOC_CONTAINERS  =  9,  /**< The number of currently allocated containers (for example, stream or map).*/
    SOLCLIENT_MSG_STATS_FREE_DATA_BLOCKS  = 10,  /**< The number of free data blocks (of specified quanta size from 0 to ::SOLCLIENT_MSG_NUMDBQUANTA - 1) */
    SOLCLIENT_MSG_STATS_ALLOC_DATA_BLOCKS = 11   /**< The number of allocated data blocks (of specified quanta size). Quanta ::SOLCLIENT_MSG_NUMDBQUANTA indicates messages currently allocated that are larger than maximum quanta size. */
  } solClient_msg_stats_t;                       /**< Type that indicates which message statistic. */

/**
 * @name solClient_msg_dumpExt mode flags
 *
 * Options to control the message dump utility.  
 */
#define SOLCLIENT_MSGDUMP_BRIEF                 (0)     /**< Display only the length of the binary attachment, XML attachment, and user property map. */
#define SOLCLIENT_MSGDUMP_FULL                  (1)     /**< Display the entire message.        */
/**
 * Allocate a solClient Msg that can be used for storing and sending 
 * messages to and from the Solace Messaging Appliance.
 * Applications are responsible for releasing all message buffers they allocate
 * by solClient_msg_alloc().
 *
 * @param  msg_p         A pointer to the memory that will hold the Msg pointer. 
 *                       This is an opaque value. Elements of the msg 
 *                       may only be accessed by the accessors defined in this 
 *                       API.
 * @returns              ::SOLCLIENT_OK on success. ::SOLCLIENT_FAIL on failure. 
 *                       msg_p is only valid after a ::SOLCLIENT_OK return.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_alloc( solClient_opaqueMsg_pt *msg_p);

/**
 * Free a solClient Msg previously allocated by solClient_msg_alloc().
 * Applications are responsible for releasing all message buffers they allocate by
 * solClient_msg_alloc() or solClient_msg_dup(). Message buffers received by callback
 * are owned by the API and <b>must not</b> be released. However the application may
 * take ownership of these message buffers as well by returning 
 * ::SOLCLIENT_CALLBACK_TAKE_MSG on return from the receive message callback function.
 * If the application returns ::SOLCLIENT_CALLBACK_TAKE_MSG, it <b>must</b> release
 * the message by calling solClient_msg_free() when it is finished with the message
 * buffer.
 * @param msg_p   A pointer to the msg_p previously allocated. On return
 *                      this reference is NULL and the memory previously referenced by
 *                      it is no longer valid.
 * @returns             ::SOLCLIENT_OK on success. ::SOLCLIENT_FAIL when msg_p does 
 *                      not point to a valid msg.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_free(solClient_opaqueMsg_pt *msg_p);

/** 
 * Duplicate a message buffer and allocate a new msg which references all the 
 * same data. For any data blocks, the reference count is incremented to 
 * indicate that two message buffers have pointers to the data.
 * Applications are responsible for releasing all message buffers they allocate by
 * solClient_msg_dup().
 * @param msg_p         A pointer to a Msg.  
 * @param dupMsg_p      A pointer to return a pointer to new msg.
 * @returns             ::SOLCLIENT_OK or ::SOLCLIENT_FAIL.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */

solClient_dllExport solClient_returnCode_t
solClient_msg_dup(solClient_opaqueMsg_pt msg_p,
                   solClient_opaqueMsg_pt *dupMsg_p); 

/** 
 * Release all memory associated with a message buffer. This function returns
 * a message buffer to its original state, as if it has just been allocated
 * by solClient_msg_alloc.
 *
 * @param msg_p   A pointer to a Msg.  
 * @returns       ::SOLCLIENT_OK or ::SOLCLIENT_FAIL.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_reset(solClient_opaqueMsg_pt msg_p);


/**
 * @section SolClient Msg accessors. 
 * The solClient Msg accessors provides a safe and managed 
 * interface for sending and receiving messages. The accessors allow 
 * application developers to set and read any part of the message.
 */

/**
 * Given a msg_p, retrieve the user property Map from binary metadata.
 * The returned map is a multimap, in which more than one value may be associated 
 * with a given field name. A call to <i>solClient_container_addXyz()</i> does not
 * overwrite an existing one, instead it adds a new field. To overwrite an existing
 * field, the field has to been deleted and then added with a new value. To get all
 * values associated with a given field name, a linear search is required.
 * The returned map should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the map
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(). If the map is closed automatically, the 
 * application cannot continue to use the map. Attempting to use a closed map
 * returns an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param map_p    A pointer to memory that contains a map pointer on return.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getUserPropertyMap(solClient_opaqueMsg_pt msg_p,
                            solClient_opaqueContainer_pt     *map_p);

/**
 * Given a msg_p, retrieve the contents of a binary attachment part 
 * as a stream.
 * The returned stream should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the stream
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(). If the stream is closed automatically, the 
 * application may not continue to use the stream. Attempting to use a closed stream
 * returns an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR)
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param stream_p pointer to memory that contains a stream pointer on return.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getBinaryAttachmentStream(solClient_opaqueMsg_pt msg_p,
                            solClient_opaqueContainer_pt     *stream_p);

/**
 * Given a msg_p, retrieve the contents of a binary attachment part as a map. 
 * The returned map is a multimap in which more than one value may be associated 
 * with a given field name. A call to <i>solClient_container_addXyz()</i> does not
 * overwrite an existing one, but adds a new one instead. To overwrite an existing
 * field, the field has to been deleted and then added with a new value. To get all
 * values associated with a given field name, a linear search is required.
 * The returned map should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the map
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(). If the map is closed automatically, the 
 * application may not continue to use the map. Attempting to use a closed map
 * will return an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR)
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param map_p    A pointer to memory that contains a map pointer on return;
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getBinaryAttachmentMap(solClient_opaqueMsg_pt msg_p,
                            solClient_opaqueContainer_pt     *map_p);

/**
 * Given a msg_p, retrieve the contents of a binary attachment part and 
 * return the pointer and length.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                       to solClient_msg_alloc() or received data callback.
 * @param bufPtr_p       A pointer to the application pointer to fill in with the 
 *                       message data pointer on return. The programmer may cast
 *                       the returned void pointer to any reference suitable for 
 *                       the application.
 * @param size_p         A pointer to memory that contains data size on 
 *                       return.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */

solClient_dllExport solClient_returnCode_t 
solClient_msg_getBinaryAttachmentPtr(solClient_opaqueMsg_pt msg_p,
                             solClient_opaquePointer_pt bufPtr_p,
                             solClient_uint32_t         *size_p);

/**
 * Given a msg_p, retrieve the contents of a binary attachment part if it is
 * a JMS string and return a pointer to the string (NULL-terminated string).
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                       to solClient_msg_alloc() or received data callback.
 * @param bufPtr_p       A pointer to memory that contains the string pointer
 *                       on return.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getBinaryAttachmentString(solClient_opaqueMsg_pt msg_p,
                                        const char *          *bufPtr_p);

/**
 * Given a msg_p, retrieve the size of the open container in the binary 
 * attachment. If there is no open container in the binary attachment, 
 * this function returns ::SOLCLIENT_NOT_FOUND.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                       to solClient_msg_alloc() or received data callback.
 * @param size_p   A pointer to memory that contains the size on return.
 * @returns        ::SOLCLIENT_OK or ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getBinaryAttachmentContainerSize(solClient_opaqueMsg_pt msg_p,
                                        size_t                       *size_p);



/**
 * Given a msg_p, retrieve the contents of a Correlation Tag part (used by 
 * an application when sending Guaranteed messages and receiving Guaranteed message
 * acknowledgments) and return the pointer and length.
 *
 * @param msg_p          solClient_opaqueMsg_pt that is returned from a previous call
 *                       to solClient_msg_alloc() or received in a receive
 *                       message callback.
 * @param bufPtr_p       A pointer to the application pointer to fill in with the 
 *                       message correlation tag pointer on return. The programmer may cast
 *                       the returned void pointer to any reference suitable for 
 *                       the application.
 * @param size_p         A pointer to memory that contains data size on 
 *                       return.
 * @see @ref adConsiderations
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getCorrelationTagPtr(solClient_opaqueMsg_pt msg_p,
                             solClient_opaquePointer_pt   bufPtr_p,
                             solClient_uint32_t          *size_p);
/**
 * Given a msg_p, retrieve the contents of a User Data part. 
 *
 * The maximum size allowed for the user data part is ::SOLCLIENT_BUFINFO_MAX_USER_DATA_SIZE.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param bufPtr_p       A pointer to the application pointer to fill in with the 
 *                       message user data pointer on return. The programmer may cast
 *                       the returned void pointer to any reference suitable for 
 *                       the application.
 * @param size_p         A pointer to memory that contains data size on 
 *                       return.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getUserDataPtr(solClient_opaqueMsg_pt       msg_p,
                             solClient_opaquePointer_pt   bufPtr_p,
                             solClient_uint32_t          *size_p);
/**
 * Given a msg_p, retrieve the contents of a XML part of the message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param bufPtr_p       A pointer to the application pointer to fill in with the 
 *                       message XML data pointer on return. The programmer may cast
 *                       the returned void pointer to any reference suitable for 
 *                       the application.
 * @param size_p         A pointer to memory that contains data size on 
 *                       return.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getXmlPtr(     solClient_opaqueMsg_pt      msg_p,
                             solClient_opaquePointer_pt  bufPtr_p,
                             solClient_uint32_t         *size_p);

/**
 * Given a msg_p, retrieve the raw Solace Message Format (SMF) message as originally
 * received.
 *
 * @param msg_p    solClient_opaqueMsg_pt received in a receive message 
 *                 callback.
 * @param bufPtr_p       A pointer to the application pointer to fill in with the 
 *                       message SMF pointer on return. SMF data is binary 8 byte data
 *                       and can be stored by copying 'size' bytes from the returned
 *                       pointer.
 * @param size_p         A pointer to memory that contains data size on 
 *                       return.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getSMFPtr(solClient_opaqueMsg_pt msg_p,
                             solClient_uint8_t         **bufPtr_p,
                             solClient_uint32_t         *size_p);
#define solClient_msg_getSmfPtr solClient_msg_getSMFPtr

/**
 * @section SolClient Msg mutators. 
 * The solClient Msg mutators are intended to provide a safe and managed 
 * interface to the Solace Message Format (SMF) message used for sending and receiving messages.
 */


/**
 * Given a msg_p, set the contents of a Binary Attachment Part to the given 
 * pointer and size. This memory may contain the pre-built payload the
 * application is sending, or an application may choose to subsequently build a 
 * structured message (a map or stream container) in the Binary Attachment
 * part and the buffer set here is used to construct the container up
 * to the size. If the structured message grows to exceed the set size, 
 * an internal data block is used to hold the entire container.
 *
 * <b>NOTE:</b> When a Guaranteed message is constructed with this API 
 * refer to @ref adConsiderations.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p          A pointer to buffer.
 * @param size           The maximum number of bytes in buffer.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setBinaryAttachmentPtr(solClient_opaqueMsg_pt msg_p,
                             void *                            buf_p,
                             solClient_uint32_t                size);

/**
 * Given a msg_p, set the contents of the binary attachment part by copying in from
 * the given pointer and size. This causes memory to be allocated from 
 * API internal or heap storage. If any binary attachment previously existed it will
 * be first removed before the new data is copied in.
 *
 * Passing in a buf_p of NULL and a size of zero results in 
 * a binary attachment not being present in the message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p          A pointer to buffer.
 * @param size           The maximum number of bytes in buffer.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid
 *                       or memory not available.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setBinaryAttachment(solClient_opaqueMsg_pt msg_p,
                             const void *                      buf_p,
                             solClient_uint32_t                size);

/**
 * Given a msg_p, set the contents of the binary attachment part to a UTF-8 or ASCII string
 * by copying in from the given pointer until null-terminated. The message
 * will be TLV-encoded suitable for reading by any other Solace Corporation Messaging APIs.
 * If any binary attachment previously existed it is first
 * removed before the new data is copied in.
 *
 * Passing in a buf_p of NULL results in 
 * a binary attachment not being present in the message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p          A pointer to a buffer containing a UTF-8 or ASCII string.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid
 *                       or memory is not available.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setBinaryAttachmentString(solClient_opaqueMsg_pt msg_p,
                             const char *                      buf_p);

/**
 * Given a msg_p, set the contents of the binary attachment part to a Map or Stream, as 
 * referenced by the given solClient_opaqueContainer_pt. This function 
 * copies in from the given container. Changes to the container after this function is called 
 * will not be propagated to this message. The message
 * will be TLV encoded suitable for reading by any other Solace Messaging API.
 * If any binary attachment previously existed it is first
 * removed before the new data is copied in.
 *
 * <b>WARNING:</b> This method is intended to be used to add an existing
 * message independent container to a message. That is, a container created by
 * either ::solClient_container_createMap() or ::solClient_container_createStream(). 
 * Do <b>not</b> call this method with a container that is already in the message,
 * returned from a call to ::solClient_msg_createBinaryAttachmentMap or 
 * ::solClient_msg_createBinaryAttachmentStream. That is unnecessary as the container
 * is already in the message. Further, the first operation of 
 * ::solClient_msg_setBinaryAttachmentContainer() is to wipe out the existing binary
 * attachment.
 *
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param cont_p   An opaque container pointer for the container to add.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid
 *                       or memory is not available.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setBinaryAttachmentContainer(solClient_opaqueMsg_pt msg_p,
                             solClient_opaqueContainer_pt   cont_p);

/**
 * Given a msg_p, set the contents of the binary attachment part to a Map or Stream, as 
 * referenced by the given solClient_opaqueContainer_pt. This function 
 * takes a reference to the given container. This is a high performance interface function that avoids a memory 
 * copy from application memory into the message binary attachment. On a subsequent ::solClient_session_sendMsg() the 
 * binary attachment contents will be copied directly from the application memory to the transmit socket or buffer. 
 *
 * <b>NOTE:</b> When a Guaranteed message is constructed with this API 
 * refer to @ref adConsiderations
 *
 * In addition to the above note, the application <b>must not</b> change the container after this function is called until 
 * the message is sent, even when the message contains a non-Guaranteed message. Changing the container, or releasing the memory referenced 
 * by the container, will cause the message contents to be a corrupt container when transmitted. If the container is added to a Guaranteed 
 * Delivery message by reference then the container and container memory must not be modified until the appliance acknowledgment for the 
 * message is received. This latter further restriction for Guaranteed Delivery is required in case the C API library
 * needs to retransmit the original message for any reason. In general, for Guaranteed Delivery, it is far safer to use
 * ::solClient_msg_setBinaryAttachmentContainer. It is unlikely the application will achieve any performance gains by
 * this function with Guaranteed messages.
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param cont_p   An opaque container pointer for the container to add.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid
 *                       or memory is not available.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setBinaryAttachmentContainerPtr(solClient_opaqueMsg_pt msg_p,
                             solClient_opaqueContainer_pt   cont_p);
/**
 * Given a msg_p, set the contents of a user data part to the given 
 * pointer and size. 
 * This function is provided for high-performance applications that 
 * must be aware that the data referenced cannot be modified until the send 
 * operation completes.
 *
 * The maximum size allowed for the user data part is ::SOLCLIENT_BUFINFO_MAX_USER_DATA_SIZE.
 *
 * <b>NOTE:</b> When a Guaranteed message is constructed with this API 
 * refer to @ref adConsiderations
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p    A pointer to buffer.
 * @param size     The number of bytes in the buffer.
 * @returns        ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setUserDataPtr(solClient_opaqueMsg_pt msg_p,
                             void *                 buf_p,
                             solClient_uint32_t     size);

/**
 * Given a msg_p, set the contents of User Data Part by copying in from
 * the given pointer and size. This causes memory to be allocated from internal
 * or heap storage. If any user data previously existed it is first
 * removed before the new data is copied in.
 *
 * The maximum size allowed for the user data part is ::SOLCLIENT_BUFINFO_MAX_USER_DATA_SIZE.
 *
 * Passing in a buf_p of NULL and a size of zero results in 
 * user data not being present in the message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p    A pointer to buffer.
 * @param size     The number of bytes in buffer.
 * @returns        ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid
 *                 or memory not available.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setUserData(solClient_opaqueMsg_pt msg_p,
                             const void *                      buf_p,
                             solClient_uint32_t                size);
/**
 * Given a msg_p, set the contents of the XML part to the given 
 * pointer and size.
 * This function is provided for high-performance applications that 
 * must be aware that the data referenced cannot be modified until the send 
 * operation completes.
 *
 * <b>NOTE:</b> When a Guaranteed message is constructed with this API 
 * refer to @ref adConsiderations.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p    A pointer to buffer.
 * @param size     The number of bytes in buffer.
 * @returns        ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setXmlPtr(solClient_opaqueMsg_pt msg_p,
                             void *            buf_p,
                             solClient_uint32_t size);

/**
 * Given a msg_p, set the contents of the XML part by copying in from
 * the given pointer and size. This causes memory to be allocated from internal
 * or heap storage. If any XML part previously existed, it is first
 * removed before the new data is copied in.
 *
 * Passing in a buf_p of NULL and a size of zero results in 
 * the XML part not being present in the message.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p    A pointer to buffer.
 * @param size     The number of bytes in buffer.
 * @returns        ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid
 *                 or memory not available.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setXml(solClient_opaqueMsg_pt msg_p,
                             const void *                      buf_p,
                             solClient_uint32_t                size);
/**
 * Given a msg_p, set the Correlation Tag to the given pointer. The Correlation Tag is a
 * local reference used by applications generating Guaranteed messages. Messages that are 
 * sent in either PERSISTENT or non-PERSISTENT mode can set the Correlation Tag,
 * which is returned when the ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT event
 * is later received. The solClient_session_eventCallbackInfo structured returned with the
 * event contains a (void *) correlation_p which will be the same pointer the application
 * initializes with this method.  
 * Important: <b>The Correlation Tag is not included in the 
 * transmitted message and is only used with the local API</b>.
 *
 * This function is provided for high-performance applications that 
 * must be aware that the data referenced cannot be modified until the send 
 * operation completes.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param correlation_p    A pointer to buffer.
 * @param size     Ignored.
 * @see @ref adConsiderations
 * @returns        ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setCorrelationTagPtr(solClient_opaqueMsg_pt msg_p,
                             void *                      correlation_p,
                             solClient_uint32_t          size);

/**
 * Given a msg_p, save correlation information Part by copying in from
 * the given pointer and size. This is not recommended for high performance applications,
 * use solClient_msg_setCorrelationTagPtr instead. The Correlation Tag is a local reference
 * used by applications generating Guaranteed messages. Messages that are 
 * sent in either PERSISTENT or non-PERSISTENT mode may set the Correlation Tag. If this 
 * method is used, a pointer to the correlation information is returned
 * when the ::SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT event
 * is later received. The solClient_session_eventCallbackInfo structured returned with the
 * event contains a (void *) correlation_p which points to a copy of the information
 * initialized with this method. The size is <b>not</b> returned. 
 * Important: <b>The Correlation Tag is not included in the 
 * transmitted message and is only used with the local API</b>.
 *
 * This causes memory to be allocated from internal
 * or heap storage.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param correlation_p  A pointer to the correlation information.
 * @param size     The number of bytes in buffer.
 * @see @ref adConsiderations
 * @returns        ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid
 *                 or memory not available.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setCorrelationTag(solClient_opaqueMsg_pt msg_p,
                             const void *              correlation_p,
                             solClient_uint32_t        size);
/**
 * Given a msg_p, set the Topic of the message as a pointer to application 
 * space. This function is provided for high-performance applications that 
 * must be aware that the data referenced cannot be modified until the send 
 * operation completes. 
 *
 * <b>NOTE:</b> When a Guaranteed message is constructed with this API 
 * refer to @ref adConsiderations.
 *
 * Only solClient_msg_setTopicPtr is provided, for copy-in operations use 
 * solClient_msg_setDestination.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param topic_p  A pointer to topic string.
 * @returns        ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setTopicPtr(solClient_opaqueMsg_pt msg_p,
                             const char *        topic_p);

/**
 * Given a msg_p, set the QueueName destination of the message as a pointer to
 * application space. 
 * This function is provided for high-performance applications that must be
 * aware that the pointer data cannot be modified until the send operation
 * completes. 
 *
 * <b>NOTE:</b> When a Guaranteed message is constructed with this API 
 * refer to @ref adConsiderations.
 *
 * Only solClient_msg_setQueueNamePtr is provided, for copy-in 
 * operations use solClient_msg_setDestination.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param queueName_p    A pointer to topic string.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL if msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setQueueNamePtr(solClient_opaqueMsg_pt msg_p,
                              const char            *queueName_p);


/**
 * Given a msg_p, retrieve the replyTo destination and set the information
 * in the passed in destination structure. On return dest_p->dest points to
 * message memory and is only valid as long as msg_p is valid.
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param dest_p         A pointer to destination structure to receive ReplyTo.
 * @param destSize       The size of (solClient_destination_t).
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getReplyTo(solClient_opaqueMsg_pt msg_p,
                        solClient_destination_t     *dest_p,
                        size_t                       destSize);

/**
 * Given a msg_p, set the ReplyTo destination.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param dest_p         A pointer to ReplyTo destination to set.
 * @param destSize       The size of (solClient_destination_t).
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setReplyTo(solClient_opaqueMsg_pt msg_p,
                        const solClient_destination_t    *dest_p,
                        size_t                            destSize);


/** 
 * Given a msg_p, delete the ReplyTo destination.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteReplyTo(solClient_opaqueMsg_pt msg_p);

/** 
 * Given a msg_p, set the Destination field (queue or topic).
 * A destination can be removed from a message
 * by setting the ::solClient_destination_t structure to 
 * {::SOLCLIENT_NULL_DESTINATION, NULL}.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param dest_p         A pointer to destination information.
 * @param destSize      The size of (solClient_destination_t).
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setDestination(solClient_opaqueMsg_pt msg_p,
                        solClient_destination_t    *dest_p,
                        size_t                      destSize);

/** 
 * Given a msg_p, get the Destination field (queue or topic), which is the
 * destination this message was published to. On successful 
 * return dest_p->dest points to message memory and is only valid as 
 * long as msg_p is valid. 
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param dest_p         A pointer to destination information.
 * @param destSize       The size of destination_t structure.
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_FOUND
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getDestination(solClient_opaqueMsg_pt msg_p,
                        solClient_destination_t    *dest_p,
                        size_t                      destSize);

/**
 * Given a msg_p, copy the SenderID pointer into the given buffer.
 *
 * This method allows the application to retrieve a pointer to the string that 
 * is the SenderID.  The SenderID is user-defined,
 * carried end-to-end, and can also be matched in a selector, but otherwise is not relevant
 * to the event broker.  
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p          A pointer to string pointer for senderId.
 * @returns              ::SOLCLIENT_OK on success,  ::SOLCLIENT_FAIL if msg 
 *                       is invalid, ::SOLCLIENT_NOT_FOUND if msg contains no SenderId field.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getSenderId(solClient_opaqueMsg_pt msg_p,
                          const char *          *buf_p);

/**
 * Given a msg_p, set the SenderID.  
 *
 * This overrides ::SOLCLIENT_SESSION_PROP_GENERATE_SENDER_ID 
 * session property and forces the specified SenderID
 * into the binary message header.
 *
 * This method allows the application to set the contents of the 
 * SenderID.  The SenderID is user-defined, carried end-to-end, and
 * can also be matched in a selector but otherwise is not relevant
 * to the event broker.  
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buf_p          A pointer to string for the data copy.
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_setSenderId(solClient_opaqueMsg_pt msg_p,
                          const char *           buf_p);

/** 
 * Given a msg_p, delete the SenderId.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteSenderId(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, copy the ApplicationMessageType topic pointer into the given pointer.
 * 
 * This method allows the application to retrieve a pointer to the string that 
 * is the ApplicationMessageType.  The ApplicationMessageType is user-defined,
 * carried end-to-end, and can also be matched in a selector, but otherwise is not relevant
 * to the event broker.  
 * 
 * In JMS applications, this field is carried as the JMSType Message Header Field.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param msgType_p      A pointer to string pointer to receive msgType pointer.
 * @returns              ::SOLCLIENT_OK on success,  ::SOLCLIENT_FAIL if msg 
 *                       is invalid, ::SOLCLIENT_NOT_FOUND if it contains no msgType field.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getApplicationMsgType(solClient_opaqueMsg_pt msg_p,
                                    const char *          *msgType_p);

/**
 * Given a msg_p, set the ApplicationMessageType field.
 *
 * This method allows the application to set the string contents of the 
 * ApplicationMessageType.  The ApplicationMessageType is user defined,
 * carried end-to-end, and can slso be matched in a selector, but otherwise is not relevant
 * to the event broker.  
 * 
 * In JMS applications, this field is carried as the JMSType Message Header Field.
 *
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param msgType        A pointer to string with msgType.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg or length is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setApplicationMsgType(solClient_opaqueMsg_pt msg_p,
                                    const char *           msgType);

/** 
 * Given a msg_p, delete the MsgType field.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteApplicationMsgType(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, copy the ApplicationMessageId pointer into the given buffer.
 *
 * This method allows the application to retrieve a pointer to the string that 
 * is the ApplicationMessageId.  The ApplicationMessageId is user-defined,
 * carried end-to-end, and can also be matched in a selector, but otherwise is not relevant
 * to the event broker.  
 * 
 * In JMS applications, this field is carried as the JMSMessageID Message Header Field.
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param messageId_p    A pointer to string pointer to receive MessageId pointer.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid, ::SOLCLIENT_NOT_FOUND for none found
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getApplicationMessageId(solClient_opaqueMsg_pt msg_p,
                                      const char *          *messageId_p);

/**
 * Given a msg_p, set the ApplicationMessageId field.
 *
 * This method allows the application to set the string contents of the 
 * ApplicationMessageId.  The ApplicationMessageId is user defined,
 * carried end-to-end, and can slso be matched in a selector, but otherwise is not relevant
 * to the event broker.  
 * 
 * In JMS applications, this field is carried as the JMSMessageId Message Header Field.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param messageId_p    pointer to string containing messageId.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setApplicationMessageId(solClient_opaqueMsg_pt msg_p,
                                      const char *           messageId_p);

/** 
 * Given a msg_p, delete the Application MessageId field.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteApplicationMessageId(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, copy the SequenceNo into the given buffer.
 * A sequence number is automatically included (if not already present) in 
 * the Solace-defined fields for each message sent if the session property
 * ::SOLCLIENT_SESSION_PROP_GENERATE_SEQUENCE_NUMBER is enabled.
 *
 * This method allows the application to retrieve the value that 
 * is the SequenceNo.  The SequenceNo is user-defined,
 * carried end-to-end, and can also be matched in a selector, but otherwise is not relevant
 * to the event broker.  
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param seqNum_p       A pointer to 64-bit field to receive the value.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid, ::SOLCLIENT_NOT_FOUND if not found.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getSequenceNumber(solClient_opaqueMsg_pt msg_p,
                               solClient_int64_t      *seqNum_p);

/**
 * Given a msg_p, set the Sequence Number field.
 * This overrides the ::SOLCLIENT_SESSION_PROP_GENERATE_SEQUENCE_NUMBER
 * session property and forces the specified Sequence Number
 * into the binary message header. This does <b>not</b> change the 
 * internal sequence numbering and the next generated sequence number will 
 * still be one more than the last generated sequence number.
 *
 * A sequence number is automatically included (if not already present) in 
 * the Solace-defined fields for each message sent if the session property
 * ::SOLCLIENT_SESSION_PROP_GENERATE_SEQUENCE_NUMBER is enabled.
 *
 * This method allows the application to set the value of the 
 * SequenceNo.  The SequenceNo is user defined,
 * carried end-to-end, and can slso be matched in a selector, but otherwise is not relevant
 * to the event broker.  
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param seqNum         The 64-bit Sequence Number.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setSequenceNumber(solClient_opaqueMsg_pt msg_p,
                               solClient_uint64_t      seqNum);

/** 
 * Given a msg_p, delete the Sequence Number field.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteSequenceNumber(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, copy the CorrelationId pointer into the given buffer.
 *
 * This method allows the application to retrieve a pointer to the string that 
 * is the CorrelationId.  The CorrelationId is user-defined,
 * carried end-to-end, and can also be matched in a selector, but otherwise is not relevant
 * to the event broker.  The CorrelationId may be used for peer-to-peer message synchronization.
 * 
 * In JMS applications, this field is carried as the JMSCorrelationID Message Header Field.
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param correlation_p  A pointer to string pointer to receive correlation 
 *                       Id pointer.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_NOT_FOUND if the field is 
 *                       not present and ::SOLCLIENT_FAIL if  msg_p is invalid
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getCorrelationId(solClient_opaqueMsg_pt msg_p,
                               const char *          *correlation_p);

/**
 * Given a msg_p, set the CorrelationId field.
 *
 * The CorrelationId is user-defined, carried end-to-end, and can also be matched
 * in a selector, but otherwise is not relevant
 * to the event broker.  The CorrelationId may be used for peer-to-peer message synchronization.
 *
 * In JMS applications, this field is carried as the JMSCorrelationID Message Header Field.
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param correlation_p  A pointer to string to copy into correlationId.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setCorrelationId(solClient_opaqueMsg_pt msg_p,
                               const char *           correlation_p);

/** 
 * Given a msg_p, delete the Correlation Id field. 
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteCorrelationId(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, copy the Receive Timestamp into the given buffer.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param timestamp_p    A pointer to a 64-bit field to receive the value.
 *                       The value is in milliseconds.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid, ::SOLCLIENT_NOT_FOUND if not found.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getRcvTimestamp(solClient_opaqueMsg_pt msg_p,
                              solClient_int64_t        *timestamp_p);

/**
 * Given a msg_p, set the Sender Timestamp field.
 * This overrides the ::SOLCLIENT_SESSION_PROP_GENERATE_SEND_TIMESTAMPS
 * session property and sets the specified Sender Timestamp value
 * in the binary message header.  
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param timestamp  The sender timestamp value to set. The value is in milliseconds.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setSenderTimestamp(solClient_opaqueMsg_pt msg_p,
                              solClient_int64_t         timestamp);

/**
 * Given a msg_p, copy the SenderTimestamp into the given buffer.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param timestamp_p    A pointer to a 64-bit field to receive the value.
 *                       The value is in milliseconds.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid, ::SOLCLIENT_NOT_FOUND for none found.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getSenderTimestamp(solClient_opaqueMsg_pt msg_p,
                              solClient_int64_t        *timestamp_p);

/** 
 * Given a msg_p, delete the SenderTimestamp.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteSenderTimestamp(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, set the expiration time field. The expiration time is the UTC time 
 * (that is, the number of milliseconds from midnight January 1, 1970 UTC) when the
 * message is to expire. The expiration time is carried in the message when set to
 * a non-zero value. Expiration time is not included when this value is set to zero.
 * 
 * The message expiration time is carried to clients that receive the message
 * unmodified and does not effect the life cycle of the message. Use
 * solClient_msg_setTimeToLive() to enforce message expiry in the network.  
 * In fact when solClient_msg_setTimeToLive() is used, setting this property has no effect. 
 * When solClient_msg_setTimeToLive() is called, the expiration time is never carried
 * in the message, however it may be calculated and retrieved by the sender if the session property
 * ::SOLCLIENT_SESSION_PROP_CALCULATE_MESSAGE_EXPIRATION is enabled. Thus if
 * ::solClient_msg_getExpiration() is called after the message is sent, a calculated
 * expiration time is returned based on the time-to-live.
 *
 * <b>Note:</b> When solClient_msg_setTimeToLive() is set on a message, the receiving 
 * client may also calculate the expiration time if it has enabled the session
 * property ::SOLCLIENT_SESSION_PROP_CALCULATE_MESSAGE_EXPIRATION.
 *
 * See solClient_msg_getExpiration() for more details.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param timestamp  The sender timestamp value to set. The value is in milliseconds.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setExpiration(solClient_opaqueMsg_pt    msg_p,
                            solClient_int64_t         timestamp);

/**
 * Given a msg_p, copy the Message Expiration timestamp into the given buffer. If
 * message expiration time is not set in the message and the session property 
 * ::SOLCLIENT_SESSION_PROP_CALCULATE_MESSAGE_EXPIRATION is enabled, the expiration time
 * is calculated based on the message Time To Live. When enabled, the expiration time 
 * for sent messages will be the UTC time when the message is sent plus the Time To Live. The
 * expiration time for received messages is the UTC time when the message was received
 * plus the Time To Live in the message at the time it was received.
 *
 * If the expiration time is not set in the message, and it cannot be calculated, the
 * timestamp is set to zero.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param timestamp_p    A pointer to a 64-bit field to receive the value.
 *                       The value is in milliseconds.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getExpiration(solClient_opaqueMsg_pt    msg_p,
                            solClient_int64_t        *timestamp_p);

/**
 * Given a msg_p, get the Class of Service from a message.
 * The  Class of Service has different semantics for direct and guaranteed messages.
 *
 * For direct messages, the class of service selects the weighted round-robin delivery
 * queue when the message is forwarded to a consumer.  {::SOLCLIENT_COS_1} are the
 * lowest priority messages and will use the solace message-router D-1 delivery queues.
 *
 * For messages published as guaranteed messages, * messages published
 * with ::SOLCLIENT_COS_1 can be rejected by the solace message-router if
 * that message would cause any queue or topic-endpoint to exceed its configured
 * <i>low-priority-max-msg-count</i>.
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param cos_p         A place to store the returned class of service, one of 
 *                      @li ::SOLCLIENT_COS_1
 *                      @li ::SOLCLIENT_COS_2
 *                      @li ::SOLCLIENT_COS_3
 * @returns             ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                      msg_p or cos_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getClassOfService(solClient_opaqueMsg_pt msg_p,
                                solClient_uint32_t     *cos_p);

/**
 * Given a msg_p, set the Class of Service to use for transmission.  
 * The  Class of Service has different semantics for direct and guaranteed messages.
 *
 * The  Class of Service has different semantics for direct and guaranteed messages.
 * For direct messages, the class of service selects the weighted round-robin delivery
 * queue when the message is forwarded to a consumer.  {::SOLCLIENT_COS_1} are the
 * lowest priority messages and will use the solace message-router D-1 delivery queues.
 *
 * For messages published as guaranteed messages, * messages published
 * with ::SOLCLIENT_COS_1 can be rejected by the solace message-router if
 * that message would cause any queue or topic-endpoint to exceed its configured
 * <i>low-priority-max-msg-count</i>.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param cos            The class of service, one of ::SOLCLIENT_COS_1,
 *                       ::SOLCLIENT_COS_2, or ::SOLCLIENT_COS_3.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setClassOfService(solClient_opaqueMsg_pt msg_p,
                               solClient_uint32_t      cos);

/**
 * Given a msg_p, get the Time To Live (TTL) from a message. If the message does not
 * contain a time to live field, zero is returned.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param ttl_p    A pointer to a 64-bit field to receive the value.
 *                 The value is in milliseconds.
 * @returns        ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                      msg_p or timestamp_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getTimeToLive  (  solClient_opaqueMsg_pt msg_p,
                                solClient_int64_t        *ttl_p);

/**
 * Given a msg_p, set the Time To Live (TTL) for a message. Setting the Time To Live to
 * zero disables TTL for the message.
 *
 * This property is only valid for Guaranteed messages (Persistent and Non-Persistent). 
 * It has no effect when used in conjunction with other message types unless the message 
 * is promoted by the appliance to a Guaranteed message. 
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param ttl      64-bit value in milliseconds to use for message time to live.
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setTimeToLive (   solClient_opaqueMsg_pt msg_p,
                                solClient_int64_t      ttl);

/**
 * Given a msg_p, return the delivery mode.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param mode_p         A place to store the returned delivery mode, one of 
 *                       @li ::SOLCLIENT_DELIVERY_MODE_DIRECT
 *                       @li ::SOLCLIENT_DELIVERY_MODE_PERSISTENT
 *                       @li ::SOLCLIENT_DELIVERY_MODE_NONPERSISTENT
 * @see @ref adConsiderations
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getDeliveryMode(solClient_opaqueMsg_pt msg_p,
                               solClient_uint32_t       *mode_p);

/**
 * Given a msg_p, set the delivery mode.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param mode           The delivery mode to use for this message. It can be one of the following:
 *                       @li ::SOLCLIENT_DELIVERY_MODE_DIRECT
 *                       @li ::SOLCLIENT_DELIVERY_MODE_PERSISTENT
 *                       @li ::SOLCLIENT_DELIVERY_MODE_NONPERSISTENT
 * @see @ref adConsiderations
 * @returns              ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setDeliveryMode(solClient_opaqueMsg_pt msg_p,
                               solClient_uint32_t    mode);

/**
 * Given a msg_p, return the Guaranteed message Id. The guaranteed message Id only
 * exists in messages received on a flow.  The message Id is only to be used for the 
 * purpose of acknowledgements.  No other meaning should be inferred from the value 
 * of the message Id.  
 *
 * Messages are acknowledged by calling ::solClient_flow_sendAck()
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param msgId_p  pointer to memory to store the returned msgId.
 * @returns        ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                 msg_p is invalid, ::SOLCLIENT_NOT_FOUND if msg_p does not contain an 
 *                 assured delivery message.
 * @see @ref adConsiderations
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getMsgId(solClient_opaqueMsg_pt   msg_p,
                       solClient_msgId_t       *msgId_p);

/**
 * Given a msg_p, return the Topic Sequence Number.  If there is no topic sequence number
 * SOLCLIENT_NOT_FOUND is returned and the sequence number returned is zero.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param seqNum_p pointer to memory to store the returned topic sequence number.
 * @returns        ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                 msg_p is invalid, ::SOLCLIENT_NOT_FOUND if msg_p does not contain
 *                 a topic sequence number.
 * @see @ref adConsiderations
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getTopicSequenceNumber(solClient_opaqueMsg_pt   msg_p,
                                     solClient_int64_t       *seqNum_p);

/**
 * Given a msg_p, return the delivery count. If delivery is supported returns 
 * SOLCLIENT_OK otherwise returns SOLCLIENT_FAIL. 
 * Note messages from browser flows will have the delivery of the next
 * consumer delivery not the current message delivery count from the endpoint.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param count_p  pointer to memory to store the returned delivery count.
 * @returns        ::SOLCLIENT_OK if delivery count is supported, 
 *                 ::SOLCLIENT_FAIL if delivery count is not supported, or 
 *                 if msg_p or count_p are invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getDeliveryCount(solClient_opaqueMsg_pt msg_p, 
                               solClient_int32_t * count_p);

/**
 * Given a msg_p, test the redeliver status.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *
 * @see @ref adConsiderations
 * @returns        True, if the message was redelivered.
 */
solClient_dllExport solClient_bool_t
solClient_msg_isRedelivered(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, return the data source (live or cached message).
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *
 * @returns        cacheStatus, one of:
 *                 @li ::SOLCLIENT_CACHE_LIVE_MESSAGE
 *                 @li ::SOLCLIENT_CACHE_MESSAGE
 *                 @li ::SOLCLIENT_CACHE_SUSPECT_MESSAGE
 */
solClient_dllExport solClient_cacheStatus_t
solClient_msg_isCacheMsg(solClient_opaqueMsg_pt msg_p);

/** 
 *  Given a msg_p containing a cached message, return the cache RequestId that
 *  the application set in the call to solClient_csession_sendCacheRequest().
 *
 * @param msg_p            solClient_opaquemsg_pt received in a receive message callback.
 * @param cacheRequestId_p pointer to location to receive the cacheRequestId.
 *
 * @returns  ::SOLCLIENT_OK, ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND if msg_p does not contain a received
 * cache message.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getCacheRequestId(solClient_opaqueMsg_pt msg_p,
                                solClient_uint64_t    *cacheRequestId_p);

/**
 * Given a msg_p, test the discard indication status.
 * Returns true if one or more messages have been discarded prior to the current 
 * message, otherwise it returns false. 
 * This indicates congestion discards only, and is not affected by message eliding.
 * 
 * @see ::solClient_msg_isElidingEligible for information about message eliding.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *
 * @returns        True, if the message had the discard indication set.
 */
solClient_dllExport solClient_bool_t
solClient_msg_isDiscardIndication(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, test the response attribute.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *
 * @returns        True, if the message is a response to a solClient_session_sendRequest().
 */
solClient_dllExport solClient_bool_t
solClient_msg_isReplyMsg(solClient_opaqueMsg_pt msg_p);

/**
 * Given a msg_p, set the Dead Message Queue (DMQ) eligible property on a message. When this
 * option is set, messages that expire in the network, are saved on a appliance dead message 
 * queue. Otherwise expired messages are discarded.
 * By default the property is set to false on newly created messages.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param val      0 - clear, 1 - set.
 * @returns        ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if
 *                       msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setDMQEligible(solClient_opaqueMsg_pt msg_p,
                             solClient_bool_t      val);

/**
 * Given a msg_p, test the Dead Message Queue (DMQ) eligible property.
 *
 * @param msg_p    solClient_opaqueMsg_pt returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *
 * @returns        True, if the message has "Dead Message Queue eligible" property set.
 */
solClient_dllExport solClient_bool_t
solClient_msg_isDMQEligible(solClient_opaqueMsg_pt msg_p);

/**
 * Append an application-specified suffix to the default topic destination for the Session.
 * When the session is established, a unique topic destination is created for the Session.
 * This topic is generated by the appliance, and the API subscribes to a wildcard extended
 * topic that is derived by appending '/>' to the unique topic destination. By default, 
 * ::solClient_session_sendRequest automatically  adds a replyTo destination derived by
 * appending '/#' to the unique topic destination. 
 *
 * When this function is used, a delimiter ('/') and the supplied string are appended instead.
 *
 * @param msg_p          A solClient_opaqueMsg_pt that is returned from a previous call
 *                       to solClient_msg_alloc() or received in a receive
 *                       message callback.
 * @param opaqueSession_p pointer to the Session the message is later used with.
 * @param suffix_p       A pointer to a NULL-terminated suffix string.
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_setReplyToSuffix(solClient_opaqueMsg_pt msg_p,
                               solClient_opaqueSession_pt opaqueSession_p,
                               const char *           suffix_p);

/**
 * This utility returns the suffix of the topic string. If the received message contains a 
 * topic destination, and the topic begins with the session topic prefix, this utility
 * returns a pointer to the string following the '/' delimiter that terminates the topic
 * prefix.
 *
 * @param msg_p          A solClient_opaqueMsg_pt that is returned from a previous call
 *                       to solClient_msg_alloc() or received in a receive
 *                       message callback.
 * @param opaqueSession_p A pointer to the Session the message was received on.
 * @param suffix_p       A pointer to pointer location that contains pointer to a NULL-
 *                       terminated suffix string on success.
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL, ::SOLCLIENT_NOT_FOUND.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getDestinationTopicSuffix(solClient_opaqueMsg_pt msg_p,
                                     solClient_opaqueSession_pt opaqueSession_p,
                                     const char *              *suffix_p);

/**
 * Set the reply attribute of the message. When this message is later transmitted it is 
 * sent as a reply even if solClient_session_sendMsg() is used instead of 
 * solClient_session_reply().
 *
 * @param msg_p          solClient_opaqueMsg_pt returned from a previous call
 *                       to solClient_msg_alloc() or received in a receive
 *                       message callback.
 * @param isReply        A Boolean that indicates whether to set or reset the reply
 *                       attribute.
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL.
 * @subcodes             
 * @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - Indicates that the attempt to set
 *                       the reply attribute was for a message received from an older
 *                       style API that has insufficient space in the binary meta header.
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_setAsReplyMsg(solClient_opaqueMsg_pt msg_p, solClient_bool_t isReply);

/**
 * Given a msg_p, set the ElidingEligible property on a message. Setting this property
 * to true indicates that this message should be eligible for eliding. Message eliding
 * enables filtering of data to avoid transmitting every single update to a subscribing
 * client. It can be used to overcome slow consumers or any situation where a slower
 * message rate is desired. 
 * 
 * Time-based eliding (supported in SolOS-TR) ensures that subscriber applications
 * always receive only the most current update of a published topic at a rate that
 * they can manage. By limiting the incoming message rate, a subscriber application
 * is able to avoid a message backlog filled with outdated messages. 
 *
 * This property does not indicate whether the message was elided or even provide
 * information about the subscriber's configuration (with regards to Message Eliding).
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param elide    A Boolean that indicates whether to set or reset the Eliding Eligible
 *                 attribute.
 * @returns        ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if msg_p is invalid.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setElidingEligible(solClient_opaqueMsg_pt msg_p, solClient_bool_t elide);

/**
 * Given a msg_p, test the ElidingEligible attribute.
 * Does not indicate whether messages were elided or provide information about the
 * subscriber profile eliding configuration.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 *
 * @returns        True, if the message has the Eliding Eligible attribute set.
 */
solClient_dllExport solClient_bool_t
solClient_msg_isElidingEligible(solClient_opaqueMsg_pt msg_p);

/*
 * @section Machine Independent Interface
 *
 * The Solace Message Buffer interface provides utilities to read and write
 * data in a machine-independent manner. These functions guarantee that data 
 * may be transferred between platforms of differing architectures without 
 * error.
 *
 * Machine Independent Data Fields may be added to any of:
 * @li The Solace Message Format (SMF) Binary Attachment in the message, as part
 * of a Solace container (either a MAP or a STREAM).
 * @li The User Property map in the SMF binary metadata header.
 */

/**
 * Create a map container in the binary attachment of the message.
 * The map is a multimap in which more than one value may be associated 
 * with a given field name. A call to <i>solClient_container_addXyz()</i> does not
 * overwrite an existing one, but adds a new one instead. To overwrite an existing
 * field, the field has to been deleted and then added with a new value. To get all
 * values associated with a given field name, a linear search is required.
 * Any existing data is overwritten with the map that will be created by subsequent
 * primitive data functions.
 * The returned opaque container reference must be used for subsequent 
 * add functions.
 * The returned map should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the map
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(). If the map is closed automatically, the 
 * application may not continue to use the map. Attempting to use a closed map
 * will return an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
*
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param map_p         A pointer location to receive the container pointer.
 * @param size          A hint to the size (in bytes) of the map to be created. This
 *                      is used to determine what size of data block to allocate.
 *                      Datablocks are available in fixed sizes from
 *                      ::SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_0
 *                      to ::SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_4. 
 *                      If the data block is too small for the subsequently created map, a
 *                      larger data block is allocated when necessary, and 
 *                      existing structured data copied into place. This 
 *                      reallocation can negatively affect performance.
 *
 * @returns             ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_createBinaryAttachmentMap(solClient_opaqueMsg_pt   msg_p,
                       solClient_opaqueContainer_pt   *map_p,
                       solClient_uint32_t              size);

/**
 * Create a stream container in the binary attachment of the 
 * message. Any existing data is overwritten 
 * with the stream that is created by subsequent primitive data functions.
 * The returned opaque container reference must be used for subsequent 
 * add functions.
 * The returned stream should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the stream
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(). If the stream is closed automatically, the 
 * application may not continue to use the stream. Attempting to use a closed stream
 * returns an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param stream_p      A pointer location to receive the container pointer.
 * @param size          A hint to the size (in bytes) of the stream to be created. This
 *                      is used to determine what size of data block to allocate.
 *                      Datablocks are available in fixed sizes from
 *                      ::SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_0
 *                      to ::SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_4. 
 *                      If it is too small for the subsequently created stream, a
 *                      larger data block is allocated when necessary and 
 *                      existing structured data copied into place. This 
 *                      reallocation can negatively affect performance.
 *
 * @returns             ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_createBinaryAttachmentStream(solClient_opaqueMsg_pt   msg_p,
                       solClient_opaqueContainer_pt   *stream_p,
                       solClient_uint32_t              size);

/**
 * Create a User Property map in the binary metadata header.
 * The map is a multimap in which more than one value may be associated 
 * with a given field name. A call to <i>solClient_container_addXyz()</i> does not
 * overwrite an existing one, but adds a new one instead. To overwrite an existing
 * field, the field has to been deleted and then added with a new value. To get all
 * values associated with a given field name, a linear search is required.
 * Any existing data is overwritten with the map that is created by subsequent
 * primitive data functions.
 * It returns an opaque container reference that must be used for subsequent 
 * add functions.
 * The returned map should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the stream
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(). If the stream is closed automatically, the 
 * application may not continue to use the stream. Attempting to use a closed stream
 * returns an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 * @param msg_p    solClient_opaqueMsg_pt returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param map_p         A pointer location to receive the container pointer.
 * @param size          A hint to the size (in bytes) of the map to be created. This
 *                      is used to determine what size of data block to allocate.
 *                      Datablocks are available in fixed sizes from
 *                      ::SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_0
 *                      to ::SOLCLIENT_GLOBAL_PROP_DBQUANTASIZE_4. 
 *                      If it is too small for the subsequently created map, a
 *                      larger data block is allocated when necessary, and 
 *                      existing structured data is copied into place. This 
 *                      reallocation can negatively affect performance.
 * @returns             ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_createUserPropertyMap(solClient_opaqueMsg_pt   msg_p,
                       solClient_opaqueContainer_pt   *map_p,
                       solClient_uint32_t              size);

/**
 * Set the User Property map in the binary metadata header.
 * If there is an existing User Property map,
 * it is overwritten with the map that is passed as a parameter. If the application
 * has any open containers referencing the existing map they are automatically closed.
 * Attempting to use a closed stream returns an invalid pointer error
 * (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 * Changes made to the passed in map subsequent to this call do affect 
 * the user property map.
 *
 * @param msg_p    solClient_opaqueMsg_pt returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param map_p    An opaque container pointer for the map to set.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setUserPropertyMap(solClient_opaqueMsg_pt   msg_p,
                       solClient_opaqueContainer_pt       map_p);

/**
 *  Check the binary attachment in the message for structured data.
 *  If there is structured data found, either a map or steam, then the 
 *  opaque container required to access the data is returned.
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param field_p  The address of the solClient_field_t structure where the API will 
 *                 returns the opaque container pointer and type.
 * @param fieldSize  sizeof(solClient_field_t).
 *
 * @returns        ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_NO_STRUCTURED_DATA 
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getBinaryAttachmentField (solClient_opaqueMsg_pt     msg_p,
                        solClient_field_t         *field_p,
                        size_t                     fieldSize);

/**
 * Returns the value of the specified message statistic. 
 *
 * @param msgStatType The type of statistic to retrieve; one of ::solClient_msg_stats.
 * @param statIndex   The zero-based index of the statistic (for example, which quanta);
 *                    only used for ::SOLCLIENT_MSG_STATS_ALLOC_DATA_BLOCKS and
 *                    ::SOLCLIENT_MSG_STATS_FREE_DATA_BLOCKS. This must be zero for
 *                    other statistic types.
 * @param statValue_p A pointer to the location to receive the statistic value from.
 *
 * @returns  ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getStat(solClient_msg_stats_t  msgStatType,
                      solClient_uint32_t     statIndex,
                      solClient_uint64_t    *statValue_p);

/**
 *  Display the contents of a message in human-readable form. This function creates a
 *  string in application-supplied buffer that the application can log or print. If
 *  the supplied buffer pointer is NULL, this function prints to STDOUT.
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buffer_p A pointer to a buffer to receive the output string. NULL means to dump
 *                 to STDOUT.
 * @param bufferSize The size of the memory area referenced by buffer_p. The output is
 *                   truncated at this size if the buffer is not large enough. The
 *                   returned string is always NULL-terminated even
 *                   if truncated.
 * @param flags    Flags to control the output. Possible values are:
 *                      @li ::SOLCLIENT_MSGDUMP_BRIEF. Display only the length of the
 *                          binary attachment, xml attachment, and user property map.
 *                      @li ::SOLCLIENT_MSGDUMP_FULL. Display the entire message.
 *
 *  @returns            @li ::SOLCLIENT_OK on success
 *                      @li ::SOLCLIENT_FAIL on failure. This function can fail if any
 *                          of the parameters to the call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR
 *  @li ::SOLCLIENT_SUBCODE_PARAM_CONFLICT
 */ 

solClient_dllExport solClient_returnCode_t
solClient_msg_dumpExt(
    solClient_opaqueMsg_pt  msg_p,
    char                   *buffer_p,
    size_t                  bufferSize,
    solClient_uint32_t      flags
);

/**
 *  Display the contents of a message in human-readable form. This function creates a
 *  string in application-supplied buffer that the application can log or print as it
 *  wishes. If the supplied buffer pointer is NULL, this function prints to STDOUT.
 *  This function is equivalent to solClient_msg_dumpExt(msg_p, buffer_p, bufferSize,
 *  SOLCLIENT_MSGDUMP_FULL).
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param buffer_p A pointer to a buffer to receive the output string. NULL means to dump
 *                 to STDOUT.
 * @param bufferSize The size of the memory area referenced by buffer_p. The output is
 *                   truncated at this size if the buffer is not large enough. The
 *                   returned string is always NULL-terminated even if truncated.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can fail if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR
 *  @li ::SOLCLIENT_SUBCODE_PARAM_CONFLICT
 */ 

solClient_dllExport solClient_returnCode_t
solClient_msg_dump(
    solClient_opaqueMsg_pt  msg_p,
    char                   *buffer_p,
    size_t                  bufferSize
);

/**
 * Given a msg_p, set the optional ACK Immediately message property.
 * When the ACK Immediately property is set to true on an outgoing Guaranteed Delivery message, 
 * it indicates that the appliance should ACK this message immediately upon receipt. 
 * By default the property is set to false on newly created messages.
 * 
 * This property, when set by a publisher, may or may not be removed by the appliance prior to delivery
 * to a consumer, so message consumers must not expect the property value indicates how the message was
 * originally published. Therefore if a received message
 * is forwarded by the application, the ACK immediately property should be explicitly set to the desired 
 * value (true or false).
 *
 * Setting this property on an outgoing direct message has no effect.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param val      A Boolean that indicates whether to set or clear the ACK Immediately message property. 
 * @returns        ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL if msg is invalid.
 *                      
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setAckImmediately(
    solClient_opaqueMsg_pt  msg_p,
    solClient_bool_t        val
);

/**
 * Given a msg_p, test if the ACK Immediately message property is set or not.
 * When the ACK Immediately property is set to true on an outgoing Guaranteed Delivery message, 
 * it indicates that the appliance should ACK this message immediately upon receipt.
 * 
 * This property, when set by a publisher, may or may not be removed by the appliance prior to delivery
 * to a consumer, so message consumers must not expect the property value indicates how the message was
 * originally published
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns         True, if the ACK Immediately message property is set to TRUE.
 */
solClient_dllExport solClient_bool_t  
solClient_msg_isAckImmediately(
    solClient_opaqueMsg_pt    msg_p
);

/**
 * Given a msg_p, retrieve the HTTP Content Type. On return type_p points to
 * message memory and is only valid as long as msg_p is valid.
 * 
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param type_p   On return, it points to message memory containing HTTP Content Type.
 * @returns        ::SOLCLIENT_OK on success, ::SOLCLIENT_NOT_FOUND if the field is 
 *                     not present and ::SOLCLIENT_FAIL if  msg_p is invalid
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getHttpContentType(solClient_opaqueMsg_pt  msg_p,
                                 const char *           *type_p);

/**
 * Given a msg_p, retrieve the HTTP Content Encoding. On return type_p points to
 * message memory and is only valid as long as msg_p is valid.
 * 
 * @param msg_p       solClient_opaqueMsg_pt that is returned from a previous call
 *                    to solClient_msg_alloc() or received in a receive
 *                    message callback.
 * @param encoding_p  On return, it points to message memory containing HTTP Content Encoding.
 * @returns           ::SOLCLIENT_OK on success, ::SOLCLIENT_NOT_FOUND if the field is 
 *                       not present and ::SOLCLIENT_FAIL if  msg_p is invalid
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_getHttpContentEncoding(solClient_opaqueMsg_pt  msg_p,
                                     const char *           *encoding_p);

/**
 * Given a msg_p, set or delete (if type_p == NULL) its HTTP Content Type.
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param type_p   A pointer to a null terminated HTTP Content Type .
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setHttpContentType(solClient_opaqueMsg_pt msg_p,
                                 const char *type_p);

/**
 * Given a msg_p, set or delete (if encoding_p == NULL) its HTTP Content Encoding. 
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @param encoding_p     A pointer to a null terminated HTTP Content Encoding.
 * @returns              ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_setHttpContentEncoding(solClient_opaqueMsg_pt msg_p,
                                 const char *encoding_p);

/** 
 * Given a msg_p, delete the HTTP Content Type. 
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteHttpContentType(solClient_opaqueMsg_pt msg_p);

/** 
 * Given a msg_p, delete the HTTP Content Encoding. 
 *
 * @param msg_p    A solClient_opaqueMsg_pt that is returned from a previous call
 *                 to solClient_msg_alloc() or received in a receive
 *                 message callback.
 * @returns        ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_msg_deleteHttpContentEncoding(solClient_opaqueMsg_pt msg_p);

/**
 * Create a map container in a memory location given by the caller. It is the
 * caller's responsibility to ensure that the memory pointer and size are valid. If the
 * map that is created by subsequent calls to add structured data types exceeds
 * <i>size</i>, the add operation fails and the error subCode is set to
 * SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE.
 * The map is a multimap in which more than one value may be associated with a given field
 * name. 
 * A call to <i>solClient_container_addXyz()</i> does not
 * overwrite an existing field, instead it adds a new one. To overwrite an existing
 * field, the field has to been deleted and then added with a new value. To get all
 * values associated with a given field name, a linear search is required.
 * Any existing data is overwritten with the map that is created by subsequent 
 * primitive data functions. The returned opaque container reference must be used for
 * subsequent add functions. In addition, the returned opaque container can itself be
 * added to other containers or directly to a message binary attachment part.
 * The returned map must later be closed by a call to 
 * ::solClient_container_closeMapStream(). 
 *
 * @param newContainer_p         A pointer location to receive the container pointer.
 * @param mem_p         A pointer to memory that is used to build the map.
 * @param size          The maximum size of the map to be created.
 *
 * @returns             ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_createMap (solClient_opaqueContainer_pt      *newContainer_p,
                               char *                             mem_p,
                               size_t                             size);

/**
 * This function creates a stream container in a memory location given by the caller. It
 * is the caller's responsibility to ensure that the memory pointer and size are valid.
 * If the stream that is built by subsequent calls to add structured data types exceeds
 * the specified maximum <i>size</i>, the add operation fails and the error subCode is
 * set to SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE.
 * Any existing data is overwritten with the stream that is created by subsequent 
 * primitive data functions. The returned opaque container reference must be used for
 * subsequent add functions. In addition, the returned opaque container itself can be
 * added to other containers or directly to a message binary attachment part.
 * The returned stream must later be closed by a call to 
 * ::solClient_container_closeMapStream(). 
 *
 * @param newContainer_p         A pointer location to receive the container pointer.
 * @param mem_p         A pointer to memory that is used to build the stream.
 * @param size          The maximum size of the stream to be created.
 *
 * @returns             ::SOLCLIENT_OK, ::SOLCLIENT_FAIL
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_createStream (solClient_opaqueContainer_pt      *newContainer_p,
                               char *                             mem_p,
                               size_t                             size);
/**
 * Open a nested subMap. The subMap is a multimap in which more than one value may be
 * associated with a given field name. This is a special add function to create a map in
 * an existing container. It returns a new solClient_opaqueContainer_pt that 
 * can be used to build a map using addXXX function. It is possible to add to 
 * the original container; however, this can cause extensive data moving 
 * operations when the subMap is later closed by a call to 
 * ::solClient_container_closeMapStream. It is more efficient to write the subMap
 * completely and close it before writing again to the original container.
 * The returned map should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the map
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(), or if the parent container is closed. 
 * If the map is closed automatically, the 
 * application may not continue to use the map. Attempting to use a closed map
 * returns an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 * @param container_p  An opaque container pointer.
 * @param newContainer_p A pointer to memory to receive returned opaque 
 *                     container pointer.
 * @param name         The name of the Map if the container_ is Map, must be 
 *                     NULL if container_p references a Stream.
 * @returns            ::SOLCLIENT_OK on success. ::SOLCLIENT_FAIL on
 *                     failure.
 * @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_openSubMap (solClient_opaqueContainer_pt container_p,
                          solClient_opaqueContainer_pt      *newContainer_p,
                          const char *                       name);

/**
 * Open a nested subStream. This is a special 'add' function to create a stream in
 * an existing container. It returns a new solClient_opaqueContainer_pt that 
 * may be used to build a stream using addXXX function. It is possible to add to 
 * the original container; however, this can cause extensive data moving 
 * operations when the subStream is later closed by a call to 
 * ::solClient_container_closeMapStream. It is more efficient to write the subStream
 * completely and close it before writing again to the original container.
 * The returned stream should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the stream
 * is automatically closed when the associated message is freed by a call to 
 * ::solClient_msg_free(), or if the parent container is closed. 
 * If the stream is closed automatically, the 
 * application may not continue to use the stream. Attempting to use a closed stream
 * will return an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 * @param container_p  An opaque container pointer.
 * @param newContainer_p A pointer to memory to receive returned opaque 
 *                     container pointer.
 * @param name         The name of the stream if the container_p is a map; it must be 
 *                     NULL if container_p references a stream.
 * @returns            ::SOLCLIENT_OK on success. ::SOLCLIENT_FAIL on
 *                     failure.
 * @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_openSubStream (solClient_opaqueContainer_pt container_p,
                             solClient_opaqueContainer_pt *newContainer_p,
                             const char *                  name);

/**
 * 
 * Finish a map or stream. This action makes the opaque container 
 * pointer invalid and fixes the structured data in memory.
 * 
 *  @param container_p  A pointer to the opaque container pointer. The pointer is
 *                      reset to NULL on return.
 *
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 * @subcodes
 * @li ::SOLCLIENT_SUBCODE_CONTAINER_BUSY
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_closeMapStream (solClient_opaqueContainer_pt *container_p);

/**
 *  Add a NULL to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addNull(solClient_opaqueContainer_pt container_p,
                        const char *                     name);

/**
 *  Add a Boolean value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A Boolean value. Any non-zero value is encoded
 *                      as 1.
 *  @param name         The name of a field for a Map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addBoolean(solClient_opaqueContainer_pt container_p,
                        solClient_bool_t                   value,
                        const char *                       name);

/**
 *  Add a unsigned 8-bit value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        An unsigned 8-bit value.
 *  @param name        The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addUint8 (solClient_opaqueContainer_pt container_p,
                       solClient_uint8_t                  value,
                       const char *                       name);
/**
 *  Add a signed 8-bit value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value         A signed 8-bit value.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addInt8 (solClient_opaqueContainer_pt container_p,
                       solClient_int8_t                   value,
                       const char *                       name);
/**
 *  Add an unsigned 16-bit value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        An unsigned 16-bit.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addUint16 (solClient_opaqueContainer_pt container_p,
                       solClient_uint16_t                 value,
                       const char *                       name);
/**
 *  Add a signed 16-bit value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A signed 16-bit value.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addInt16 (solClient_opaqueContainer_pt container_p,
                       solClient_int16_t                  value,
                       const char *                       name);
/**
 *  Add an unsigned 32-bit value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        An unsigned 32-bit value.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addUint32 (solClient_opaqueContainer_pt container_p,
                       solClient_uint32_t                  value,
                       const char *                        name);
/**
 *  Add a signed 32-bit value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A signed 32-bit value.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addInt32 (solClient_opaqueContainer_pt container_p,
                       solClient_int32_t                  value,
                       const char *                       name);
/**
 *  Add a unsigned 64-bit value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        An unsigned 64-bit value.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addUint64 (solClient_opaqueContainer_pt container_p,
                       solClient_uint64_t                  value,
                       const char *                        name);
/**
 *  Add a signed 64-bit value to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A signed 64-bit value.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addInt64 (solClient_opaqueContainer_pt container_p,
                       solClient_int64_t                  value,
                       const char *                       name);
/**
 *  Add an ASCII character to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        An ASCII character.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addChar (solClient_opaqueContainer_pt container_p,
                       char                               value,
                       const char *                       name);
/**
 *  Add a unicode (wide) character to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A 16-bit unicode character.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addWchar (solClient_opaqueContainer_pt container_p,
                       solClient_wchar_t                  value,
                       const char *                       name);
/**
 *  Add a byte array to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param arr_p        A pointer to the byte array.
 *  @param length       The length of the byte array.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addByteArray (solClient_opaqueContainer_pt container_p,
                           const solClient_uint8_t           *arr_p,
                           solClient_uint32_t                 length,
                           const char *                       name);

/**
 *  Add a Solace Message Format (SMF) message to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param smf_p        A pointer to the SMF message.
 *  @param length       The length of the SMF message.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addSmf (solClient_opaqueContainer_pt container_p,
                           const solClient_uint8_t           *smf_p,
                           solClient_uint32_t                 length,
                           const char *                       name);

/**
 *  Add an existing container to a map or stream. If the container being added to is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL.
 *  This is a copy-in operation. Changing the subContainer after calling this function will not 
 *  be reflected in the copy created by this function. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param subContainer_p  An opaque container pointer for the container to add.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addContainer (solClient_opaqueContainer_pt container_p,
                           solClient_opaqueContainer_pt subContainer_p,
                           const char *                       name);

/**
 *  Add a 32-bit floating point number to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A 32-bit floating point number
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addFloat (solClient_opaqueContainer_pt container_p,
                       float                              value,
                       const char *                       name);
/**
 *  Add a 64-bit floating point number to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A 64-bit floating point number
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addDouble (solClient_opaqueContainer_pt container_p,
                       double                             value,
                       const char *                       name);
/**
 *  Add a null terminated string (ASCII or UTF-8) to a map or stream. If the container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a NULL-terminated string.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addString (solClient_opaqueContainer_pt container_p,
                       const char *                       value,
                       const char *                       name);

/**
 *  Add a Solace destination (queue or topic) to the container. If the 
 *  container is a stream, the
 *  name parameter must be NULL. If the container is a map, the name parameter
 *  must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a solClient_destination_t.
 *  @param destSize     The size of solClient_destination_t
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_addDestination (solClient_opaqueContainer_pt container_p,
                       const solClient_destination_t     *value,
                       size_t                             destSize,
                       const char                        *name);

/**
 *  Add an application-defined field to the container. The field must be properly 
 *  encoded according to the Solace-defined machine-independent coding rules.
 *  Typically, this function is only used by forwarding applications that need to 
 *  forward unrecognized fields discovered by solClient_container_getField (field type is ::SOLCLIENT_UNKNOWN).
 *  If the container is a stream, the name parameter must be NULL. If the container is a map, the 
 *  name parameter must be non-NULL. @see @ref msgMaps
 *
 *  @param container_p  An opaque container pointer.
 *  @param field_p      A pointer to a byte array containing the encoded field.
 *  @param length       The length of the field.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @see @ref encoding-overhead
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_PARAM_NULL_PTR     - The opaque container pointer is invalid, or the container is a map and the field name 
 *                      pointer is invalid.
 *  @li ::SOLCLIENT_SUBCODE_PARAM_OUT_OF_RANGE - The container is a stream and the field name pointer is non-null.
 *  @li ::SOLCLIENT_SUBCODE_OUT_OF_MEMORY      - The container created by solClient_msg_createBinaryAttachmentMap(), 
 *                      or solClient_msg_createBinaryAttachmentStream(), or solClient_msg_createUserPropertyMap() has consumed all available
 *                      space in the @ref solclientmsg "message buffer" and the API is unable to allocate more memory from the operating 
 *                      system heap.
 *  @li ::SOLCLIENT_SUBCODE_INSUFFICIENT_SPACE - The container created by solClient_container_createMap() or solClient_container_createStream() 
 *                      has consumed all available space in the buffer. <b>NOTE:</b> This error can occur when such a container has an open 
 *                      subcontainer; until the open subcontainer is closed, this function will fail and set this subcode. This does not
 *                      occur on containers created in @ref solclientmsg as memory can be dynamically allocated.
 *
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_container_addUnknownField    ( solClient_opaqueContainer_pt container_p,
                                         const solClient_uint8_t           *field_p,
                                         size_t                             length,
                                         const char *                       name);

/**
 * Rewind a container.
 * 
 * Rewind the stream or map so that the getting of fields can be started over from
 * the start of the stream or map.
 *
 *  @param container_p  An opaque container pointer.
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t
solClient_container_rewind (solClient_opaqueContainer_pt container_p);


/**
 * When iterating through a container (a map or stream), this function will return false when
 * the end of the container is reached without advancing the read cursor. It will return true
 * otherwise.
 * @param container_p An opaque container pointer.
 * @returns   TRUE, if there is a next field in the container; FALSE, otherwise.
 * @subcodes
 * @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_bool_t
solClient_container_hasNextField (solClient_opaqueContainer_pt container_p);


/**
 * Read the next field of the map (or stream, although this function is not preferred for streams).
 * 
 * For a map, this returns the next field in the map, including the name of
 * the field. This allows the application to iterate over the contents of the
 * map.
 *
 * While this function will work for a stream, it is preferable to use
 * solClient_container_getField or solClient_container_getXXX for a specific
 * field type. If this routine is used, a NULL pointer is returned for the 
 * field name if the provided pointer to the field name is not NULL.
 * 
 * If the returned field is a container (map or stream), the container should 
 * later be closed by a call to ::solClient_container_closeMapStream(). However, if it is not, 
 * the container is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(), or if the parent container is closed. 
 * If the container is closed automatically, the application may not continue 
 * to use the container. Attempting to use a closed container
 * will return an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 *  @param container_p  An opaque container pointer.
 *  @param field_p      The address of solClient_field_t where the API will return 
 *                      field type and value.
 *  @param fieldSize    sizeof(solClient_field_t). This parameter is used for backwards binary 
 *                      compatibility if solClient_field_t grows.
 *  @param name_p       The address of the pointer where API will return a pointer to
 *                        the NULL-terminated field name. For a stream, the 
 *                        passed in pointer can be NULL, and if not, the returned
 *                        pointer will be NULL.
 *
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_FAIL on failure.
 *                      ::SOLCLIENT_EOS at end of container.
 *  @subcodes
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getNextField (solClient_opaqueContainer_pt container_p,
                                  solClient_field_t       *field_p,
                                  size_t                   fieldSize,
                                  const char *            *name_p);
                                                         
/**
 * Get a field value from the map or stream.
 * For a map, the name pointer must not be NULL, and the field corresponding to
 * the name is returned.
 * For a stream, the name pointer must be NULL. The next field in the stream is 
 * extracted, and the read position in the stream is advanced.
 *
 * Note that normally the solClient_container_getXXX for a specific type is used, 
 * instead of solClient_container_getField. However, this function can be used 
 * to get a named field from a map, or the next field from a stream, without knowing
 * its type in advance.
 * 
 * If the returned field is a container (map or stream), the container should 
 * later be closed by a call to ::solClient_container_closeMapStream(). However, if it is not, 
 * the container is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(), or if the parent container is closed. 
 * If the container is closed automatically, the 
 * application may not continue to use the container. Attempting to use a closed container
 * will return an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a solClient_field_t to receive the returned 
 *                      type and value.
 *  @param fieldSize    sizeof(solClient_field_t). This parameter is used for backwards binary compatibility if solClient_field_t grows.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            ::SOLCLIENT_OK on success, ::SOLCLIENT_EOS on the end of a stream, ::SOLCLIENT_NOT_FOUND if not found in a map, ::SOLCLIENT_FAIL on failure.
 *  @subcodes
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getField (solClient_opaqueContainer_pt container_p,
                       solClient_field_t           *value,
                       size_t                       fieldSize,
                       const char                  *name);

/**
 * Return the size of the map or stream.   
 *
 *  @param container_p  An opaque container pointer.
 *  @param size_p       A pointer to a size_t location where the size is returned.
 * 
 *  @returns            @li  ::SOLCLIENT_OK on success.
 *                      @li  ::SOLCLIENT_FAIL if container_p is invalid.
 *  @subcodes
 *  @see ::solClient_subCode for a description of all subcodes.
 */ 
solClient_dllExport solClient_returnCode_t 
solClient_container_getSize (solClient_opaqueContainer_pt container_p,
                             size_t                      *size_p);

/**
 *  Get a NULL value from the map or stream.
 *  Any data type, including maps and streams, can be read as a NULL
 *  type.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 * 
 *  @param container_p  An opaque container pointer.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if any of the parameters to the call are 
 *                      invalid.
 *  @subcodes
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getNull (solClient_opaqueContainer_pt container_p,
                       const char                  *name);
                       
/**
 *  Get a Boolean value from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 * 
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a Boolean, it contains binary 0 or 1
 *                      on successful return.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to
 *                      Boolean, or if any of the parameters to the call are 
 *                      invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getBoolean (solClient_opaqueContainer_pt container_p,
                       solClient_bool_t            *value,
                       const char                  *name);

/**
 *  Get an unsigned 8-bit int from the map or stream. 
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a unsigned 8-bit int.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to an
 *                      unsigned 8-bit int, or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getUint8 (solClient_opaqueContainer_pt container_p,
                       solClient_uint8_t           *value,
                       const char                  *name);

/**
 *  Get a signed 8-bit int from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a signed 8-bit int.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to a
 *                      signed 8-bit int or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getInt8 (solClient_opaqueContainer_pt container_p,
                       solClient_int8_t            *value,
                       const char                  *name);

/**
 *  Get an unsigned 16-bit int from the map or stream.  
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to an unsigned 16-bit int.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to an
 *                      unsigned 16-bit int or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getUint16 (solClient_opaqueContainer_pt container_p,
                       solClient_uint16_t           *value,
                       const char                   *name);

/**
 *  Get a signed 16-bit int from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a signed 16-bit int.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to a
 *                      signed 16-bit int or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getInt16 (solClient_opaqueContainer_pt container_p,
                       solClient_int16_t           *value,
                       const char                  *name);

/**
 *  Get an unsigned 32-bit int from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to an unsigned 32-bit int.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success
 *                      @li ::SOLCLIENT_EOS on the end of a stream
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to an
 *                      unsigned 32-bit int, or if any of the parameters to the call are invalid. 
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getUint32 (solClient_opaqueContainer_pt container_p,
                       solClient_uint32_t          *value,
                       const char                  *name);

/**
 *  Get a signed 32-bit int from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a signed 32-bit int.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to a
 *                      signed 32-bit int or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getInt32 (solClient_opaqueContainer_pt container_p,
                       solClient_int32_t           *value,
                       const char                  *name);

/**
 *  Get an unsigned 64-bit int from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to an unsigned 64-bit int.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function may 
 *                      fail if the referenced field cannot be converted to an
 *                      unsigned 64 bit int, or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getUint64 (solClient_opaqueContainer_pt container_p,
                       solClient_uint64_t          *value,
                       const char                  *name);


/**
 *  Get a signed 64-bit int from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted, and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a signed 64-bit int.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to a
 *                      signed 64-bit int or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getInt64 (solClient_opaqueContainer_pt container_p,
                       solClient_int64_t           *value,
                       const char                  *name);

/**
 *  Get an ASCII character from the map or stream. 
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a char.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to an
 *                      ASCII char, or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getChar (solClient_opaqueContainer_pt container_p,
                       char                        *value,
                       const char                  *name);

/**
 *  Get a unicode character from the map or stream. 
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a unicode character.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to a
 *                      unicode char, or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getWchar (solClient_opaqueContainer_pt container_p,
                       solClient_wchar_t           *value,
                       const char                  *name);

/**
 *  Get a pointer to a byte array from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param array_p      The address where the pointer to a byte array is returned.
 *  @param arrayLength_p The returned array length.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field is not a byte array,
 *                      or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getByteArrayPtr (solClient_opaqueContainer_pt container_p,
                       solClient_uint8_t          **array_p,
                       solClient_uint32_t          *arrayLength_p,
                       const char                  *name);

/**
 *  Get the contents of a byte array from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted, and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param array_p      A pointer to where the byte array is returned
 *  @param arrayLength_p  The IN/OUT maximum length of array.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.

 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field is not a byte array
 *                      or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getByteArray (solClient_opaqueContainer_pt container_p,
                       solClient_uint8_t          *array_p,
                       solClient_uint32_t         *arrayLength_p,
                       const char                 *name);

/**
 *  Get a pointer to a Solace Message Format (SMF) message from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted, and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param smf_p        An address where a pointer to a byte array is returned
 *  @param smfLength_p  The returned array length.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can
 *                      fail if the referenced field is not a byte array,
 *                      or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getSmfPtr (solClient_opaqueContainer_pt container_p,
                       solClient_uint8_t          **smf_p,
                       solClient_uint32_t          *smfLength_p,
                       const char                  *name);

/**
 *  Get contents of a Solace Message Format (SMF) message from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param smf_p      A pointer to where the byte array is returned
 *  @param smfLength_p  The IN/OUT maximum length of array.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success
 *                      @li ::SOLCLIENT_EOS on the end of a stream
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field is not a byte array,
 *                      or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getSmf (solClient_opaqueContainer_pt container_p,
                       solClient_uint8_t          *smf_p,
                       solClient_uint32_t         *smfLength_p,
                       const char                 *name);

/**
 *  Get a float from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a float value.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to a
 *                      32-bit floating point int, or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getFloat (solClient_opaqueContainer_pt container_p,
                       float                       *value,
                       const char                  *name);

/**
 *  Get a double from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted, and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a double.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field cannot be converted to a
 *                      64-bit floating point int, or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getDouble (solClient_opaqueContainer_pt container_p,
                       double                      *value,
                       const char                  *name);

/**
 *  Get a null terminated string from the map or stream. 
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted, and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param string       An address of pointer to a string location.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field is not a string,
 *                      or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getStringPtr (solClient_opaqueContainer_pt container_p,
                                  const char *                *string,
                                  const char                  *name);

/**
 *  Copy a null terminated string from the map or stream.
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted, and the read position in the stream is advanced.
 *
 *  @param container_p  An opaque container pointer.
 *  @param string       A pointer to a string location.
 *  @param size         A maximum length of string.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field is not a string
 *                      int or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getString (solClient_opaqueContainer_pt container_p,
                       char                       *string,
                       size_t                      size,
                       const char                 *name);

/**
 *  Get a destination.  
 *  For a map, the name pointer must not be NULL, and the field corresponding to
 *  the name is returned.
 *  For a stream, the name pointer must be NULL. The next field in the stream is 
 *  extracted and the read position in the stream is advanced.
 *
 *  If the field is a destination, this function copies the destination
 *  information to the supplied 'destination' structure. Within the destination is a pointer
 *  to the destination name (a string within the message). This string is only valid as 
 *  long as the solClient_msg is valid.
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a solClient_destination_t.
 *  @param destSize     sizeof(solClient_destination_t). This parameter is used for backwards binary compatibility if solClient_destination_t changes in the future.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field does not contain a 
 *                      destination, or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getDestination (solClient_opaqueContainer_pt container_p,
                       solClient_destination_t      *value,
                       size_t                        destSize,
                       const char                   *name);

/**
 * Get a container pointer for the SubMap found in the current
 * container. 
 * For a map, the name pointer must not be NULL, and the field corresponding to
 * the name is returned.
 * For a stream, the name pointer must be NULL. The next field in the stream is 
 * extracted, and the read position in the stream is advanced.
 *
 * The returned map should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the map
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(). If the map is closed automatically, the 
 * application may not continue to use the map. Attempting to use a closed map
 * will return an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a container pointer.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field is not a map
 *                      or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getSubMap (solClient_opaqueContainer_pt container_p,
                       solClient_opaqueContainer_pt  *value,
                       const char                    *name);

/**
 * Get a container pointer for the SubStream found in the current
 * container. 
 * For a map, the name pointer must not be NULL, and the field corresponding to
 * the name is returned.
 * For a stream, the name pointer must be NULL. The next field in the stream is 
 * extracted and the read position in the stream is advanced.
 *
 * The returned stream should later be closed by a call to 
 * ::solClient_container_closeMapStream(). However, if it is not, the stream
 * is automatically closed when the associated message is freed through a call to 
 * ::solClient_msg_free(). If the stream is closed automatically, the 
 * application may not continue to use the stream. Attempting to use a closed stream
 * will return an invalid pointer error (::SOLCLIENT_SUBCODE_PARAM_NULL_PTR).
 *
 *  @param container_p  An opaque container pointer.
 *  @param value        A pointer to a container pointer.
 *  @param name         The name of a field for a map. This parameter must be NULL for a stream.
 *
 *  @returns            @li ::SOLCLIENT_OK on success.
 *                      @li ::SOLCLIENT_EOS on the end of a stream.
 *                      @li ::SOLCLIENT_NOT_FOUND if not found in a map.
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if the referenced field is not a stream,
 *                      or if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @li ::SOLCLIENT_SUBCODE_INVALID_DATA_CONVERSION
*  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_getSubStream (solClient_opaqueContainer_pt container_p,
                       solClient_opaqueContainer_pt   *value,
                       const char                     *name);

/**
 *  Remove a field from a map. This function returns an error if called on
 *  a stream. The serialized map is scanned and the named field is removed.
 *  All subsequent fields in the serialized map are moved; this can be a slow
 *  operation.
 *
 *  @param container_p  An opaque container pointer.
 *  @param name         The name of the field to remove.
 *
 *  @returns            @li ::SOLCLIENT_OK on success
 *                      @li ::SOLCLIENT_FAIL on failure. This function can 
 *                      fail if any of the parameters to the 
 *                      call are invalid.
 *  @subcodes
 *  @see ::solClient_subCode for a description of all subcodes.
 */
solClient_dllExport solClient_returnCode_t 
solClient_container_deleteField (solClient_opaqueContainer_pt container_p,
                                 const char                  *name);

/**
 * Create an initialized message block from the passed in serialized (or raw)
 * SMF message. A solClient message buffer is allocated. Applications
 * are responsible for releasing the message buffer.
 *
 * @param bufinfo_p   A pointer to the bufInfo describing a validly formatted SMF direct message.
 * @param msg_p       Pointer to a reference to a message. This value is set
 *                    on successful return to a msg_p suitable for subsequent function calls.
 *                    The message buffer has to be freed by applications on successful return.   
 * @returns           ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 *
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_decodeFromSmf(solClient_bufInfo_pt       bufinfo_p,
                             solClient_opaqueMsg_pt     *msg_p);

/**
 * Encode a message in Solace Message Format (SMF) and save the SMF encodings in 
 * the message bufinfo part.
 * If both bufinfo_p and datab_p are not NULL pointers, they contain 
 * the bufinfo for the SMF encodings and its associated data block as returns. 
 * Client applications are responsible for releasing the associated data block.
 * 
 * It returns SOLCLIENT_FAIL if one of bufinfo_p and datab_p is NULL pointer and the other is not a NULL pointer.
 *
 * @param msg_p    A pointer to the message 
 * @param bufinfo_p       A pointer to a bufInfo containing buffer information (ptr, size).as returns.
 * @param datab_p         A pointer to a data block reference returns data block pointer.The data block has to be released by applications.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_encodeToSMF(solClient_opaqueMsg_pt         msg_p,
                           solClient_bufInfo_pt           bufinfo_p,
                           solClient_opaqueDatablock_pt  *datab_p);

/**
 * Get message priority.
 *
 * @param msg_p    A pointer to the message 
 * @param priority_p   A pointer to memory that contains priority on return, or -1 if it is not set.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getPriority(solClient_opaqueMsg_pt msg_p,
                          solClient_int32_t  *priority_p);
/**
 * Set message priority.
 *
 * @param msg_p    A pointer to the message 
 * @param priority Priority value. The valid value range is 0-255 with 0 as the lowest priority and 255 as the highest, or -1 to delete priority.
 * @returns              ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_setPriority(solClient_opaqueMsg_pt msg_p,
                          solClient_int32_t    priority);

#define SOLCLIENT_REPLICATION_GROUP_MESSAGE_ID_INITIALIZER {{0}}
#define SOLCLIENT_REPLICATION_GROUP_MESSAGE_ID_SIZE 16
#define SOLCLIENT_REPLICATION_GROUP_MESSAGE_ID_STRING_LENGTH 41

typedef struct solClient_replicationGroupMessageId{
    char replicationGroupMessageId[SOLCLIENT_REPLICATION_GROUP_MESSAGE_ID_SIZE];
} solClient_replicationGroupMessageId_t, *solClient_replicationGroupMessageId_pt;

/**
 * Validate a Replication Group Message Id
 *
 * @param rgmid_p       A pointer to a ::solClient_replicationGroupMessageId_t. The 
 *                      object must have been previously initialized by either
 *                      ::solClient_msg_getReplicationGroupMessageId() or 
 *                      ::solClient_replicationGroupMessageId_fromString()
 * @returns             ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 */
solClient_dllExport solClient_bool_t
solClient_replicationGroupMessageId_isValid(solClient_replicationGroupMessageId_pt rgmid_p);

/**
 * Compare two Replication Group Message Id.  Not all valid ::solClient_replicationGroupMessageId_t
 * can be compared.  If the messages identified were not published to the same broker or HA pair, then
 * they are not comparable and this method returns ::SOLCLIENT_FAIL
 *
 * @param rgmid1_p      A pointer to the first ::solClient_replicationGroupMessageId_t
 * @param rgmid2_p      A pointer to the second ::solClient_replicationGroupMessageId_t
 * @param compare_p     A pointer to an integer for the result which is:
 *                              *compare_p < 0 if first is less than the second
 *                              *compare_p == 0 if both are the same.
 *                              *compare_p >0 if the first is greater than the second.
 * @returns             ::SOLCLIENT_OK on success.  ::SOLCLIENT_FAIL if the Replication Group 
 *                      Message Id cannot be compared.
 * @subcodes            ::SOLCLIENT_SUBCODE_MESSAGE_ID_NOT_COMPARABLE
 */
solClient_dllExport solClient_returnCode_t           
solClient_replicationGroupMessageId_compare(solClient_replicationGroupMessageId_pt rgmid1_p,
                                            solClient_replicationGroupMessageId_pt rgmid2_p,
                                            int *compare_p);

/** 
 * Convert a Replication Group Message Id to a defined string format.  The standard 
 * format can be stored for later use in ::solClient_replicationGroupMessageId_fromString. 
 *
 * @param rgmid_p       A pointer to ::solClient_replicationGroupMessageId_t to serialize.
 * @param size_rgmid    The return from sizeof(solClient_replicationGroupMessageId_t)
 * @param str           Pointer to string location to copy the string into.
 * @param size_str      The available memory for the string. It should be at least 45 bytes
 *                      for the standard string: rmid1:xxxxx-xxxxxxxxxxx-xxxxxxxx-xxxxxxxx
 *                      If there is not enough room the output is truncated.
 *
 * @returns             ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 */
solClient_dllExport solClient_returnCode_t
solClient_replicationGroupMessageId_toString(solClient_replicationGroupMessageId_pt rgmid_p,
                                             size_t size_rgmid,
                                             char *str,
                                             size_t size_str);

/** 
 * Create a Replication Group Message Id from the string format.  The string may be 
 * retrieved by a call to ::solClient_replicationGroupMessageId_toString, or it can be retrieved
 * from any of the broker admin interfaces.
 *
 * @param rgmid_p       A pointer to ::solClient_replicationGroupMessageId_t to be filled.
 * @param size_rgmid    The return from sizeof(solClient_replicationGroupMessageId_t)
 * @param rgmid_str     Pointer to string representation of the Replication Group MessageId.
 *
 * @returns             ::SOLCLIENT_OK or ::SOLCLIENT_FAIL
 */
solClient_dllExport solClient_returnCode_t 
solClient_replicationGroupMessageId_fromString(solClient_replicationGroupMessageId_pt rgmid_p,
                                               size_t size_rgmid,
                                               const char *rgmid_str);

/** 
 * Retrieve a Replication Group Message Id from a received message.
 *
 * @param msg_p         solClient_opaqueMsg_pt that is received in a receive message callback.
 * @param rgmid_p       A pointer to ::solClient_replicationGroupMessageId_t to be filled.
 * @param size          The return from sizeof(solClient_replicationGroupMessageId_t)
 *
 * @returns             ::SOLCLIENT_OK or ::SOLCLIENT_FAIL or ::SOLCLIENT_NOT_FOUND
 */
solClient_dllExport solClient_returnCode_t
solClient_msg_getReplicationGroupMessageId(solClient_opaqueMsg_pt msg_p,
                                           solClient_replicationGroupMessageId_pt rgmid_p,
                                           size_t size);
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* SOLCLIENTMSGBUFFER_H */
