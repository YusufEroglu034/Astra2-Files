

#pragma once

#include "iodevice_provider.h"
#include "File.h"
#include "DataBuffer.hpp"

#include <Windows.h>

class IODeviceProvider_File : public IODeviceProvider
{
public:
	IODeviceProvider_File();

	IODeviceProvider_File(
		const std::string &filename,
		File::OpenMode mode,
		uint32_t access,
		uint32_t share,
		uint32_t flags);

	~IODeviceProvider_File();

	int32_t get_size() const override;
	int32_t get_position() const override;

	bool open(
		const std::string &filename,
		File::OpenMode mode,
		uint32_t access,
		uint32_t share,
		uint32_t flags);

	void close();

	int32_t read(void *buffer, uint32_t size, bool read_all);
	int32_t write(const void *buffer, int32_t size, bool write_all);
	int32_t send(const void *data, int32_t len, bool send_all) override;
	int32_t receive(void *data, int32_t len, bool receive_all) override;
	int32_t peek(void *data, int32_t len) override;

	bool seek(int32_t position, IODevice::SeekMode mode) override;

	IODeviceProvider *duplicate() override;

private:
	int32_t lowlevel_read(void *buffer, int32_t size, bool read_all);

	std::string filename;
	File::OpenMode open_mode;
	uint32_t access;
	uint32_t share;
	uint32_t flags;

#ifdef WIN32
	HANDLE handle;
#else
	int32_t handle;
#endif
	DataBuffer peeked_data;
};

