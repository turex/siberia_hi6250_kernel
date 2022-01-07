obj-$(CONFIG_FS_ENCRYPTION)	+= fscrypto.o

fscrypto-y := crypto.o \
	      fname.o \
	      hooks.o \
	      keyring.o \
	      keysetup.o \
	      policy.o

fscrypto-$(CONFIG_BLOCK) += bio.o
