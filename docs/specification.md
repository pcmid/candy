# Specification

- [Index](index.md)
- [Design](design.md)
- Specification

## Handshake Message

The handshake message is sent from the client to the server, and the server does not generate any response whether the authentication is passed or not.

- Type field, used to distinguish the packet type, always 0
- TUN IP field, used to establish a mapping between the destination address and WebSocket
- Timestamp field, used to check the handshake time and avoid replay attacks
- Hash field, used for identity authentication, generated by password, TUN IP and Timestamp
- Confusion field, used to hide the fixed-length feature of the handshake packet, occupy a random length, and fill with random content

```plaintext
+--------------------------------------------------------------------------------------------------+
| Type (1 Byte) | TUN IP (4 Bytes) | Timestamp (8 Bytes) | Hash (32 Bytes) | Confusion (n Byte[s]) |
+--------------------------------------------------------------------------------------------------+
```

Type is 0. TUN IP and Timestamp are network sequences. Hash value is calculated from password, TUN IP and Timestamp through SHA256.


## IP Packet Forwarding Message

The client and the server send each other, the client forwards the data read from TUN to the server, and forwards the data received from the server to TUN. After receiving the data packet, the server analyzes the source address IP, verifies whether the mapping relationship between IP and Websocket has been established, obtains the destination address IP, and forwards the data packet to the corresponding WebSocket.

- Type field, used to distinguish the packet type, always 1
- Raw field, IP layer raw data

```plaintext
+-------------------------------+
| Type (1 Byte) | Raw (n Bytes) |
+-------------------------------+
```

## Dynamic Address Message

The dynamic address message is used by the client to request the available address from the server.

- Type field, used to distinguish the packet type, always 2
- Timestamp field, same as Handshake Message
- CIDR field, a '\0' terminated string.
- Hash field, same as Handshake Message
- Confusion field, same as Handshake Message

```plaintext
+-------------------------------------------------------------------------------------------------+
| Type (1 Byte) | Timestamp (8 Bytes) | CIDR (32 Bytes) | Hash (32 Bytes) | Confusion (n Byte[s]) |
+-------------------------------------------------------------------------------------------------+
```
