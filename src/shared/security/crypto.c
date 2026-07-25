#include "crypto.h"

#include "../debug/logs.h"

//--============
// -- LOGIC
//--============

void crypto_init(void) {
	if (sodium_init() < 0) {
		log_msg(ERROR_MSG, OTHER_RT,
				"Failed to initialize libsodium");
		exit(EXIT_FAILURE);
	}
}

void crypto_context_init(CRYPTO_CONTEXT_T *ctx) {
	memset(ctx, 0, sizeof(*ctx));
	ctx->verified = 0;
	ctx->tx_nonce = 0;
	ctx->rx_nonce = 0;
}

void crypto_generate_nonce(uint8_t *nonce, size_t size) {
	randombytes_buf(nonce, size);
}

int crypto_generate_identity(CRYPTO_CONTEXT_T *ctx) {
	if (crypto_sign_keypair(ctx->public_key, ctx->private_key) != 0) {
		log_msg(ERROR_MSG, OTHER_RT,
				"Failed to generate identity key pair");
		return 1;
	}

	return 0;
}

int crypto_sign_message(
		CRYPTO_CONTEXT_T *ctx,
		const unsigned char *message,
		size_t message_length,
		unsigned char signature[CRYPTO_SIGNATURE_SIZE]) {

	if (crypto_sign_detached(
				signature,
				NULL,
				message,
				(unsigned long long)message_length,
				ctx->private_key) != 0) {

		log_msg(ERROR_MSG, OTHER_RT,
				"Failed to sign message");
		return 1;
	}

	return 0;
}

int crypto_verify_message(
		const unsigned char public_key[CRYPTO_PUBLIC_KEY_SIZE],
		const unsigned char *message,
		size_t message_length,
		const unsigned char signature[CRYPTO_SIGNATURE_SIZE]) {

	if (crypto_sign_verify_detached(
				signature,
				message,
				(unsigned long long)message_length,
				public_key) != 0) {

		return 1;
	}

	return 0;
}
