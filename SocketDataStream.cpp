#include "SocketDataStream.hpp"
#include <cstring>
#include <algorithm>
#include <stdexcept>


std::string BitUtils::packInt8(int8_t v) {
    return std::string(1, static_cast<char>(v));
}

std::string BitUtils::packUInt8(uint8_t v) {
    return std::string(1, static_cast<char>(v));
}

std::string BitUtils::packInt16(int16_t v) {
    std::string s;
    s.push_back(static_cast<char>((v >> 8) & 0xFF));
    s.push_back(static_cast<char>(v & 0xFF));
    return s;
}

std::string BitUtils::packUInt16(uint16_t v) {
    return packInt16(static_cast<int16_t>(v));
}

std::string BitUtils::packInt32(int32_t v) {
    std::string s;
    s.push_back(static_cast<char>((v >> 24) & 0xFF));
    s.push_back(static_cast<char>((v >> 16) & 0xFF));
    s.push_back(static_cast<char>((v >> 8) & 0xFF));
    s.push_back(static_cast<char>(v & 0xFF));
    return s;
}

std::string BitUtils::packUInt32(uint32_t v) {
    return packInt32(static_cast<int32_t>(v));
}

std::string BitUtils::packInt64(int64_t v) {
    std::string s;
    s.push_back(static_cast<char>((v >> 56) & 0xFF));
    s.push_back(static_cast<char>((v >> 48) & 0xFF));
    s.push_back(static_cast<char>((v >> 40) & 0xFF));
    s.push_back(static_cast<char>((v >> 32) & 0xFF));
    s.push_back(static_cast<char>((v >> 24) & 0xFF));
    s.push_back(static_cast<char>((v >> 16) & 0xFF));
    s.push_back(static_cast<char>((v >> 8) & 0xFF));
    s.push_back(static_cast<char>(v & 0xFF));
    return s;
}

std::string BitUtils::packFloat(float v) {
    uint32_t bits;
    memcpy(&bits, &v, sizeof(float));
    return packUInt32(bits);
}

std::string BitUtils::packDouble(double v) {
    uint64_t bits;
    memcpy(&bits, &v, sizeof(double));
    return packInt64(static_cast<int64_t>(bits));
}

std::string BitUtils::packBoolean(bool v) {
    return std::string(1, static_cast<char>(v ? 1 : 0));
}

std::string BitUtils::packUTF(const std::string& s) {
    std::string result;
    result += packUInt16(static_cast<uint16_t>(s.length()));
    result += s;
    return result;
}

std::pair<int8_t, size_t> BitUtils::unpackInt8(const std::string& data, size_t pos) {
    if (pos > data.length()) throw std::out_of_range("unpackInt8");
    return { static_cast<int8_t>(data[pos - 1]), pos + 1 };
}

std::pair<uint8_t, size_t> BitUtils::unpackUInt8(const std::string& data, size_t pos) {
    if (pos > data.length()) throw std::out_of_range("unpackUInt8");
    return { static_cast<uint8_t>(data[pos - 1]), pos + 1 };
}

std::pair<int16_t, size_t> BitUtils::unpackInt16(const std::string& data, size_t pos) {
    if (pos + 1 > data.length()) throw std::out_of_range("unpackInt16");
    int16_t v = (static_cast<uint8_t>(data[pos - 1]) << 8) | static_cast<uint8_t>(data[pos]);
    return { v, pos + 2 };
}

std::pair<uint16_t, size_t> BitUtils::unpackUInt16(const std::string& data, size_t pos) {
    auto result = unpackInt16(data, pos);
    return { static_cast<uint16_t>(result.first), result.second };
}

std::pair<int32_t, size_t> BitUtils::unpackInt32(const std::string& data, size_t pos) {
    if (pos + 3 > data.length()) throw std::out_of_range("unpackInt32");
    int32_t v = (static_cast<uint8_t>(data[pos - 1]) << 24) |
        (static_cast<uint8_t>(data[pos]) << 16) |
        (static_cast<uint8_t>(data[pos + 1]) << 8) |
        static_cast<uint8_t>(data[pos + 2]);
    return { v, pos + 4 };
}

std::pair<uint32_t, size_t> BitUtils::unpackUInt32(const std::string& data, size_t pos) {
    auto result = unpackInt32(data, pos);
    return { static_cast<uint32_t>(result.first), result.second };
}

std::pair<int64_t, size_t> BitUtils::unpackInt64(const std::string& data, size_t pos) {
    if (pos + 7 > data.length()) throw std::out_of_range("unpackInt64");
    int64_t v = 0;
    for (int i = 0; i < 8; i++) {
        v = (v << 8) | static_cast<uint8_t>(data[pos - 1 + i]);
    }
    return { v, pos + 8 };
}

std::pair<float, size_t> BitUtils::unpackFloat(const std::string& data, size_t pos) {
    auto bits = unpackUInt32(data, pos);
    float v;
    memcpy(&v, &bits.first, sizeof(float));
    return { v, bits.second };
}

std::pair<double, size_t> BitUtils::unpackDouble(const std::string& data, size_t pos) {
    auto bits = unpackInt64(data, pos);
    double v;
    memcpy(&v, &bits.first, sizeof(double));
    return { v, bits.second };
}

std::pair<bool, size_t> BitUtils::unpackBoolean(const std::string& data, size_t pos) {
    auto result = unpackUInt8(data, pos);
    return { result.first != 0, result.second };
}

std::pair<std::string, size_t> BitUtils::unpackUTF(const std::string& data, size_t pos) {
    auto len = unpackUInt16(data, pos);
    if (pos + len.first > data.length()) throw std::out_of_range("unpackUTF");
    std::string s = data.substr(pos + 1, len.first);
    return { s, pos + 2 + len.first };
}

std::vector<uint8_t> BitUtils::toByteArray(const std::string& data) {
    return std::vector<uint8_t>(data.begin(), data.end());
}

std::string BitUtils::fromByteArray(const std::vector<uint8_t>& arr) {
    return std::string(arr.begin(), arr.end());
}

// =====================================================================
// DataOutputStream Implementation (IDataWriter, buffered)
// =====================================================================

void DataOutputStream::writeByte(int8_t v) { _buf.push_back(BitUtils::packInt8(v)); }
void DataOutputStream::writeSignedByte(int8_t v) { _buf.push_back(BitUtils::packInt8(v)); }
void DataOutputStream::writeUnsignedByte(uint8_t v) { _buf.push_back(BitUtils::packUInt8(v)); }
void DataOutputStream::writeShort(int16_t v) { _buf.push_back(BitUtils::packInt16(v)); }
void DataOutputStream::writeUnsignedShort(uint16_t v) { _buf.push_back(BitUtils::packUInt16(v)); }
void DataOutputStream::writeInt(int32_t v) { _buf.push_back(BitUtils::packInt32(v)); }
void DataOutputStream::writeUnsignedInt(uint32_t v) { _buf.push_back(BitUtils::packUInt32(v)); }
void DataOutputStream::writeLong(int64_t v) { _buf.push_back(BitUtils::packInt64(v)); }
void DataOutputStream::writeFloat(float v) { _buf.push_back(BitUtils::packFloat(v)); }
void DataOutputStream::writeDouble(double v) { _buf.push_back(BitUtils::packDouble(v)); }
void DataOutputStream::writeBoolean(bool v) { _buf.push_back(BitUtils::packBoolean(v)); }
void DataOutputStream::writeUTF(const std::string& s) { _buf.push_back(BitUtils::packUTF(s)); }
void DataOutputStream::writeBytes(const std::string& s) { _buf.push_back(s); }

void DataOutputStream::writeArray(const std::vector<int8_t>& arr) {
    for (auto v : arr) writeByte(v);
}

std::string DataOutputStream::toString() const {
    std::string result;
    for (const auto& s : _buf) result += s;
    return result;
}

std::vector<uint8_t> DataOutputStream::toByteArray() const {
    return BitUtils::toByteArray(toString());
}

size_t DataOutputStream::size() const {
    return toString().length();
}

// =====================================================================
// DataInputStream Implementation (IDataReader, buffered)
// =====================================================================

DataInputStream::DataInputStream(const std::string& data) : _read(data) {}

DataInputStream DataInputStream::fromArray(const std::vector<uint8_t>& arr) {
    return DataInputStream(BitUtils::fromByteArray(arr));
}

size_t DataInputStream::available() const {
    if (_pos > _read.length()) return 0;
    return _read.length() - _pos + 1;
}

void DataInputStream::mark() { _mark = _pos; }
void DataInputStream::reset() { if (_mark > 0) _pos = _mark; }

size_t DataInputStream::skipBytes(size_t n) {
    size_t skipped = std::min(n, available());
    _pos += skipped;
    return skipped;
}

int DataInputStream::read() {
    if (_pos > _read.length()) return -1;
    auto result = BitUtils::unpackUInt8(_read, _pos);
    _pos = result.second;
    return result.first;
}

int8_t DataInputStream::readByte() {
    auto result = BitUtils::unpackInt8(_read, _pos);
    _pos = result.second;
    return result.first;
}

uint8_t DataInputStream::readUnsignedByte() {
    auto result = BitUtils::unpackUInt8(_read, _pos);
    _pos = result.second;
    return result.first;
}

bool DataInputStream::readBoolean() {
    auto result = BitUtils::unpackBoolean(_read, _pos);
    _pos = result.second;
    return result.first;
}

int16_t DataInputStream::readShort() {
    auto result = BitUtils::unpackInt16(_read, _pos);
    _pos = result.second;
    return result.first;
}

uint16_t DataInputStream::readUnsignedShort() {
    auto result = BitUtils::unpackUInt16(_read, _pos);
    _pos = result.second;
    return result.first;
}

int32_t DataInputStream::readInt() {
    auto result = BitUtils::unpackInt32(_read, _pos);
    _pos = result.second;
    return result.first;
}

uint32_t DataInputStream::readUnsignedInt() {
    auto result = BitUtils::unpackUInt32(_read, _pos);
    _pos = result.second;
    return result.first;
}

int64_t DataInputStream::readLong() {
    auto result = BitUtils::unpackInt64(_read, _pos);
    _pos = result.second;
    return result.first;
}

float DataInputStream::readFloat() {
    auto result = BitUtils::unpackFloat(_read, _pos);
    _pos = result.second;
    return result.first;
}

double DataInputStream::readDouble() {
    auto result = BitUtils::unpackDouble(_read, _pos);
    _pos = result.second;
    return result.first;
}

std::string DataInputStream::readUTF() {
    auto result = BitUtils::unpackUTF(_read, _pos);
    _pos = result.second;
    return result.first;
}

std::string DataInputStream::readBytes(size_t n) {
    if (_pos + n - 1 > _read.length()) throw std::out_of_range("readBytes");
    std::string s = _read.substr(_pos - 1, n);
    _pos += n;
    return s;
}

std::vector<int8_t> DataInputStream::readFully(size_t n) {
    std::string s = readBytes(n);
    std::vector<int8_t> result;
    for (char c : s) result.push_back(static_cast<int8_t>(c));
    return result;
}

std::string DataInputStream::readLine() {
    std::string chars;
    while (_pos <= _read.length()) {
        char c = _read[_pos - 1];
        _pos++;
        if (c == '\n') break;
        if (c != '\r') chars += c;
    }
    return chars;
}

// =====================================================================
// SocketInputStream Implementation (IDataReader, live socket)
// =====================================================================

SocketInputStream::SocketInputStream(SocketReader* reader) : _reader(reader) {}

void SocketInputStream::ensureBytes(size_t n) {
    while (_buffer.length() - _bufPos + 1 < n) {
        if (!_reader || !_reader->isConnected()) {
            throw std::runtime_error("Socket not connected");
        }

        uint8_t buf[1024];
        size_t need = n - (_buffer.length() - _bufPos + 1);
        int toRead = static_cast<int>(std::min(sizeof(buf), need));
        int bytesRead = _reader->read(buf, toRead);

        if (bytesRead > 0) {
            _buffer.append(reinterpret_cast<char*>(buf), bytesRead);
        }
        else if (bytesRead == 0) {
            continue; // No data available yet, keep trying
        }
        else {
            throw std::runtime_error("Failed to read from socket");
        }
    }
}

int SocketInputStream::read() {
    try {
        ensureBytes(1);
        return readByte();
    }
    catch (...) {
        return -1;
    }
}

int8_t SocketInputStream::readByte() {
    ensureBytes(1);
    auto result = BitUtils::unpackInt8(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

uint8_t SocketInputStream::readUnsignedByte() {
    ensureBytes(1);
    auto result = BitUtils::unpackUInt8(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

bool SocketInputStream::readBoolean() {
    ensureBytes(1);
    auto result = BitUtils::unpackBoolean(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

int16_t SocketInputStream::readShort() {
    ensureBytes(2);
    auto result = BitUtils::unpackInt16(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

uint16_t SocketInputStream::readUnsignedShort() {
    ensureBytes(2);
    auto result = BitUtils::unpackUInt16(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

int32_t SocketInputStream::readInt() {
    ensureBytes(4);
    auto result = BitUtils::unpackInt32(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

uint32_t SocketInputStream::readUnsignedInt() {
    ensureBytes(4);
    auto result = BitUtils::unpackUInt32(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

int64_t SocketInputStream::readLong() {
    ensureBytes(8);
    auto result = BitUtils::unpackInt64(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

float SocketInputStream::readFloat() {
    ensureBytes(4);
    auto result = BitUtils::unpackFloat(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

double SocketInputStream::readDouble() {
    ensureBytes(8);
    auto result = BitUtils::unpackDouble(_buffer, _bufPos);
    _bufPos = result.second;
    return result.first;
}

std::string SocketInputStream::readUTF() {
    auto len = readUnsignedShort();
    return readBytes(len);
}

std::string SocketInputStream::readBytes(size_t n) {
    ensureBytes(n);
    std::string s = _buffer.substr(_bufPos - 1, n);
    _bufPos += n;
    return s;
}

std::vector<int8_t> SocketInputStream::readFully(size_t n) {
    std::string s = readBytes(n);
    std::vector<int8_t> result;
    for (char c : s) result.push_back(static_cast<int8_t>(c));
    return result;
}

size_t SocketInputStream::available() const {
    return _buffer.length() - _bufPos + 1;
}

void SocketInputStream::close() {
    if (_reader) {
        _reader->close();
        _reader = nullptr;
    }
}

bool SocketInputStream::isConnected() const {
    return _reader && _reader->isConnected();
}

// =====================================================================
// SocketOutputStream Implementation (IDataWriter, live socket)
// =====================================================================

SocketOutputStream::SocketOutputStream(SocketWriter* writer) : _writer(writer) {}

void SocketOutputStream::writeByte(int8_t v) {
    if (!_writer || !_writer->isConnected()) {
        throw std::runtime_error("Socket not connected");
    }
    uint8_t data = static_cast<uint8_t>(v);
    if (!_writer->write(&data, 1)) {
        throw std::runtime_error("Failed to write byte");
    }
}

void SocketOutputStream::writeSignedByte(int8_t v) { writeByte(v); }
void SocketOutputStream::writeUnsignedByte(uint8_t v) { writeByte(static_cast<int8_t>(v)); }

void SocketOutputStream::writeShort(int16_t v) {
    if (!_writer || !_writer->isConnected()) {
        throw std::runtime_error("Socket not connected");
    }
    uint8_t buf[2] = {
        static_cast<uint8_t>((v >> 8) & 0xFF),
        static_cast<uint8_t>(v & 0xFF)
    };
    if (!_writer->write(buf, 2)) {
        throw std::runtime_error("Failed to write short");
    }
}

void SocketOutputStream::writeUnsignedShort(uint16_t v) {
    writeShort(static_cast<int16_t>(v));
}

void SocketOutputStream::writeInt(int32_t v) {
    if (!_writer || !_writer->isConnected()) {
        throw std::runtime_error("Socket not connected");
    }
    uint8_t buf[4] = {
        static_cast<uint8_t>((v >> 24) & 0xFF),
        static_cast<uint8_t>((v >> 16) & 0xFF),
        static_cast<uint8_t>((v >> 8) & 0xFF),
        static_cast<uint8_t>(v & 0xFF)
    };
    if (!_writer->write(buf, 4)) {
        throw std::runtime_error("Failed to write int");
    }
}

void SocketOutputStream::writeUnsignedInt(uint32_t v) {
    writeInt(static_cast<int32_t>(v));
}

void SocketOutputStream::writeLong(int64_t v) {
    if (!_writer || !_writer->isConnected()) {
        throw std::runtime_error("Socket not connected");
    }
    uint8_t buf[8] = {
        static_cast<uint8_t>((v >> 56) & 0xFF),
        static_cast<uint8_t>((v >> 48) & 0xFF),
        static_cast<uint8_t>((v >> 40) & 0xFF),
        static_cast<uint8_t>((v >> 32) & 0xFF),
        static_cast<uint8_t>((v >> 24) & 0xFF),
        static_cast<uint8_t>((v >> 16) & 0xFF),
        static_cast<uint8_t>((v >> 8) & 0xFF),
        static_cast<uint8_t>(v & 0xFF)
    };
    if (!_writer->write(buf, 8)) {
        throw std::runtime_error("Failed to write long");
    }
}

void SocketOutputStream::writeFloat(float v) {
    uint32_t bits;
    memcpy(&bits, &v, sizeof(float));
    writeInt(static_cast<int32_t>(bits));
}

void SocketOutputStream::writeDouble(double v) {
    uint64_t bits;
    memcpy(&bits, &v, sizeof(double));
    writeLong(static_cast<int64_t>(bits));
}

void SocketOutputStream::writeBoolean(bool v) {
    writeByte(static_cast<int8_t>(v ? 1 : 0));
}

void SocketOutputStream::writeUTF(const std::string& s) {
    writeUnsignedShort(static_cast<uint16_t>(s.length()));
    if (!s.empty()) {
        writeBytes(s);
    }
}

void SocketOutputStream::writeBytes(const std::string& s) {
    if (!_writer || !_writer->isConnected()) {
        throw std::runtime_error("Socket not connected");
    }
    if (!s.empty()) {
        if (!_writer->write(reinterpret_cast<const uint8_t*>(s.c_str()), s.length())) {
            throw std::runtime_error("Failed to write bytes");
        }
    }
}

void SocketOutputStream::writeArray(const std::vector<int8_t>& arr) {
    for (auto v : arr) writeByte(v);
}

void SocketOutputStream::flush() {
    if (_writer) {
        _writer->flush();
    }
}

void SocketOutputStream::close() {
    if (_writer) {
        _writer->close();
        _writer = nullptr;
    }
}

bool SocketOutputStream::isConnected() const {
    return _writer && _writer->isConnected();
}