/*
 * fscrypt_private.h
 *
 * Copyright (C) 2015, Google, Inc.
 *
 * This contains encryption key functions.
 *
 * Written by Michael Halcrow, Ildar Muslukhov, and Uday Savagaonkar, 2015.
 */

#ifndef _FSCRYPT_PRIVATE_H
#define _FSCRYPT_PRIVATE_H

#define __FS_HAS_ENCRYPTION 1
#include <linux/fscrypt.h>
#include <crypto/hash.h>

#define CONST_STRLEN(str)	(sizeof(str) - 1)

#define FS_KEY_DERIVATION_NONCE_SIZE	16

#define FSCRYPT_MIN_KEY_SIZE		16

/**
 * Encryption context for inode
 *
 * Protector format:
 *  1 byte: Protector format (1 = this version)
 *  1 byte: File contents encryption mode
 *  1 byte: File names encryption mode
 *  1 byte: Flags
 *  8 bytes: Master Key descriptor
 *  16 bytes: Encryption Key derivation nonce
 */
struct fscrypt_context {
	u8 format;
	u8 contents_encryption_mode;
	u8 filenames_encryption_mode;
	u8 flags;
	u8 master_key_descriptor[FSCRYPT_KEY_DESCRIPTOR_SIZE];
	u8 nonce[FS_KEY_DERIVATION_NONCE_SIZE];
} __packed;

#define FS_ENCRYPTION_CONTEXT_FORMAT_V1		1


typedef enum {
	FS_DECRYPT = 0,
	FS_ENCRYPT,
} fscrypt_direction_t;

#define FS_CTX_REQUIRES_FREE_ENCRYPT_FL		0x00000001
#define FS_CTX_HAS_BOUNCE_BUFFER_FL		0x00000002

/* bio stuffs */
#define REQ_OP_READ	READ
#define REQ_OP_WRITE	WRITE
#define bio_op(bio)	((bio)->bi_rw & 1)

static inline void bio_set_op_attrs(struct bio *bio, unsigned op,
		unsigned op_flags)
{
	bio->bi_rw = op | op_flags;
}

/* crypto.c */
extern struct workqueue_struct *fscrypt_read_workqueue;
extern int fscrypt_do_page_crypto(const struct inode *inode,
				  fscrypt_direction_t rw, u64 lblk_num,
				  struct page *src_page,
				  struct page *dest_page,
				  unsigned int len, unsigned int offs,
				  gfp_t gfp_flags);
extern struct page *fscrypt_alloc_bounce_page(struct fscrypt_ctx *ctx,
					      gfp_t gfp_flags);

/* keyinfo.c */
extern void __exit fscrypt_essiv_cleanup(void);
extern void __printf(3, 4) __cold
fscrypt_msg(const struct inode *inode, const char *level, const char *fmt, ...);

#define fscrypt_warn(inode, fmt, ...)		\
	fscrypt_msg((inode), KERN_WARNING, fmt, ##__VA_ARGS__)
#define fscrypt_err(inode, fmt, ...)		\
	fscrypt_msg((inode), KERN_ERR, fmt, ##__VA_ARGS__)

#define FSCRYPT_MAX_IV_SIZE	32

union fscrypt_iv {
	struct {
		/* logical block number within the file */
		__le64 lblk_num;

		/* per-file nonce; only set in DIRECT_KEY mode */
		u8 nonce[FS_KEY_DERIVATION_NONCE_SIZE];
	};
	u8 raw[FSCRYPT_MAX_IV_SIZE];
};

void fscrypt_generate_iv(union fscrypt_iv *iv, u64 lblk_num,
			 const struct fscrypt_info *ci);

/* fname.c */
extern int fname_encrypt(struct inode *inode, const struct qstr *iname,
			 u8 *out, unsigned int olen);
extern bool fscrypt_fname_encrypted_size(const struct inode *inode,
					 u32 orig_len, u32 max_len,
					 u32 *encrypted_len_ret);

/* keyring.c */

/*
 * fscrypt_master_key_secret - secret key material of an in-use master key
 */
struct fscrypt_master_key_secret {

	/* Size of the raw key in bytes */
	u32			size;

	/* The raw key */
	u8			raw[FSCRYPT_MAX_KEY_SIZE];

} __randomize_layout;

/*
 * fscrypt_master_key - an in-use master key
 *
 * This represents a master encryption key which has been added to the
 * filesystem and can be used to "unlock" the encrypted files which were
 * encrypted with it.
 */
struct fscrypt_master_key {

	/* The secret key material */
	struct fscrypt_master_key_secret	mk_secret;

	/* Arbitrary key descriptor which was assigned by userspace */
	struct fscrypt_key_specifier		mk_spec;

} __randomize_layout;

static inline const char *master_key_spec_type(
				const struct fscrypt_key_specifier *spec)
{
	switch (spec->type) {
	case FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR:
		return "descriptor";
	}
	return "[unknown]";
}

static inline int master_key_spec_len(const struct fscrypt_key_specifier *spec)
{
	switch (spec->type) {
	case FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR:
		return FSCRYPT_KEY_DESCRIPTOR_SIZE;
	}
	return 0;
}

extern struct key *
fscrypt_find_master_key(struct super_block *sb,
			const struct fscrypt_key_specifier *mk_spec);

extern int __init fscrypt_init_keyring(void);

/* keysetup.c */

struct fscrypt_mode {
	const char *friendly_name;
	const char *cipher_str;
	int keysize;
	int ivsize;
	bool logged_impl_name;
	bool needs_essiv;
};

static inline bool
fscrypt_mode_supports_direct_key(const struct fscrypt_mode *mode)
{
	return mode->ivsize >= offsetofend(union fscrypt_iv, nonce);
}

extern struct crypto_skcipher *
fscrypt_allocate_skcipher(struct fscrypt_mode *mode, const u8 *raw_key,
			  const struct inode *inode);

extern int fscrypt_set_derived_key(struct fscrypt_info *ci,
				   const u8 *derived_key);

/* keysetup_v1.c */

extern void fscrypt_put_direct_key(struct fscrypt_direct_key *dk);

extern int fscrypt_setup_v1_file_key(struct fscrypt_info *ci,
				     const u8 *raw_master_key);

extern int fscrypt_setup_v1_file_key_via_subscribed_keyrings(
					struct fscrypt_info *ci);

#endif /* _FSCRYPT_PRIVATE_H */
