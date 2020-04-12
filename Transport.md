# TCP Model

## Class TcpSocket
Base class of all TcpSockets
Only hosts TcpSocket attributes that can be resused different implementations

## Class TcpSocketFactory
Used by applications to create TCP sockets
Holds global default variables used to initialize newly created sockets

## Tcp Socket Class Attributes
- SndBufSize: TcpSocket maximum transmit buffer size (bytes) > cwnd
  - ns3::UintegerValue
  - uint32_t (init value: 131072)

- RcvBufSize: TcpSocket maximum receive buffer size (bytes) > rwnd
  - ns3::UintegerValue
  - uint32_t (init value: 131072)

- SegmentSize: TCP maximum segment size (bytes)
  - ns3::UintegerValue
  - uint32_t (init value: 536)

- InitialSlowStartThreshold: TCP slow start threshold (bytes) > ssthresh
  - ns3::UintegerValue
  - uint32_t (init value: 65535)

- InitialCwnd: TCP initial congestion window size (segments) > IW (Initial Window)
  - ns3::UintegerValue
  - uint32_t (init value: 1)

-----

# UDP Model

## Class UdpSocket
Base class of all UdpSockets

## Class UdpSocketFactory
Used by applications to create UDP sockets

## Udp Socket Class Attributes
- RcvBufSize: Udp Socket maximum receive buffer size (bytes)
  - ns3::UintegerValue
  - uint32_t (init value: 131072)

- IpTtl: socket-specific TTL for unicast IP packets (if non-zero)
  - ns3::UintegerValue
  - uint8_t (init value: 0)

- IpMulticastTtl: socket-specific TTL for multicast IP packets (if non-zero)
  - ns3::UintegerValue
  - uint8_t (init value: 0)
