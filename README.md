mtkeepmgr: Mediatek EEPROM manager
==================================

This is a utility to dump, parse and print the EEPROM content of Mediatek based wireless equipment (NICs, APs, etc.).

The following EEPROM content access methods are supported:
* via USB (using libusb and vendor specific USB device commands)
* file dump

Building
--------

Utility building as simple as typing `make` on average system. Builing requirements:
* gcc
* GNU make
* pkg-config (optional, used only to build with libusb support)
* libusb (optional, allows accessing USB devices)

Usage examples
--------------

**mtkeepmgr** contains a detailed usage info, in any ambiguous case just type *mtkeepmgr -h*.

### Work with saved EEPROM dumps

#### Print contents of an EEPROM dump file

To check device configuration using earlier saved EEPROM data file dump.bin just type:

```
$ mtkeepmgr -F dump.bin
```

### USB dongle handling

When linking with *libusb* the utility provide few useful options for USB dongle work analysis or debugging. **mtkeepmgr** supports multiple ways to specify target USB device, see the utility usage info for details.

#### Print EEPROM configuration of first USB dongle

It is quite common to have only one USB dongle connected to your host. If USB VID/PID of this dongle is known to **mtkeepmgr** then use the following command to obtain device EEPROM configuration:

```
$ mtkeepmgr -U any
```

#### Print or save EEPROM configuration of a specific USB device

If you have multiple devices connected to your host, then you would like to specify exact device to interact with. Lets say that your device have address #123 on USB bus #3. Then you could you the following command to access target device:

```
$ mtkeepmgr -U 3:123
```

**mtkeepmgr** is able to save EEPROM data for future analysis. E.g. to save EEPROM data to *dump.bin* file:

```
$ mtkeepmgr -U 3:123 save dump.bin
```

#### Save EEPROM data of a new USB device

Sometime you could face USB device that is not known to **mtkeepmgr**. Normally **mtkeepmgr** will reject to interact with unknown USB devices. But if you certanly know that the new USB device is based on a known chip model, then you could force **mtkeepmgr** to work with it. Lets assume that the device from the previous example has USB identifiers 0x0123/0xabcd (VID/PID), then command to save EEPROM data of such device will be:

```
$ mtkeepmgr -U 3:123,0123:abcd save dump.bin
```

#### Print EEPROM configuration of a device in a specific USB port

If you periodically disconnect and connect again your USB dongle, then it bus address will change. In this case, it becomes impractical to specifying device by its address. It becomes better to specify device by its connection port and **mtkeepmgr** support this.

Lets say that you have a couple of cascaded USB hubs and a bunch of USB dongles that are connected to the second USB hub. Lets assume that the first hub is connected to a host USB bus #3 port #2 and then the second USB hub connected to the 4th port of the first hub. And also lets assume that the target USB device is connected to the 1st port of the second USB hub. To access the target device type device path starting from USB bus number and then list ports one by one from root toward the target device delimiting item ports with slash:

```
$ mtkeepmgr -U 3/2/4/1
```

If USB hubs in above example have only one connected USB dongle, then you could save your time by specifying only the path prefix till the first hub:

```
$ mtkeepmgr -U 3/2/
```

Note the trailing slash that indicates that the path **prefix** was specified.

License
-------

This project is licensed under the terms of the ISC license. See the LICENSE file for license rights and limitations.
