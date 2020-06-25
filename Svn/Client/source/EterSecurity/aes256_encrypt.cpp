

#include "stdafx.h"
#include "aes256_encrypt.h"
#include "DataBuffer.hpp"
#include "Exception.h"

#ifndef WIN32
#include <cstring>
#endif

#include <exception>
#include <algorithm>
#include <vector>

AES256_Encrypt_Impl::AES256_Encrypt_Impl() : initialisation_vector_set(false), cipher_key_set(false), padding_enabled(true), padding_pkcs7(true), padding_num_additional_padded_blocks(0)
{
	reset();
}

DataBuffer AES256_Encrypt_Impl::get_data() const
{
	return databuffer;
}

void AES256_Encrypt_Impl::reset()
{
	calculated = false;
	memset(chunk, 0, sizeof(chunk));
	chunk_filled = 0;
	databuffer.set_size(0);
}

void AES256_Encrypt_Impl::set_iv(const uint8_t iv[16])
{
	initialisation_vector_1 = get_word(iv);
	initialisation_vector_2 = get_word(iv + 4);
	initialisation_vector_3 = get_word(iv + 8);
	initialisation_vector_4 = get_word(iv + 12);

	initialisation_vector_set = true;
}

void AES256_Encrypt_Impl::set_padding(bool value, bool use_pkcs7, uint32_t num_additional_padded_blocks)
{
	padding_enabled = value;
	padding_pkcs7 = use_pkcs7;
	padding_num_additional_padded_blocks = num_additional_padded_blocks;
	if (padding_num_additional_padded_blocks > 15)
		throw Exception("Number of additional blocks in padding must be 0 to 15");
}

void AES256_Encrypt_Impl::set_key(const uint8_t key[32])
{
	cipher_key_set = true;
	extract_encrypt_key256(key, key_expanded);
}

void AES256_Encrypt_Impl::add(const void *_data, int32_t size)
{
	if (calculated)
		reset();

	if (!initialisation_vector_set)
		throw Exception("AES-256 initialisation vector has not been set");

	if (!cipher_key_set)
		throw Exception("AES-256 cipher key has not been set");

	const uint8_t *data = (const uint8_t *)_data;
	int32_t pos = 0;
	while (pos < size)
	{
		int32_t data_left = size - pos;
		int32_t buffer_space = aes256_block_size_bytes - chunk_filled;
		int32_t data_used = min(buffer_space, data_left);
		memcpy(chunk + chunk_filled, data + pos, data_used);
		chunk_filled += data_used;
		pos += data_used;
		if (chunk_filled == aes256_block_size_bytes)
		{
			process_chunk();
			chunk_filled = 0;
		}
	}
}

void AES256_Encrypt_Impl::calculate()
{
	if (calculated)
		reset();

	if (padding_enabled)
	{
		if (padding_pkcs7)
		{
			uint8_t pad_size = aes256_block_size_bytes - chunk_filled;
			memset(chunk + chunk_filled, pad_size, pad_size);
			process_chunk();
			chunk_filled = 0;
		}
		else
		{
			std::vector<uint8_t> buffer;
			uint32_t pad_size = aes256_block_size_bytes - chunk_filled;
			pad_size += aes128_block_size_bytes * padding_num_additional_padded_blocks;
			buffer.resize(pad_size, pad_size - 1);
			add(&buffer[0], pad_size);
		}
	}
	else
	{
		if (chunk_filled > 0)
			throw Exception("You must provide data with a block size of 16 when padding is disabled");
	}

	calculated = true;
	initialisation_vector_set = false;
	cipher_key_set = false;
	memset(key_expanded, 0, sizeof(key_expanded));
}

void AES256_Encrypt_Impl::process_chunk()
{
	const uint32_t *key_expanded_ptr = key_expanded;

	

	uint32_t s0 = initialisation_vector_1 ^ get_word(chunk) ^ key_expanded_ptr[0];
	uint32_t s1 = initialisation_vector_2 ^ get_word(chunk + 4) ^ key_expanded_ptr[1];
	uint32_t s2 = initialisation_vector_3 ^ get_word(chunk + 8) ^ key_expanded_ptr[2];
	uint32_t s3 = initialisation_vector_4 ^ get_word(chunk + 12) ^ key_expanded_ptr[3];

	uint32_t t0;
	uint32_t t1;
	uint32_t t2;
	uint32_t t3;

	t0 = table_e0[s0 >> 24] ^ table_e1[(s1 >> 16) & 0xff] ^ table_e2[(s2 >> 8) & 0xff] ^ table_e3[s3 & 0xff] ^ key_expanded_ptr[4];
	t1 = table_e0[s1 >> 24] ^ table_e1[(s2 >> 16) & 0xff] ^ table_e2[(s3 >> 8) & 0xff] ^ table_e3[s0 & 0xff] ^ key_expanded_ptr[5];
	t2 = table_e0[s2 >> 24] ^ table_e1[(s3 >> 16) & 0xff] ^ table_e2[(s0 >> 8) & 0xff] ^ table_e3[s1 & 0xff] ^ key_expanded_ptr[6];
	t3 = table_e0[s3 >> 24] ^ table_e1[(s0 >> 16) & 0xff] ^ table_e2[(s1 >> 8) & 0xff] ^ table_e3[s2 & 0xff] ^ key_expanded_ptr[7];
	s0 = table_e0[t0 >> 24] ^ table_e1[(t1 >> 16) & 0xff] ^ table_e2[(t2 >> 8) & 0xff] ^ table_e3[t3 & 0xff] ^ key_expanded_ptr[8];
	s1 = table_e0[t1 >> 24] ^ table_e1[(t2 >> 16) & 0xff] ^ table_e2[(t3 >> 8) & 0xff] ^ table_e3[t0 & 0xff] ^ key_expanded_ptr[9];
	s2 = table_e0[t2 >> 24] ^ table_e1[(t3 >> 16) & 0xff] ^ table_e2[(t0 >> 8) & 0xff] ^ table_e3[t1 & 0xff] ^ key_expanded_ptr[10];
	s3 = table_e0[t3 >> 24] ^ table_e1[(t0 >> 16) & 0xff] ^ table_e2[(t1 >> 8) & 0xff] ^ table_e3[t2 & 0xff] ^ key_expanded_ptr[11];
	t0 = table_e0[s0 >> 24] ^ table_e1[(s1 >> 16) & 0xff] ^ table_e2[(s2 >> 8) & 0xff] ^ table_e3[s3 & 0xff] ^ key_expanded_ptr[12];
	t1 = table_e0[s1 >> 24] ^ table_e1[(s2 >> 16) & 0xff] ^ table_e2[(s3 >> 8) & 0xff] ^ table_e3[s0 & 0xff] ^ key_expanded_ptr[13];
	t2 = table_e0[s2 >> 24] ^ table_e1[(s3 >> 16) & 0xff] ^ table_e2[(s0 >> 8) & 0xff] ^ table_e3[s1 & 0xff] ^ key_expanded_ptr[14];
	t3 = table_e0[s3 >> 24] ^ table_e1[(s0 >> 16) & 0xff] ^ table_e2[(s1 >> 8) & 0xff] ^ table_e3[s2 & 0xff] ^ key_expanded_ptr[15];
	s0 = table_e0[t0 >> 24] ^ table_e1[(t1 >> 16) & 0xff] ^ table_e2[(t2 >> 8) & 0xff] ^ table_e3[t3 & 0xff] ^ key_expanded_ptr[16];
	s1 = table_e0[t1 >> 24] ^ table_e1[(t2 >> 16) & 0xff] ^ table_e2[(t3 >> 8) & 0xff] ^ table_e3[t0 & 0xff] ^ key_expanded_ptr[17];
	s2 = table_e0[t2 >> 24] ^ table_e1[(t3 >> 16) & 0xff] ^ table_e2[(t0 >> 8) & 0xff] ^ table_e3[t1 & 0xff] ^ key_expanded_ptr[18];
	s3 = table_e0[t3 >> 24] ^ table_e1[(t0 >> 16) & 0xff] ^ table_e2[(t1 >> 8) & 0xff] ^ table_e3[t2 & 0xff] ^ key_expanded_ptr[19];
	t0 = table_e0[s0 >> 24] ^ table_e1[(s1 >> 16) & 0xff] ^ table_e2[(s2 >> 8) & 0xff] ^ table_e3[s3 & 0xff] ^ key_expanded_ptr[20];
	t1 = table_e0[s1 >> 24] ^ table_e1[(s2 >> 16) & 0xff] ^ table_e2[(s3 >> 8) & 0xff] ^ table_e3[s0 & 0xff] ^ key_expanded_ptr[21];
	t2 = table_e0[s2 >> 24] ^ table_e1[(s3 >> 16) & 0xff] ^ table_e2[(s0 >> 8) & 0xff] ^ table_e3[s1 & 0xff] ^ key_expanded_ptr[22];
	t3 = table_e0[s3 >> 24] ^ table_e1[(s0 >> 16) & 0xff] ^ table_e2[(s1 >> 8) & 0xff] ^ table_e3[s2 & 0xff] ^ key_expanded_ptr[23];
	s0 = table_e0[t0 >> 24] ^ table_e1[(t1 >> 16) & 0xff] ^ table_e2[(t2 >> 8) & 0xff] ^ table_e3[t3 & 0xff] ^ key_expanded_ptr[24];
	s1 = table_e0[t1 >> 24] ^ table_e1[(t2 >> 16) & 0xff] ^ table_e2[(t3 >> 8) & 0xff] ^ table_e3[t0 & 0xff] ^ key_expanded_ptr[25];
	s2 = table_e0[t2 >> 24] ^ table_e1[(t3 >> 16) & 0xff] ^ table_e2[(t0 >> 8) & 0xff] ^ table_e3[t1 & 0xff] ^ key_expanded_ptr[26];
	s3 = table_e0[t3 >> 24] ^ table_e1[(t0 >> 16) & 0xff] ^ table_e2[(t1 >> 8) & 0xff] ^ table_e3[t2 & 0xff] ^ key_expanded_ptr[27];
	t0 = table_e0[s0 >> 24] ^ table_e1[(s1 >> 16) & 0xff] ^ table_e2[(s2 >> 8) & 0xff] ^ table_e3[s3 & 0xff] ^ key_expanded_ptr[28];
	t1 = table_e0[s1 >> 24] ^ table_e1[(s2 >> 16) & 0xff] ^ table_e2[(s3 >> 8) & 0xff] ^ table_e3[s0 & 0xff] ^ key_expanded_ptr[29];
	t2 = table_e0[s2 >> 24] ^ table_e1[(s3 >> 16) & 0xff] ^ table_e2[(s0 >> 8) & 0xff] ^ table_e3[s1 & 0xff] ^ key_expanded_ptr[30];
	t3 = table_e0[s3 >> 24] ^ table_e1[(s0 >> 16) & 0xff] ^ table_e2[(s1 >> 8) & 0xff] ^ table_e3[s2 & 0xff] ^ key_expanded_ptr[31];
	s0 = table_e0[t0 >> 24] ^ table_e1[(t1 >> 16) & 0xff] ^ table_e2[(t2 >> 8) & 0xff] ^ table_e3[t3 & 0xff] ^ key_expanded_ptr[32];
	s1 = table_e0[t1 >> 24] ^ table_e1[(t2 >> 16) & 0xff] ^ table_e2[(t3 >> 8) & 0xff] ^ table_e3[t0 & 0xff] ^ key_expanded_ptr[33];
	s2 = table_e0[t2 >> 24] ^ table_e1[(t3 >> 16) & 0xff] ^ table_e2[(t0 >> 8) & 0xff] ^ table_e3[t1 & 0xff] ^ key_expanded_ptr[34];
	s3 = table_e0[t3 >> 24] ^ table_e1[(t0 >> 16) & 0xff] ^ table_e2[(t1 >> 8) & 0xff] ^ table_e3[t2 & 0xff] ^ key_expanded_ptr[35];
	t0 = table_e0[s0 >> 24] ^ table_e1[(s1 >> 16) & 0xff] ^ table_e2[(s2 >> 8) & 0xff] ^ table_e3[s3 & 0xff] ^ key_expanded_ptr[36];
	t1 = table_e0[s1 >> 24] ^ table_e1[(s2 >> 16) & 0xff] ^ table_e2[(s3 >> 8) & 0xff] ^ table_e3[s0 & 0xff] ^ key_expanded_ptr[37];
	t2 = table_e0[s2 >> 24] ^ table_e1[(s3 >> 16) & 0xff] ^ table_e2[(s0 >> 8) & 0xff] ^ table_e3[s1 & 0xff] ^ key_expanded_ptr[38];
	t3 = table_e0[s3 >> 24] ^ table_e1[(s0 >> 16) & 0xff] ^ table_e2[(s1 >> 8) & 0xff] ^ table_e3[s2 & 0xff] ^ key_expanded_ptr[39];
	s0 = table_e0[t0 >> 24] ^ table_e1[(t1 >> 16) & 0xff] ^ table_e2[(t2 >> 8) & 0xff] ^ table_e3[t3 & 0xff] ^ key_expanded_ptr[40];
	s1 = table_e0[t1 >> 24] ^ table_e1[(t2 >> 16) & 0xff] ^ table_e2[(t3 >> 8) & 0xff] ^ table_e3[t0 & 0xff] ^ key_expanded_ptr[41];
	s2 = table_e0[t2 >> 24] ^ table_e1[(t3 >> 16) & 0xff] ^ table_e2[(t0 >> 8) & 0xff] ^ table_e3[t1 & 0xff] ^ key_expanded_ptr[42];
	s3 = table_e0[t3 >> 24] ^ table_e1[(t0 >> 16) & 0xff] ^ table_e2[(t1 >> 8) & 0xff] ^ table_e3[t2 & 0xff] ^ key_expanded_ptr[43];
	t0 = table_e0[s0 >> 24] ^ table_e1[(s1 >> 16) & 0xff] ^ table_e2[(s2 >> 8) & 0xff] ^ table_e3[s3 & 0xff] ^ key_expanded_ptr[44];
	t1 = table_e0[s1 >> 24] ^ table_e1[(s2 >> 16) & 0xff] ^ table_e2[(s3 >> 8) & 0xff] ^ table_e3[s0 & 0xff] ^ key_expanded_ptr[45];
	t2 = table_e0[s2 >> 24] ^ table_e1[(s3 >> 16) & 0xff] ^ table_e2[(s0 >> 8) & 0xff] ^ table_e3[s1 & 0xff] ^ key_expanded_ptr[46];
	t3 = table_e0[s3 >> 24] ^ table_e1[(s0 >> 16) & 0xff] ^ table_e2[(s1 >> 8) & 0xff] ^ table_e3[s2 & 0xff] ^ key_expanded_ptr[47];
	s0 = table_e0[t0 >> 24] ^ table_e1[(t1 >> 16) & 0xff] ^ table_e2[(t2 >> 8) & 0xff] ^ table_e3[t3 & 0xff] ^ key_expanded_ptr[48];
	s1 = table_e0[t1 >> 24] ^ table_e1[(t2 >> 16) & 0xff] ^ table_e2[(t3 >> 8) & 0xff] ^ table_e3[t0 & 0xff] ^ key_expanded_ptr[49];
	s2 = table_e0[t2 >> 24] ^ table_e1[(t3 >> 16) & 0xff] ^ table_e2[(t0 >> 8) & 0xff] ^ table_e3[t1 & 0xff] ^ key_expanded_ptr[50];
	s3 = table_e0[t3 >> 24] ^ table_e1[(t0 >> 16) & 0xff] ^ table_e2[(t1 >> 8) & 0xff] ^ table_e3[t2 & 0xff] ^ key_expanded_ptr[51];
	t0 = table_e0[s0 >> 24] ^ table_e1[(s1 >> 16) & 0xff] ^ table_e2[(s2 >> 8) & 0xff] ^ table_e3[s3 & 0xff] ^ key_expanded_ptr[52];
	t1 = table_e0[s1 >> 24] ^ table_e1[(s2 >> 16) & 0xff] ^ table_e2[(s3 >> 8) & 0xff] ^ table_e3[s0 & 0xff] ^ key_expanded_ptr[53];
	t2 = table_e0[s2 >> 24] ^ table_e1[(s3 >> 16) & 0xff] ^ table_e2[(s0 >> 8) & 0xff] ^ table_e3[s1 & 0xff] ^ key_expanded_ptr[54];
	t3 = table_e0[s3 >> 24] ^ table_e1[(s0 >> 16) & 0xff] ^ table_e2[(s1 >> 8) & 0xff] ^ table_e3[s2 & 0xff] ^ key_expanded_ptr[55];

	key_expanded_ptr += aes256_num_rounds_nr * 4;

	s0 = (sbox_substitution_values[(t0 >> 24)] & 0xff000000) ^ (sbox_substitution_values[(t1 >> 16) & 0xff] & 0x00ff0000) ^ (sbox_substitution_values[(t2 >> 8) & 0xff] & 0x0000ff00) ^ (sbox_substitution_values[(t3) & 0xff] & 0x000000ff) ^ key_expanded_ptr[0];
	s1 = (sbox_substitution_values[(t1 >> 24)] & 0xff000000) ^ (sbox_substitution_values[(t2 >> 16) & 0xff] & 0x00ff0000) ^ (sbox_substitution_values[(t3 >> 8) & 0xff] & 0x0000ff00) ^ (sbox_substitution_values[(t0) & 0xff] & 0x000000ff) ^ key_expanded_ptr[1];
	s2 = (sbox_substitution_values[(t2 >> 24)] & 0xff000000) ^ (sbox_substitution_values[(t3 >> 16) & 0xff] & 0x00ff0000) ^ (sbox_substitution_values[(t0 >> 8) & 0xff] & 0x0000ff00) ^ (sbox_substitution_values[(t1) & 0xff] & 0x000000ff) ^ key_expanded_ptr[2];
	s3 = (sbox_substitution_values[(t3 >> 24)] & 0xff000000) ^ (sbox_substitution_values[(t0 >> 16) & 0xff] & 0x00ff0000) ^ (sbox_substitution_values[(t1 >> 8) & 0xff] & 0x0000ff00) ^ (sbox_substitution_values[(t2) & 0xff] & 0x000000ff) ^ key_expanded_ptr[3];

	store_block(s0, s1, s2, s3, databuffer);

	initialisation_vector_1 = s0;
	initialisation_vector_2 = s1;
	initialisation_vector_3 = s2;
	initialisation_vector_4 = s3;
}

AES256_Encrypt::AES256_Encrypt()
	: impl(std::make_shared<AES256_Encrypt_Impl>())
{
}

DataBuffer AES256_Encrypt::get_data() const
{
	return impl->get_data();
}

void AES256_Encrypt::reset()
{
	impl->reset();
}

void AES256_Encrypt::set_iv(const uint8_t iv[16])
{
	impl->set_iv(iv);
}

void AES256_Encrypt::set_key(const uint8_t key[32])
{
	impl->set_key(key);
}

void AES256_Encrypt::set_padding(bool value, bool use_pkcs7, uint32_t num_additional_padded_blocks)
{
	impl->set_padding(value, use_pkcs7, num_additional_padded_blocks);
}

void AES256_Encrypt::add(const void *data, int32_t size)
{
	impl->add(data, size);
}

void AES256_Encrypt::add(const DataBuffer &data)
{
	add(data.get_data(), data.get_size());
}

void AES256_Encrypt::calculate()
{
	impl->calculate();
}