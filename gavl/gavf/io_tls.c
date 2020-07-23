
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <gavl/gavf.h>

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

/* Global initialization */

static int tls_initialized = 0;
static pthread_mutex_t initialized_mutex = PTHREAD_MUTEX_INITIALIZER;
static gnutls_certificate_credentials_t xcred;

#define BUFFER_SIZE 10240

static void tls_global_init()
  {
  pthread_mutex_lock(&initialized_mutex);

  if(tls_initialized)
    {
    pthread_mutex_unlock(&initialized_mutex);
    return;
    }

  if(gnutls_global_init() < 0)
    {
    fprintf(stderr, "gnutls_global_init failed\n");
    }
  
  if(gnutls_certificate_allocate_credentials(&xcred) < 0)
    {
    fprintf(stderr, "gnutls_certificate_allocate_credentials failed\n");
    }
  
  if(gnutls_certificate_set_x509_system_trust(xcred) < 0)
    {
    fprintf(stderr, "gnutls_certificate_set_x509_system_trust failed\n");
    }
  tls_initialized = 1;
  pthread_mutex_unlock(&initialized_mutex);

  }


#if defined(__GNUC__)

static void cleanup_tls() __attribute__ ((destructor));

static void cleanup_tls()
  {
  pthread_mutex_lock(&initialized_mutex);

  if(!tls_initialized)
    {
    pthread_mutex_unlock(&initialized_mutex);
    return;
    }

  gnutls_certificate_free_credentials(xcred);
  gnutls_global_deinit();
  pthread_mutex_unlock(&initialized_mutex);
  }

#endif


typedef struct
  {
  int fd;
  gnutls_session_t session;  
  
  gavl_buffer_t read_buffer;
  gavl_buffer_t write_buffer;
  } tls_t;

static int read_record(tls_t * p)
  {
  ssize_t result;

  do
    {
    result = gnutls_record_recv(p->session, p->read_buffer.buf, BUFFER_SIZE);
    } while((result == GNUTLS_E_AGAIN) || (result == GNUTLS_E_INTERRUPTED));

  if(result > 0)
    {
    p->read_buffer.len = result;
    p->read_buffer.pos = 0;
    return 1;
    }
  return 0;
  }

static int flush_tls(void * priv)
  {
  tls_t * p = priv;

  if(p->write_buffer.len > p->write_buffer.pos)
    {
    ssize_t result;

    do
      {
      result = gnutls_record_send(p->session, p->write_buffer.buf + p->write_buffer.pos,
                                  p->write_buffer.len - p->write_buffer.pos);
      
      } while((result == GNUTLS_E_AGAIN) || (result == GNUTLS_E_INTERRUPTED));

    if(result <= 0)
      return 0;

    if(result > 0)
      p->write_buffer.pos += result;
    }

  p->write_buffer.len = 0;
  p->write_buffer.pos = 0;
  
  return 1;
  }

static int read_tls(void * priv, uint8_t * data, int len)
  {
  tls_t * p = priv;
  int bytes_read = 0;
  int bytes_to_read;

  if(!flush_tls(priv))
    return 0;
    
  while(bytes_read < len)
    {
    if(p->read_buffer.pos >= p->read_buffer.len)
      {
      if(!read_record(p))
        break;
      }

    bytes_to_read = len - bytes_read;

    if(bytes_to_read > p->read_buffer.len - p->read_buffer.pos)
      {
      bytes_to_read = p->read_buffer.len - p->read_buffer.pos;
      }

    memcpy(data + bytes_read, p->read_buffer.buf + p->read_buffer.pos, bytes_to_read);
    
    bytes_read += bytes_to_read;
    }
  
  return bytes_read;
  }


    
static int write_tls(void * priv, const uint8_t * data, int len)
  {
  int bytes_to_copy;
  int bytes_sent = 0;
  tls_t * p = priv;

  while(bytes_sent < len)
    {
    if(p->write_buffer.len == BUFFER_SIZE)
      {
      if(!flush_tls(priv))
        return 0;
      }

    bytes_to_copy = len - bytes_sent;

    if(bytes_to_copy > BUFFER_SIZE - p->write_buffer.len)
      {
      bytes_to_copy = BUFFER_SIZE - p->write_buffer.len;
      }

    memcpy(p->write_buffer.buf + p->write_buffer.len, data + bytes_sent, bytes_to_copy);
    
    p->write_buffer.len += bytes_to_copy;
    bytes_sent += bytes_to_copy;
    }
  return 1;
  }

// static int64_t (*gavf_seek_func)(void * priv, int64_t pos, int whence);

static void close_tls(void * priv)
  {
  tls_t * p = priv;

  gnutls_bye(p->session, GNUTLS_SHUT_RDWR);
  gnutls_deinit(p->session);

  gavl_buffer_free(&p->read_buffer);
  gavl_buffer_free(&p->write_buffer);

  free(p);
  }


gavf_io_t * gavf_io_create_tls_client(int fd, const char * server_name)
  {
  tls_t * p;
  int result;
  tls_global_init();

  p = calloc(1, sizeof(*p));

  
  
  gnutls_init(&p->session, GNUTLS_CLIENT);

  gnutls_server_name_set(p->session, GNUTLS_NAME_DNS, server_name,
                         strlen(server_name));

  gnutls_set_default_priority(p->session);

  gnutls_credentials_set(p->session, GNUTLS_CRD_CERTIFICATE, xcred);

  gnutls_session_set_verify_cert(p->session, server_name, 0);

  gnutls_transport_set_int(p->session, fd);
  gnutls_handshake_set_timeout(p->session,
                               GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

  /* Perform the TLS handshake
   */

  do{
    result = gnutls_handshake(p->session);
    }
  while(result < 0 && gnutls_error_is_fatal(result) == 0);

  if(result < 0)
    {
    if(result == GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR)
      {
      gnutls_datum_t out;
      unsigned status;
      /* check certificate verification status */
      int type = gnutls_certificate_type_get(p->session);
      status = gnutls_session_get_verify_cert_status(p->session);

      gnutls_certificate_verification_status_print(status, type, &out, 0);
      printf("cert verify output: %s\n", out.data);
      gnutls_free(out.data);
      }
    fprintf(stderr, "*** Handshake failed: %s\n", gnutls_strerror(result));
    goto fail;
    }
  else
    {
    char * desc = gnutls_session_get_desc(p->session);
    printf("- Session info: %s\n", desc);
    gnutls_free(desc);
    }

  gavl_buffer_alloc(&p->read_buffer, BUFFER_SIZE);
  gavl_buffer_alloc(&p->write_buffer, BUFFER_SIZE);
  
  return gavf_io_create(read_tls,
                        write_tls,
                        NULL,
                        close_tls,
                        flush_tls,
                        p);
  
  fail:

  close_tls(p);
  return NULL;
  }

