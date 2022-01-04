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

#include <linux/fscrypt_supp.h>

#define FS_FNAME_CRYPTO_DIGEST_SIZE	32

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

#endif /* _FSCRYPT_PRIVATE_H */
