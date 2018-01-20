# klibs (unstable)

WARNING: **While we attempt to make klibs as stable and secure as possible, as a C library, it may be vulnerable to one or more memory-related RCE exploits. This is not to be taken lightly.**

Utilities include:
  - Cross platform TCP socket API, with support for TLS encryption
  - UDP datagram send and receive
  - Cross platform mutexes
  - HTTP(S) client
  - TweetNaCl cryptography and random number generation
  - Byte array manipulation helpers

Utilities coming soon:
  - High-speed hash map
  - Multithreading
  - Concurrent item queue (similar to Go channel)
  - ICMP ping

# Contributing

Encouraged contributions:

- Don't negatively impact speed
- Increase runtime speed
- Are readable
- Don't add additional dependencies
- Have commits that are neatly separated and properly named
- Work across all supported platforms (Linux, Darwin & Windows)

