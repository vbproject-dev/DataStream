#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <utility>

// =====================================================================
// BitUtils - shared binary codec, used by every stream implementation
// so packing/unpacking logic exists in exactly one place.
// =====================================================================
class BitUtils {
public:
    static std::string packInt8(int8_t v);
    static std::string packUInt8(uint8_t v);
    static std::string packInt16(int16_t v);
    static std::string packUInt16(uint16_t v);
    static std::string packInt32(int32_t v);
    static std::string packUInt32(uint32_t v);
    static std::string packInt64(int64_t v);
    static std::string packFloat(float v);
    static std::string packDouble(double v);
    static std::string packBoolean(bool v);
    static std::string packUTF(const std::string& s);

    static std::pair<int8_t, size_t>   unpackInt8(const std::string& data, size_t pos);
    static std::pair<uint8_t, size_t>  unpackUInt8(const std::string& data, size_t pos);
    static std::pair<int16_t, size_t>  unpackInt16(const std::string& data, size_t pos);
    static std::pair<uint16_t, size_t> unpackUInt16(const std::string& data, size_t pos);
    static std::pair<int32_t, size_t>  unpackInt32(const std::string& data, size_t pos);
    static std::pair<uint32_t, size_t> unpackUInt32(const std::string& data, size_t pos);
    static std::pair<int64_t, size_t>  unpackInt64(const std::string& data, size_t pos);
    static std::pair<float, size_t>    unpackFloat(const std::string& data, size_t pos);
    static std::pair<double, size_t>   unpackDouble(const std::string& data, size_t pos);
    static std::pair<bool, size_t>     unpackBoolean(const std::string& data, size_t pos);
    static std::pair<std::string, size_t> unpackUTF(const std::string& data, size_t pos);

    static std::vector<uint8_t> toByteArray(const std::string& data);
    static std::string fromByteArray(const std::vector<uint8_t>& arr);
};

// =====================================================================
// IDataWriter - contract for anything that can serialize primitives.
// Both a buffered stream and a live socket implement this directly;
// neither needs to inherit from the other or override an unrelated
// concrete class.
// =====================================================================
class IDataWriter {
public:
    virtual ~IDataWriter() = default;

    virtual void writeByte(int8_t v) = 0;
    virtual void writeSignedByte(int8_t v) = 0;
    virtual void writeUnsignedByte(uint8_t v) = 0;
    virtual void writeShort(int16_t v) = 0;
    virtual void writeUnsignedShort(uint16_t v) = 0;
    virtual void writeInt(int32_t v) = 0;
    virtual void writeUnsignedInt(uint32_t v) = 0;
    virtual void writeLong(int64_t v) = 0;
    virtual void writeFloat(float v) = 0;
    virtual void writeDouble(double v) = 0;
    virtual void writeBoolean(bool v) = 0;
    virtual void writeUTF(const std::string& s) = 0;
    virtual void writeBytes(const std::string& s) = 0;
    virtual void writeArray(const std::vector<int8_t>& arr) = 0;

    virtual void flush() {}
    virtual void close() {}
};

// =====================================================================
// IDataReader - contract for anything that can deserialize primitives.
// =====================================================================
class IDataReader {
public:
    virtual ~IDataReader() = default;

    virtual int read() = 0;
    virtual int8_t readByte() = 0;
    virtual uint8_t readUnsignedByte() = 0;
    virtual int8_t readSignByte() { return readByte(); }
    virtual bool readBoolean() = 0;
    virtual int16_t readShort() = 0;
    virtual uint16_t readUnsignedShort() = 0;
    virtual int32_t readInt() = 0;
    virtual uint32_t readUnsignedInt() = 0;
    virtual int64_t readLong() = 0;
    virtual float readFloat() = 0;
    virtual double readDouble() = 0;
    virtual std::string readUTF() = 0;
    virtual std::string readBytes(size_t n) = 0;
    virtual std::vector<int8_t> readFully(size_t n) = 0;
    virtual size_t available() const = 0;
    virtual void close() {}
};

// =====================================================================
// DataOutputStream - IDataWriter backed by an in-memory buffer.
// =====================================================================
class DataOutputStream : public IDataWriter {
public:
    DataOutputStream() = default;

    void writeByte(int8_t v) override;
    void writeSignedByte(int8_t v) override;
    void writeUnsignedByte(uint8_t v) override;
    void writeShort(int16_t v) override;
    void writeUnsignedShort(uint16_t v) override;
    void writeInt(int32_t v) override;
    void writeUnsignedInt(uint32_t v) override;
    void writeLong(int64_t v) override;
    void writeFloat(float v) override;
    void writeDouble(double v) override;
    void writeBoolean(bool v) override;
    void writeUTF(const std::string& s) override;
    void writeBytes(const std::string& s) override;
    void writeArray(const std::vector<int8_t>& arr) override;

    std::string toString() const;
    std::vector<uint8_t> toByteArray() const;
    size_t size() const;

protected:
    std::vector<std::string> _buf;
};

// =====================================================================
// DataInputStream - IDataReader backed by an in-memory buffer.
// =====================================================================
class DataInputStream : public IDataReader {
public:
    explicit DataInputStream(const std::string& data = "");
    static DataInputStream fromArray(const std::vector<uint8_t>& arr);

    void mark();
    void reset();
    size_t skipBytes(size_t n);
    std::string readLine();

    int read() override;
    int8_t readByte() override;
    uint8_t readUnsignedByte() override;
    bool readBoolean() override;
    int16_t readShort() override;
    uint16_t readUnsignedShort() override;
    int32_t readInt() override;
    uint32_t readUnsignedInt() override;
    int64_t readLong() override;
    float readFloat() override;
    double readDouble() override;
    std::string readUTF() override;
    std::string readBytes(size_t n) override;
    std::vector<int8_t> readFully(size_t n) override;
    size_t available() const override;

protected:
    std::string _read;
    size_t _pos = 1;   // 1-based: matches BitUtils pack/unpack convention
    size_t _mark = 0;
};

// =====================================================================
// SocketReader / SocketWriter - raw transport abstraction
// =====================================================================
class SocketReader {
public:
    virtual ~SocketReader() = default;
    virtual int read(uint8_t* buffer, int len) = 0;
    virtual bool isConnected() const = 0;
    virtual void close() = 0;
};

class SocketWriter {
public:
    virtual ~SocketWriter() = default;
    virtual bool write(const uint8_t* data, int len) = 0;
    virtual bool isConnected() const = 0;
    virtual void close() = 0;
    virtual void flush() = 0;
};

// =====================================================================
// SocketInputStream - IDataReader backed by a live socket, reusable
// with any SocketReader implementation.
// =====================================================================
class SocketInputStream : public IDataReader {
public:
    explicit SocketInputStream(SocketReader* reader);

    int read() override;
    int8_t readByte() override;
    uint8_t readUnsignedByte() override;
    bool readBoolean() override;
    int16_t readShort() override;
    uint16_t readUnsignedShort() override;
    int32_t readInt() override;
    uint32_t readUnsignedInt() override;
    int64_t readLong() override;
    float readFloat() override;
    double readDouble() override;
    std::string readUTF() override;
    std::string readBytes(size_t n) override;
    std::vector<int8_t> readFully(size_t n) override;
    size_t available() const override;
    void close() override;
    bool isConnected() const;

private:
    void ensureBytes(size_t n);

    SocketReader* _reader = nullptr;
    std::string _buffer;
    size_t _bufPos = 1;   // 1-based: matches BitUtils pack/unpack convention
};

// =====================================================================
// SocketOutputStream - IDataWriter backed by a live socket, reusable
// with any SocketWriter implementation. Writes straight through
// instead of buffering, so it implements IDataWriter directly rather
// than inheriting (and overriding all of) DataOutputStream.
// =====================================================================
class SocketOutputStream : public IDataWriter {
public:
    explicit SocketOutputStream(SocketWriter* writer);

    void writeByte(int8_t v) override;
    void writeSignedByte(int8_t v) override;
    void writeUnsignedByte(uint8_t v) override;
    void writeShort(int16_t v) override;
    void writeUnsignedShort(uint16_t v) override;
    void writeInt(int32_t v) override;
    void writeUnsignedInt(uint32_t v) override;
    void writeLong(int64_t v) override;
    void writeFloat(float v) override;
    void writeDouble(double v) override;
    void writeBoolean(bool v) override;
    void writeUTF(const std::string& s) override;
    void writeBytes(const std::string& s) override;
    void writeArray(const std::vector<int8_t>& arr) override;

    void flush() override;
    void close() override;
    bool isConnected() const;

private:
    SocketWriter* _writer = nullptr;
};