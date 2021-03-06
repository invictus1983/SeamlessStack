
JERASUREDIR = jerasure2.0
GF_COMPLETEDIR = gf-complete
MEMCACHEDDIR = libmemcached-1.0.8
MONGOCDIR = mongo-c-driver
BSONDIR = $(MONGOCDIR)/src/libbson
LZ4DIR = lz4
CONFIGURE = conf
OSS_DIR = $(PWD)
OSS_INSTALL_DIR = $(OSS_DIR)/oss_install
OSS_LIBSDIR = $(OSS_INSTALL_DIR)/lib
OSS_INCLUDEDIR = $(OSS_INSTALL_DIR)/include
GFP = $(shell pwd)/gf-complete
RBTREE_DIR = rbtree
CLISH_DIR = clish-0.7.3
ROCKSDB_DIR = rocksdb
CZMQ_DIR = czmq
RM = rm
CC = gcc
CP = cp
MKDIR = mkdir -p
CD = cd
SED = sed
MV = mv
CP = cp
LN = ln

all: $(CONFIGURE)
	$(MAKE) -C $(JERASUREDIR)
	$(MAKE) -C $(MEMCACHEDDIR)
	$(MAKE) -C $(MEMCACHEDDIR) install
	$(MAKE) -C $(MONGOCDIR)
	$(CP) $(JERASUREDIR)/src/.libs/*.so $(OSS_LIBSDIR)
	$(LN) -sf $(OSS_LIBSDIR)/libJerasure.so $(OSS_LIBSDIR)/libJerasure.so.2	
	$(CP) $(MONGOCDIR)/.libs/*.so $(OSS_LIBSDIR)
	$(CP) $(BSONDIR)/.libs/*.so $(OSS_LIBSDIR)
	$(LN) -sf $(OSS_LIBSDIR)/libmongoc-1.0.so $(OSS_LIBSDIR)/libmongoc.so
	$(LN) -sf $(OSS_LIBSDIR)/libbson-1.0.so $(OSS_LIBSDIR)/libbson.so
	$(MAKE) -C $(LZ4DIR) 
	$(CP) $(LZ4DIR)/*.so $(OSS_LIBSDIR)
	$(MAKE) -C $(RBTREE_DIR)
	$(CP) $(RBTREE_DIR)/*.so $(OSS_LIBSDIR)
	$(MAKE) -C $(CLISH_DIR)
	$(MAKE) -C $(ROCKSDB_DIR) shared_lib
	$(CP) $(ROCKSDB_DIR)/*.so $(OSS_LIBSDIR)
	$(MAKE) -C $(CZMQ_DIR)
	$(CP) $(CZMQ_DIR)/src/.libs/*.so $(OSS_LIBSDIR)

$(CONFIGURE): gf_comp jerasure_touch
	$(CD) $(MEMCACHEDDIR) && ./configure --prefix=$(OSS_INSTALL_DIR)
	$(CD) $(JERASUREDIR) && ./configure LDFLAGS=-L$(GFP)/src/.libs \
			CPPFLAGS=-I$(GFP)/include
	$(CD) $(CZMQ_DIR) && ./autogen.sh
	$(CD) $(CZMQ_DIR) && ./configure
	$(CD) $(MONGOCDIR) && ./autogen.sh
	$(CD) $(MONGOCDIR) && ./configure --with-libbson
	$(CD) $(MONGOCDIR) && git submodule init
	$(CD) $(MONGOCDIR) && git submodule update
	$(CD) $(BSONDIR) && ./autogen.sh
	$(CD) $(BSONDIR) && ./configure



gf_comp:
	$(MAKE) -C $(GF_COMPLETEDIR)

jerasure_touch:
	$(CD) $(JERASUREDIR) && touch configure.ac aclocal.m4 configure \
				Makefile.am Makefile.in

clean:
	$(MAKE) -C $(JERASUREDIR) clean
	$(MAKE) -C $(MEMCACHEDDIR) clean
	$(MAKE) -C $(MONGOCDIR) clean
	$(MAKE) -C $(BSONDIR) clean
	$(MAKE) -C $(LZ4DIR) clean
	$(MAKE) -C $(RBTREE_DIR) clean
	$(MAKE) -C $(CLISH_DIR) clean
	$(MAKE) -C $(GF_COMPLETEDIR) clean
	$(MAKE) -C $(ROCKSDB_DIR) clean
	$(MAKE) -C $(CZMQ_DIR) clean
