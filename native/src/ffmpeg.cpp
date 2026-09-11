
#include <cstdio>

extern "C" {

#include <openssl/store.h>
#include <openssl/ui.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>

}

#include "ffmpeg.h"

static void log_callback(void *, int level, const char *fmt, va_list vl)
{
    vfprintf(stderr, fmt, vl);
}

static void dump_version() {
    
    
    printf("AV version: %s\n", av_version_info());
    printf("AV format version: %d\n", avformat_version());

    extern const URLProtocol ff_file_protocol;

    printf("name      = %s\n", ff_file_protocol.name);
    printf("url_read  = %p\n", (void *)ff_file_protocol.url_read);
    printf("url_write = %p\n", (void *)ff_file_protocol.url_write);

    void* opaque = nullptr;
    const char* name;

    printf("Input protocols:\n");
    while ((name = avio_enum_protocols(&opaque, 0))) {
        printf("\t%s\n", name);
    }
}

static void dump_ssl_store() {
     OSSL_STORE_CTX *ctx = OSSL_STORE_open(
        "org.openssl.winstore:",
        UI_get_default_method(),
        nullptr,
        nullptr,
        nullptr
    );

    if (!ctx) {
        fprintf(stderr, "Failed to open Windows certificate store\n");

        unsigned long err;
        while ((err = ERR_get_error()) != 0) {
            char buf[256];
            ERR_error_string_n(err, buf, sizeof(buf));
            fprintf(stderr, "OpenSSL: %s\n", buf);
        }
    } else {
        fprintf(stderr, "Successfully opened Windows certificate store\n");
        OSSL_STORE_close(ctx);
    }
}

static void dump_ssl_paths() {

    int ret;

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    if (!ctx) {
        printf("SSL_CTX_new failed\n");
        return;
    }

    ret = SSL_CTX_load_verify_store(
        ctx,
        "org.openssl.winstore:"
    );

    printf("SSL_CTX_load_verify_store: %d\n", ret);

    if (!ret) {
        ERR_print_errors_fp(stderr);
    }


    ret = SSL_CTX_set_default_verify_paths(ctx);

    printf("SSL_CTX_set_default_verify_paths: %d\n", ret);

    X509_STORE *store = SSL_CTX_get_cert_store(ctx);

    STACK_OF(X509_OBJECT) *objs = X509_STORE_get0_objects(store);

    printf("Objects in X509_STORE: %d\n",
        objs ? sk_X509_OBJECT_num(objs) : -1);

    SSL_CTX_free(ctx);
}

static void dump_ssl() {

    printf("OpenSSL: %s\n", OpenSSL_version(OPENSSL_VERSION));
   
    dump_ssl_store();
    dump_ssl_paths();
}

void init_ffmpeg() {

#ifdef FFMPEG_DEBUG
    dump_version();
    dump_ssl();

    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_callback(log_callback);
#else
    av_log_set_level(AV_LOG_QUIET);
#endif
}
