#pragma once
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdexcept>
#include <string>

inline void tls_init_once() {
    static bool inited=false;
    if(!inited){
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        inited=true;
    }
}

inline std::string tls_err() {
    char buf[256]; unsigned long e = ERR_get_error();
    ERR_error_string_n(e, buf, sizeof(buf));
    return std::string(buf);
}

inline SSL_CTX* tls_server_ctx(const std::string& cert, const std::string& key) {
    tls_init_once();
    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if(!ctx) throw std::runtime_error("SSL_CTX_new: "+tls_err());
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    if (SSL_CTX_use_certificate_file(ctx, cert.c_str(), SSL_FILETYPE_PEM) <= 0)
        throw std::runtime_error("use_certificate: "+tls_err());
    if (SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM) <= 0)
        throw std::runtime_error("use_private_key: "+tls_err());
    if (!SSL_CTX_check_private_key(ctx))
        throw std::runtime_error("key_mismatch");
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5");
    return ctx;
}

inline SSL_CTX* tls_client_ctx(const std::string& ca_cert_path) {
    tls_init_once();
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if(!ctx) throw std::runtime_error("SSL_CTX_new: "+tls_err());
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    if (!SSL_CTX_load_verify_locations(ctx, ca_cert_path.c_str(), nullptr))
        throw std::runtime_error("load CA: "+tls_err());
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    X509_VERIFY_PARAM* param = SSL_CTX_get0_param(ctx);
    X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
#endif
    return ctx;
}

inline SSL* tls_wrap_fd_as_server(SSL_CTX* ctx, int fd) {
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    if (SSL_accept(ssl) <= 0) throw std::runtime_error("SSL_accept: "+tls_err());
    return ssl;
}

inline SSL* tls_wrap_fd_as_client(SSL_CTX* ctx, int fd, const std::string& hostname) {
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    SSL_set_tlsext_host_name(ssl, hostname.c_str());
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    X509_VERIFY_PARAM* param = SSL_get0_param(ssl);
    X509_VERIFY_PARAM_set1_host(param, hostname.c_str(), 0);
#endif
    if (SSL_connect(ssl) <= 0) throw std::runtime_error("SSL_connect: "+tls_err());
    long vr = SSL_get_verify_result(ssl);
    if (vr != X509_V_OK) throw std::runtime_error("verify fail");
    return ssl;
}

inline int tls_send(SSL* ssl, const void* data, int len) {
    int n = SSL_write(ssl, data, len);
    if(n <= 0) throw std::runtime_error("SSL_write: "+tls_err());
    return n;
}

inline int tls_recv(SSL* ssl, void* buf, int buflen) {
    int n = SSL_read(ssl, buf, buflen);
    if(n <= 0) throw std::runtime_error("SSL_read: "+tls_err());
    return n;
}

inline void tls_close(SSL* ssl) {
    if(!ssl) return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
}