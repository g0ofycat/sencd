#include "crypto.h"

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
				public_key) != 0)
		return 1;

	return 0;
}

int crypto_identity_load_or_create(CRYPTO_CONTEXT_T *ctx, const char *path) {
	FILE *f = fopen(path, "rb");
	if (f) {
		size_t pub_read  = fread(ctx->public_key, 1, CRYPTO_PUBLIC_KEY_SIZE, f);
		size_t priv_read = fread(ctx->private_key, 1, CRYPTO_PRIVATE_KEY_SIZE, f);
		fclose(f);

		if (pub_read == CRYPTO_PUBLIC_KEY_SIZE && priv_read == CRYPTO_PRIVATE_KEY_SIZE) {
			unsigned char sig[CRYPTO_SIGNATURE_SIZE];
			const unsigned char probe[] = "sencd-identity-check";
			if (crypto_sign_message(ctx, probe, sizeof(probe), sig) == 0 && crypto_verify_message(ctx->public_key, probe, sizeof(probe), sig) == 0)
				return 0;
			log_msg(WARN_MSG, OTHER_RT, "Identity file failed integrity check, regenerating");
		}

		log_msg(WARN_MSG, OTHER_RT, "Identity file is corrupt, regenerating");
	}

	if (crypto_generate_identity(ctx) != 0)
		return 1;

	FILE *out = fopen(path, "wb");
	if (out == NULL) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to persist identity key");
		return 1;
	}

	fwrite(ctx->public_key, 1, CRYPTO_PUBLIC_KEY_SIZE, out);
	fwrite(ctx->private_key, 1, CRYPTO_PRIVATE_KEY_SIZE, out);
	fclose(out);
	chmod(path, S_IRUSR | S_IWUSR);

	return 0;
}

int crypto_trust_key_load(const char *path, unsigned char out_key[CRYPTO_PUBLIC_KEY_SIZE]) {
	FILE *f = fopen(path, "rb");
	if (f == NULL)
		return 1;

	size_t read_bytes = fread(out_key, 1, CRYPTO_PUBLIC_KEY_SIZE, f);
	fclose(f);
	return (read_bytes == CRYPTO_PUBLIC_KEY_SIZE) ? 0 : 1;
}

int crypto_trust_key_save(const char *path, const unsigned char key[CRYPTO_PUBLIC_KEY_SIZE]) {
	FILE *f = fopen(path, "wb");
	if (f == NULL) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to persist trusted server key");
		return 1;
	}

	fwrite(key, 1, CRYPTO_PUBLIC_KEY_SIZE, f);
	fclose(f);
	chmod(path, S_IRUSR | S_IWUSR);
	return 0;
}

int crypto_config_path(const char *filename, char *out_path, size_t out_size) {
	const char *home = getenv("HOME");
	if (home == NULL || home[0] == '\0') {
		struct passwd *pw = getpwuid(getuid());
		if (pw == NULL) {
			log_msg(ERROR_MSG, OTHER_RT, "Could not determine home directory");
			return 1;
		}
		home = pw->pw_dir;
	}

	char dir[PATH_MAX];
	if (snprintf(dir, sizeof(dir), "%s/%s", home, SENCD_CONFIG_DIRNAME) >= (int)sizeof(dir))
		return 1;

	if (mkdir(dir, S_IRWXU) != 0 && errno != EEXIST) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to create config directory");
		return 1;
	}

	if (snprintf(out_path, out_size, "%s/%s", dir, filename) >= (int)out_size)
		return 1;

	return 0;
}

int crypto_generate_ephemeral(CRYPTO_CONTEXT_T *ctx) {
	if (crypto_kx_keypair(ctx->eph_public_key, ctx->eph_private_key) != 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to generate ephemeral key pair");
		return 1;
	}
	return 0;
}

int crypto_derive_server_keys(CRYPTO_CONTEXT_T *ctx,
		const unsigned char client_eph_pub[crypto_kx_PUBLICKEYBYTES]) {
	if (crypto_kx_server_session_keys(
				ctx->rx_key, ctx->tx_key,
				ctx->eph_public_key, ctx->eph_private_key,
				client_eph_pub) != 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to derive server session keys");
		return 1;
	}
	return 0;
}

int crypto_derive_client_keys(CRYPTO_CONTEXT_T *ctx,
		const unsigned char server_eph_pub[crypto_kx_PUBLICKEYBYTES]) {
	if (crypto_kx_client_session_keys(
				ctx->rx_key, ctx->tx_key,
				ctx->eph_public_key, ctx->eph_private_key,
				server_eph_pub) != 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to derive client session keys");
		return 1;
	}
	return 0;
}

//--============
// -- PACKETS
//--============

/// @brief build a 12-byte AEAD nonce from an 8-byte counter (4 zero bytes prefix)
/// @param counter
/// @param nonce
static void crypto_build_nonce(uint64_t counter,
		unsigned char nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES]) {
	memset(nonce, 0, crypto_aead_chacha20poly1305_ietf_NPUBBYTES);
	uint64_t be_counter = htobe64(counter);
	memcpy(nonce + (crypto_aead_chacha20poly1305_ietf_NPUBBYTES - sizeof(be_counter)),
			&be_counter, sizeof(be_counter));
}

int crypto_encrypt_packet(CRYPTO_CONTEXT_T *ctx, uint64_t counter,
		const unsigned char *plaintext, size_t plaintext_len,
		unsigned char *ciphertext_out, unsigned long long *ciphertext_len_out) {
	unsigned char nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES];
	crypto_build_nonce(counter, nonce);

	if (crypto_aead_chacha20poly1305_ietf_encrypt(
				ciphertext_out, ciphertext_len_out,
				plaintext, plaintext_len,
				NULL, 0,
				NULL,
				nonce, ctx->tx_key) != 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to encrypt tunnel packet");
		return 1;
	}

	return 0;
}

int crypto_decrypt_packet(CRYPTO_CONTEXT_T *ctx, uint64_t counter,
		const unsigned char *ciphertext, size_t ciphertext_len,
		unsigned char *plaintext_out, unsigned long long *plaintext_len_out) {
	unsigned char nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES];
	crypto_build_nonce(counter, nonce);

	if (crypto_aead_chacha20poly1305_ietf_decrypt(
				plaintext_out, plaintext_len_out,
				NULL,
				ciphertext, ciphertext_len,
				NULL, 0,
				nonce, ctx->rx_key) != 0) {
		return 1;
	}

	return 0;
}
