#ifdef __cplusplus
extern "C" {
#endif

typedef void (*socket_io_recv)(const char *data);

int socket_io_init(const char * uid);// return 0 is OK, other is error
void socket_io_register_recv_cb(socket_io_recv cb);
int socket_io_send(const char *data);// 0 ok, -1 not connect

#ifdef __cplusplus
}
#endif
