//
// RSACipherImpl.cpp
//
// $Id: //poco/Main/Crypto/src/RSACipherImpl.cpp#1 $
//
// Library: Crypto
// Package: CryptoCore
// Module:  RSACipherImpl
//
// Copyright (c) 2008, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Crypto/RSACipherImpl.h"
#include "Poco/Crypto/CryptoTransform.h"
#include "Poco/Exception.h"
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <string>
#include <cstring>


namespace Poco {
namespace Crypto {


static void throwError()
{
	unsigned long err;
	std::string msg;

	while ((err = ERR_get_error()))
	{
		if (!msg.empty())
			msg.append("; ");
		msg.append(ERR_error_string(err, 0));
	}

	throw Poco::IOException(msg);
}


class RSAEncryptImpl : public CryptoTransform
{
public:
	enum
	{
		OVERFLOW = 11
	};
	RSAEncryptImpl(
		const RSA* pRSA);

	~RSAEncryptImpl();

	std::size_t blockSize() const;

	std::streamsize transform(
		const unsigned char* input,
		std::streamsize		 inputLength,
		unsigned char*		 output,
		std::streamsize		 outputLength);

	std::streamsize finalize(
		unsigned char*	output,
		std::streamsize length);

private:
	std::streamsize _pos;
	const RSA*      _pRSA;
	unsigned char*  _pBuf;
};


RSAEncryptImpl::RSAEncryptImpl(
	const RSA* pRSA):
		_pos(0),
		_pRSA(pRSA),
		_pBuf(0)
{
	_pBuf = new unsigned char[blockSize()];
}


RSAEncryptImpl::~RSAEncryptImpl()
{
	delete _pBuf;
}


std::size_t RSAEncryptImpl::blockSize() const
{
	return RSA_size(_pRSA);
}


std::streamsize RSAEncryptImpl::transform(
	const unsigned char* input,
	std::streamsize		 inputLength,
	unsigned char*		 output,
	std::streamsize		 outputLength)
{

	// always fill up the buffer before writing!
	std::streamsize rsaSize = blockSize();
	poco_assert_dbg(_pos <= rsaSize);
	poco_assert (outputLength >= rsaSize);
	int rc = 0;
	while (inputLength > 0)
	{
		// check how many data bytes we are missing to get the buffer full
		poco_assert_dbg (rsaSize >= _pos);
		std::streamsize missing = rsaSize - _pos;
		if (missing == 0)
		{
			poco_assert (outputLength >= rsaSize);
			int tmp = RSA_public_encrypt(rsaSize, _pBuf, output, const_cast<RSA*>(_pRSA), RSA_NO_PADDING);
			if (tmp == -1)
				throwError();
			rc += tmp;
			output += tmp;
			outputLength -= tmp;
			_pos = 0;

		}
		else
		{
			if (missing > inputLength)
				missing = inputLength;

			std::memcpy(_pBuf+_pos, input, missing);
			input += missing;
			_pos += missing;
			inputLength -= missing;
		}
	}

	return rc;
}


std::streamsize RSAEncryptImpl::finalize(
	unsigned char*	output,
	std::streamsize length)
{
	poco_assert (length >= blockSize());
	int rc = 0;
	if (_pos > 0)
	{
		rc = RSA_public_encrypt(_pos, _pBuf, output, const_cast<RSA*>(_pRSA), RSA_PKCS1_PADDING);
		if (rc == -1)
			throwError();
	}

	return rc;
}




class RSADecryptImpl : public CryptoTransform
{
public:
	enum
	{
		OVERFLOW = 11
	};
	RSADecryptImpl(
		const RSA* pRSA);

	~RSADecryptImpl();

	std::size_t blockSize() const;

	std::streamsize transform(
		const unsigned char* input,
		std::streamsize		 inputLength,
		unsigned char*		 output,
		std::streamsize		 outputLength);

	std::streamsize finalize(
		unsigned char*	output,
		std::streamsize length);

private:
	std::streamsize _pos;
	const RSA*      _pRSA;
	unsigned char*  _pBuf;
};


RSADecryptImpl::RSADecryptImpl(
	const RSA* pRSA):
		_pos(0),
		_pRSA(pRSA),
		_pBuf(0)
{
	_pBuf = new unsigned char[blockSize()];
}


RSADecryptImpl::~RSADecryptImpl()
{
	delete _pBuf;
}


std::size_t RSADecryptImpl::blockSize() const
{
	return RSA_size(_pRSA);
}


std::streamsize RSADecryptImpl::transform(
	const unsigned char* input,
	std::streamsize		 inputLength,
	unsigned char*		 output,
	std::streamsize		 outputLength)
{

	// always fill up the buffer before decrypting!
	std::streamsize rsaSize = blockSize();
	poco_assert_dbg(_pos <= rsaSize);
	poco_assert (outputLength >= rsaSize);
	int rc = 0;
while (inputLength > 0)
	{
		// check how many data bytes we are missing to get the buffer full
		poco_assert_dbg (rsaSize >= _pos);
		std::streamsize missing = rsaSize - _pos;
		if (missing == 0)
		{
			int tmp = RSA_private_decrypt(rsaSize, _pBuf, output, const_cast<RSA*>(_pRSA), RSA_NO_PADDING);
			if (tmp == -1)
				throwError();
			rc += tmp;
			output += tmp;
			outputLength -= tmp;
			_pos = 0;

		}
		else
		{
			if (missing > inputLength)
				missing = inputLength;

			std::memcpy(_pBuf+_pos, input, missing);
			input += missing;
			_pos += missing;
			inputLength -= missing;
		}
	}

	return rc;
}


std::streamsize RSADecryptImpl::finalize(
	unsigned char*	output,
	std::streamsize length)
{
	poco_assert (length >= blockSize());
	int rc = 0;
	if (_pos > 0)
	{
		rc = RSA_private_decrypt(_pos, _pBuf, output, const_cast<RSA*>(_pRSA), RSA_PKCS1_PADDING);
		if (rc == -1)
			throwError();
	}

	return rc;
}

RSACipherImpl::RSACipherImpl(const RSAKey& key):
	_key(key)
{
}


RSACipherImpl::~RSACipherImpl()
{
}


CryptoTransform* RSACipherImpl::createEncryptor()
{
	return new RSAEncryptImpl(_key.impl()->getRSA());
}


CryptoTransform* RSACipherImpl::createDecryptor()
{
	return new RSADecryptImpl(_key.impl()->getRSA());
}


} } // namespace Poco::Crypto
