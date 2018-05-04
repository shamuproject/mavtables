Configuration {#configuration}
==============================

* [MAVLink Addressing](#mavlink-addressing)
  * [MAVLink Subnets](#mavlink-subnets)
* [Basic Grammar](#basic-grammar)
  * [Identifiers](#identifiers)
  * [Statements](#statements)
  * [Blocks](#blocks)
  * [Comments](#comments)
* [Global Settings](#global-settings)
* [udp block](#udp-block)
  * [port statement](#port-statement)
  * [address statement](#address-statement)
  * [max_bitrate statement](#max_bitrate-statement)
* [serial block](#serial-block)
  * [device statement](#device-statement)
  * [baudrate statement](#baudrate-statement)
  * [flow_control statement](#flow_control-statement)
  * [preload statement](#preload-statement)
* [chain block](#chain-block)
  * [Rules](#rules)
  * [Action](#action)
  * [Priority](#priority)
  * [Condition](#condition)
    * [\<packet type\>](#packet-type)
    * [\<source address\>](#source-address)
    * [\<dest address\>](#dest-address)


mavtables' configuration files are used to define default actions, interfaces,
and filter chains.  A full example configuration file can be found at
[`../examples/mavtables.conf`](../examples/mavtables.conf).


# MAVLink Addressing

A MAVLink address consists of 2 octets.  The first being the system ID and the
second the component ID.  Therefore, a MAVLink address looks like half of an
IPv4 address.
```
<system ID>.<component ID>
```


## MAVLink Subnets

MAVLink subnets are similar to IP subnets and thus it is recommended that the
reader understand IP subnets before continuing.

 There are four representations of MAVLink subnets:
 
   1. "\<System ID>.<Component ID\>:\<System ID mask\>.\<Component ID mask\>"
   2. "\<System ID>.<Component ID\>/\<bits\>"
   3. "\<System ID>.<Component ID\>\\<bits\>"
   4. "\<System ID>.<Component ID\>"
 
 The first form is self explanatory, but the 2nd and 3rd are not as simple.
 In the 2nd case the number of bits (0 - 16) is the number of bits from the
 left that must match for an address to be in the subnet.  The 3rd form is
 like the 2nd, but does not require the system ID (first octet) to match and
 starts with the number of bits of the component ID (0 - 8) that must match
 from the left for an address to be in the subnet.  The last form is
 shorthand for "<System ID>.<Component ID>/16", that is an exact match.
 
 Below is a table relating the slash postfix to the subnet mask in \<System
 mask\>.\<Component mask\> notation.
 
 | Mask with `/` | Mask with `\` | Postfix (\#bits) |
 | -------------:| -------------:| ----------------:|
 |       255.255 |  out of range |               16 |
 |       255.254 |  out of range |               15 |
 |       255.252 |  out of range |               14 |
 |       255.248 |  out of range |               13 |
 |       255.240 |  out of range |               12 |
 |       255.224 |  out of range |               11 |
 |       255.192 |  out of range |               10 |
 |       255.128 |  out of range |                9 |
 |         255.0 |         0.255 |                8 |
 |         254.0 |         0.254 |                7 |
 |         252.0 |         0.252 |                6 |
 |         248.0 |         0.248 |                5 |
 |         240.0 |         0.240 |                4 |
 |         224.0 |         0.224 |                3 |
 |         192.0 |         0.192 |                2 |
 |         128.0 |         0.128 |                1 |
 |           0.0 |             0 |                0 |


# Basic Grammar

There are four major types of grammar elements in configuration files.  These
are identifiers, statements, blocks, and comments.  Configuration files have
a similar syntax to the C programming language and like C they are not
whitespace sensitive.


## Identifiers

An identifier is a name containing any combination of letters (upper and lower),
digits, and the underscores ('\_').  However, it must not start with a number.
This is the same definition as identifiers in the C programming language.


## Statements

A statement is defined as an identifier, followed by a value, and then the *end
of statement* character '`;`'.  Therefore, it takes the form of
```
<identifier> <value>;
```
An example would be:
```
boadrate 57600;
```
where '`boadrate`' is the identifier, '`57600`' is the value (in this case
a number) and it ends with `;`.


## Blocks

A block is a sectioning construct that can contain one or more statements, or in
the special case of filter blocks, a rule.  Blocks consist of a type and
optionally an identifier followed by '`{`', one or more statements (or rules)
and finally ending in '`}`'.  The format is:
```
<type> <optional identifer> {
    <statement 1>;
    <statement 2>;
    <statement ...>;
    <statement n>;
}
```
An example would be:
```
udp {
    port 14555;
    address 127.0.0.1;
}
```


## Comments

The comment character is '`#`'.  Everything after this character to the end of
the line is considered a comment and will be ignored.  An example of using
comments is:
```
udp {  # UDP Inteface
    port 14555;         # port number, the default is 14500
    address 127.0.0.1;  # listen on localhost only, the default is any address
}
```



# Global Settings

The following sections document the components of a configuration file.

## default_action statement

Set the default action to take when the filter does not determine what to do
with a packet.  The options are:
* `accept` - to accept packets by default
* `reject` - to reject packets by default

To accept packets by default use:
```
default_action accept;
```
To reject packets by default use:
```
default_action reject;
```



# udp block

The `udp` block defines a UDP interface to listen for connections on.  An
example is:
```
udp {
    port 14555;
    address 127.0.0.1;
    max_bitrate 8388608;
}
```

There is no limit to the number of UDP interfaces that can be defined.


## port statement

A statement that sets the port number to listen on.  The format is:
```
port <port number>;
```

An example is:
```
port 14555;
```

This is the only required statement in a `udp` block.


## address statement (optional)

A statement that sets the IP address to listen on.  The format is:
```
address <IP v4 address>;
```

An example is:
```
address 127.0.0.1;
```
which restricts mavtables to only listening for connections from `localhost`.

If not provided the default is to listen on every address and therefore to
accept connections from remote systems (subject to the firewall's running on the
host operating system).


## max_bitrate statement (optional)

A statement that sets the bitrate limit (in bits per second) when transmitting
packets over UDP.  The format is:
```
max_bitrate <maximum bitrate>
```

An example is:
```
max_bitrate 8388608;  # 8 Mbps
```

If not provided there will not be any limit on the rate of packet transmission
over UDP.

If downstream components tend to lose packets due to the operating system's UDP
buffers filling up, consider slowing down mavtables by providing a bitrate
limit.  This feature was added because mavtables is much faster than components
written in interpreted languages such as Python and thus must be limited so it
can store packets in it's own buffers to avoid overflowing the operating system
buffers.



# serial block

The `serial` block defines a serial port interface to listen for connections on.
An example is:
```
serial {
    device /dev/ttyUSB0;
    baudrate 115200;
    flow_control yes;
    preload 1.1;
}
```

There is no limit to the number of serial port interfaces that can be defined.


## device statement

A statement that sets the serial device to listen on.  The format is:
```
device <device string>;
```
where the `<device string>` is the path to the serial device.

An example is:
```
device /dev/ttyUSB0;
```

This is the only required statement in a `serial` block.


## baudrate statement (optional)

A statement that sets the bitrate (in bits per second) of the serial port.  The
format is:
```
baudrate <bitrate>;
```

An example is:
```
baudrate 115200;
```
which sets the baudrate to 112.5 kbps.

If not provided the default is to use a baudrate of 9600 (9.4 kbps)


## flow_control statement (optional)

A statement that enables/disables hardware flow control on the serial port.  The
format is:
```
flow_control <yes/no>;
```

To turn on hardware flow control:
```
flow_control yes;
```

To turn off hardware flow control:
```
flow_control no;
```

If not provided the default is to disable hardware flow control.

## preload statement (optional)

A statement that can preload a MAVLink address on the serial port.  Typically
mavtables learns about a component when that component sends a packet to the
router.  In some cases a component may not send a packet until it receives one,
in this case the component's address can be preloaded.

One particular case is when two mavtables are in use.  Neither instance knows
about the other because mavtables will not send any packets over a serial port
if nothing has been sent to it over said serial port.  By preloading addresses
on both instances for the serial port connection they share, the packets from
other components will be sent to the other mavtables instance and thus the
instances can discover each other's components.

The format is:
```
preload <MAVLink address>;
```

An example is:
```
preload 1.1;
```
This indicates that system 1 and component 1 can be reached on this serial port,
even before that component sends a packet to the router.

Any number of `preload` statements are allowed.  Every address listed will be
added to the serial port.



# chain block

A `chain` block defines a filter chain consisting of one or more rules.  Filter
chains are used to define firewall rules and collect them into groups.  The
format is:
```
chain <chain name> {
    <rule 1>;
    <rule 2>;
    <rule ...>;
    <rule n>;
}
```

An example is:
```
chain default
{
    reject if ENCAPSULATED_DATA;
    accept;
}
```
which rejects the `ENCAPSULATED_DATA` packet, but accepts all others.

The '`default`' chain is a reserved name and it must exist.  The '`default`'
chain is the one used to filter packets before re-transmitting them.  All other
chains are called from the default chain or another chain via the '`call`' or
'`goto`' rules.

There is no limit to the number of filter chains, or to the number of rules in
a filter chain.


## Rules

A rule has the form
```
<action> <optional priority> <optional condition>;
```
an example is
```
accept with priority 3 if ENCAPSULATED_DATA from 192.168 to 172.0/8;
```

When evaluating the filter, the first rule a packet matches will decide the fate
of the packet.  If a condition is not provided the rule will match all packets,
otherwise the condition decides whether the rule matches or not.

### Action

There are 4 types of actions a rule can have.

* `accept` - accept the packet
* `reject` - reject the packet
* `call <chain name>` - delegate accept/reject decision to another chain,
    `<chain name>`.  Returns to parent chain if no match in the target chain.
* `goto <chain name>` - delegate accept/reject decision to another chain,
    `<chain name>`.  If the target chain never matches the packet then the
    default action, set in `default_action` is used.

### Priority

The priority form is:
```
with priority <priority level>
```
where `<priority level>` is a positive or negative integer.  An example is:
```
with priority -4
```

A greater integer indicates a higher priority.  Packets have a priority of 0 by
default.  If communications slow down and mavtables starts to build a queue of
packets to send it will send out the higher priority packets first.

### Condition

A condition determines whether or not a packet and the address it is being sent
to matches a rule.  Conditions have the form:
```
if <packet type> from <source address> to <dest address>;
```
where `<packet type>`, `from <source address>`, and `to <dest address>` are all
optional, but one is required, otherwise the rule should not have a condition at
all.

#### <packet type>

The name of the packet type to match (upper case).  If left out all packet types
will match.

#### <source address>

A MAVLink subnet to test for containment of the source MAVLink address of the
packet.

#### <dest address>

A MAVLink subnet to test for containment of the destination MAVLink address of
the packet.  The destination address is not regarded as the destination
contained in the packet (as that may not exist) but is the MAVLink address of
where mavtables is attempting to send the packet.
