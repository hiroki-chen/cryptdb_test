#include <vector>
#include <string>

#include "cryptohandlers.hh"
#include "base64.hh"
#include "sm4.hh"

std::vector<unsigned char>
pad(std::vector<unsigned char> data, unsigned int unit)
{
	// pad does not work for padding unit more than 256 bytes
	assert(unit < 256);

	size_t blocks = getBlocks(unit, data.size());
	size_t multipleLen = blocks * unit;
	size_t padding;
	if (multipleLen == data.size())
	{
		padding = unit;
	}
	else
	{
		padding = multipleLen - data.size();
	}
	assert(padding > 0 && padding <= BLOCK_SIZE);
	size_t paddedLen = data.size() + padding;
	assert((paddedLen > 0) && ((paddedLen % BLOCK_SIZE) == 0));

	// cerr << "length of padding " << padding << " length of padded data " << paddedLen << "\n";

	std::vector<unsigned char> res(paddedLen, 0);
	res[paddedLen - 1] = (unsigned char)padding;
	memcpy(&res[0], &data[0], data.size());
	return res;
}

std::vector<unsigned char>
unpad(std::vector<unsigned char> data)
{
	const size_t len = data.size();
	assert((len > 0) && ((len % BLOCK_SIZE) == 0));
	const size_t pad_count = static_cast<int>(data[len - 1]);
	//cerr << "padding to remove " << (int)data[len-1] << "\n";
	const size_t actualLen = len - pad_count;
	//cerr << " len is " << len << " and data[len-1] " << (int)data[len-1] << "\n";
	// Padding will never be larger than a block.
	if (false == ((pad_count > 0) && (pad_count <= BLOCK_SIZE)))
	{
		throw CryptoException("SM4 padding is wrong size!");
	}
	// Tells us when we have a bad length.
	assert(pad_count <= len);
	std::vector<unsigned char> res(actualLen);
	memcpy(&res[0], &data[0], actualLen);
	return res;
}

std::string
encrypt_DET(const std::string &plaintext, const std::string &salt, const std::string &rawkey)
{
	const std::string ptext = plaintext + salt;

	sm4_context ctx;
	std::vector<unsigned char> ptext_buf = pad(std::vector<unsigned char>(ptext.begin(), ptext.end()), BLOCK_SIZE);
	std::vector<unsigned char> ctext_buf(ptext_buf.size());

	sm4_setkey_enc(&ctx, (unsigned char *)rawkey.c_str());
	sm4_crypt_ecb(&ctx, 1, ptext_buf.size(), &ptext_buf[0], &ctext_buf[0]);

	// return (EncodeBase64((char *)&ctext_buf[0], ctext_buf.size()));
	return base64_encode(std::string((char *)&ctext_buf[0], ctext_buf.size()));
}

std::string
decrypt_DET(const std::string &ciphertext, const std::string &rawkey, const unsigned int &salt_length)
{
	sm4_context ctx;
	const std::string ctext = base64_decode(ciphertext);

	std::vector<unsigned char> ptext_buf(ctext.size());

	sm4_setkey_dec(&ctx, (unsigned char *)rawkey.c_str());
	sm4_crypt_ecb(&ctx, 0, ctext.size(), (unsigned char *)&ctext[0], &ptext_buf[0]);
	std::vector<unsigned char> res = unpad(ptext_buf);

	return std::string((char *)&res[0], res.size() - salt_length);
}
