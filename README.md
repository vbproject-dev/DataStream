# DataStream

Binary data serialization and deserialization utilities.

```text
IDataWriter
├── DataOutputStream       Buffered memory writer
└── SocketOutputStream     Live socket writer

IDataReader
├── DataInputStream        Buffered memory reader
└── SocketInputStream      Live socket reader
```

All primitive values use **big-endian (network byte order)** encoding.

---

## DataOutputStream

`DataOutputStream` provides buffered serialization of primitive types, strings, and byte arrays into memory.

### Write Methods

| Method                                       | Description                                            |
| -------------------------------------------- | ------------------------------------------------------ |
| `writeByte(int8_t v)`                        | Write a signed 8-bit integer                           |
| `writeSignedByte(int8_t v)`                  | Write a signed 8-bit integer (alias of `writeByte`)    |
| `writeUnsignedByte(uint8_t v)`               | Write an unsigned 8-bit integer                        |
| `writeShort(int16_t v)`                      | Write a signed 16-bit integer in big-endian order      |
| `writeUnsignedShort(uint16_t v)`             | Write an unsigned 16-bit integer in big-endian order   |
| `writeInt(int32_t v)`                        | Write a signed 32-bit integer in big-endian order      |
| `writeUnsignedInt(uint32_t v)`               | Write an unsigned 32-bit integer in big-endian order   |
| `writeLong(int64_t v)`                       | Write a signed 64-bit integer in big-endian order      |
| `writeFloat(float v)`                        | Write a 32-bit IEEE 754 floating-point value           |
| `writeDouble(double v)`                      | Write a 64-bit IEEE 754 floating-point value           |
| `writeBoolean(bool v)`                       | Write a boolean as one byte (`0x01` or `0x00`)         |
| `writeUTF(const std::string& s)`             | Write a UTF-8 string prefixed with a `uint16_t` length |
| `writeBytes(const std::string& s)`           | Write raw bytes without a length prefix                |
| `writeArray(const std::vector<int8_t>& arr)` | Write an array of bytes                                |

### Output Methods

| Method                | Description                                          |
| --------------------- | ---------------------------------------------------- |
| `toString() const`    | Return the serialized data as a `std::string`        |
| `toByteArray() const` | Return the serialized data as `std::vector<uint8_t>` |
| `size() const`        | Return the number of bytes currently written         |

### Example

```cpp
DataOutputStream output;

output.writeByte(10);
output.writeInt(12345);
output.writeBoolean(true);
output.writeUTF("Hello");

std::string data = output.toString();
```

The resulting data can be sent directly through a socket or passed to `DataInputStream` for deserialization.


# DataInputStream

`DataInputStream` provides buffered deserialization of binary data. It reads data in **big-endian (network byte order)** and maintains an internal read position.

## Constructors

| Constructor                                                   | Description                                |
| ------------------------------------------------------------- | ------------------------------------------ |
| `DataInputStream(const std::string& data)`                    | Create a data stream from a `std::string`. |
| `DataInputStream::fromArray(const std::vector<uint8_t>& arr)` | Create a data stream from a byte array.    |

## Read Methods

| Method                | Description                                                                         |
| --------------------- | ----------------------------------------------------------------------------------- |
| `read()`              | Read the next byte as an `int`. Returns `-1` when the end of the stream is reached. |
| `readByte()`          | Read a signed 8-bit integer (`int8_t`).                                             |
| `readUnsignedByte()`  | Read an unsigned 8-bit integer (`uint8_t`).                                         |
| `readBoolean()`       | Read a boolean value.                                                               |
| `readShort()`         | Read a signed 16-bit integer (`int16_t`) in big-endian order.                       |
| `readUnsignedShort()` | Read an unsigned 16-bit integer (`uint16_t`) in big-endian order.                   |
| `readInt()`           | Read a signed 32-bit integer (`int32_t`) in big-endian order.                       |
| `readUnsignedInt()`   | Read an unsigned 32-bit integer (`uint32_t`) in big-endian order.                   |
| `readLong()`          | Read a signed 64-bit integer (`int64_t`) in big-endian order.                       |
| `readFloat()`         | Read a 32-bit IEEE 754 floating-point value.                                        |
| `readDouble()`        | Read a 64-bit IEEE 754 floating-point value.                                        |
| `readUTF()`           | Read a UTF-8 string prefixed with a `uint16_t` length.                              |
| `readBytes(size_t n)` | Read `n` raw bytes and return them as a `std::string`.                              |
| `readFully(size_t n)` | Read `n` bytes and return them as `std::vector<int8_t>`.                            |
| `readLine()`          | Read bytes until `'\n'`. Carriage returns (`'\r'`) are omitted.                     |

## Position Control

| Method                | Description                                                         |
| --------------------- | ------------------------------------------------------------------- |
| `available()`         | Return the number of unread bytes remaining in the stream.          |
| `mark()`              | Mark the current read position.                                     |
| `reset()`             | Reset the read position to the previously marked position.          |
| `skipBytes(size_t n)` | Skip up to `n` bytes and return the actual number of bytes skipped. |

## Byte Order

All multi-byte numeric values use **big-endian (network byte order)**.

For example, writing:

```cpp
DataOutputStream output;

output.writeInt(0x12345678);
```

produces:

```text
12 34 56 78
```

The corresponding read operation:

```cpp
DataInputStream input(output.toString());

int32_t value = input.readInt();
```

returns:

```text
0x12345678
```

## Example

```cpp
DataOutputStream output;

output.writeByte(10);
output.writeUnsignedShort(500);
output.writeInt(123456);
output.writeBoolean(true);
output.writeUTF("Hello");

DataInputStream input(output.toString());

int byteValue = input.read();
uint16_t shortValue = input.readUnsignedShort();
int32_t intValue = input.readInt();
bool enabled = input.readBoolean();
std::string message = input.readUTF();
```

`DataInputStream` is the in-memory counterpart to `SocketInputStream`: it reads from an existing byte buffer rather than directly from a socket.


