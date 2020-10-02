We use lightweight VPN to access internal resources. It's just to bypass NAT and keep IP addresses static. Nothing more.

`⚠️ WARNING ⚠️` There is no security restrictions in VPN network. Use your own secuity tools to protect yourself: firewals, strong password, SSH keys and so on. 

# Zero Tier

[Zero Tier](https://www.zerotier.com/) — easy to use VPN replacement for all platforms. No configuration required, only one command to join network. Works great behind NAT and firewalls. Each node will keep their IP address static forever after join to network.

## Install 

Get package for your platform: https://www.zerotier.com/download/  Linux, Windows, macOS and even iOS and Android supported.

## Join network 

Join our developers network: 

`sudo zerotier-cli join b6079f73c697cbc4`

## Ask for approve

Ask @zhovner to approve your node. After few seconds after approve, you will get access to our network:

```
IPv4 network: 172.25.0.0/16
IPv6 network: fdb6:079f:73c6:97cb:c499:93__:____:___
```
