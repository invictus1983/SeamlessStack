TOPDIR=..
CFLAGS += -Wall
CFLAGS += -g
#CFLAGS += -O3
CFLAGS += -I $(TOPDIR)/common/include -I include -I $(TOPDIR)/db/mongo/include
CFLAGS += -I $(TOPDIR)/common/include/bds_slab
CFLAGS += -I $(TOPDIR)/policy/include
CFLAGS += -I $(TOPDIR)/lib/memcache/include -I $(TOPDIR)/oss_install/include
CFLAGS += -I $(TOPDIR)/lib/serdes/include -I $(TOPDIR)/cli/include/
CFLAGS += -I$(TOPDIR)/oss
CFLAGS += -I $(TOPDIR)/oss/mongo-c-driver/src/mongoc
CFLAGS += -I $(TOPDIR)/oss/mongo-c-driver/src/libbson/src/bson
CFLAGS += -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DFUSE_USE_VERSION=26
CFLAGS += -DMONGO_HAVE_STDINT
#CFLAGS += -fsanitize=address -fno-omit-frame-pointer 
OSS_LIBSDIR = $(TOPDIR)/oss_install/lib
LIB = $(shell pkg-config --libs fuse) -lpthread
LIB += -L$(TOPDIR)/db/mongo -lmongo_db
LIB += -L$(OSS_LIBSDIR) -lmongoc -lbson
LIB += -L$(TOPDIR)/lib/memcache -lmcache 
LIB += -L$(TOPDIR)/policy -lpolicy
LIB += -L$(TOPDIR)/common -lsstack_common
LIB += -L$(TOPDIR)/oss_install/lib -lmemcached
LIB += -L$(TOPDIR)/lib/serdes -lserdes
LIB += -L$(TOPDIR)/lib/validate -lvalidate
LIB += -L$(TOPDIR)/cli -lsfscli
#LIB += -lasan
C99 = c99

SFS_OBJS = src/sfs.o src/sfs_entry.o src/sfs_job.o src/sfs_idp.o
SFS_OBJS += src/sfs_traceroute.o src/sfs_sfscli.o src/sfs_helper.o



all:: sfs

sfs: $(SFS_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SFS_OBJS) $(LIB)

clean:
	rm -f sfs src/*.o
