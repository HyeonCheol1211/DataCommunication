#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <libgen.h>
#define MAX_SIZE 300
#define IP_ADDRESS "127.0.0.1"

struct L
{
    int length;
    int type;
    char data[MAX_SIZE];
};

void L_send(char *input, int type);
char *L_receive(int *);
void data_send(char *data, int length);
char *data_receive(int *);

void *do_thread(void *);
void check_is_server(char *const *argv);
void init_socket();

void strwrSub(char str[]);
void struprSub(char str[]);

int sndsock, rcvsock, clen;
struct sockaddr_in s_addr, r_addr;
int is_server = 0;

int main(int argc, char *argv[])
{
    char input[MAX_SIZE];
    int type;
    pthread_t t_id;
    check_is_server(argv);
    init_socket();
    int status = pthread_create(&t_id, NULL, do_thread, NULL);
    if (status != 0)
    {
        printf("Thread Error!\n");
        exit(1);
    }
    while (1)
    {
        scanf("%s", input);
        scanf("%d", &type);
        L_send(input, type);
    }
}

void L_send(char *input, int type)
{
    struct L layer;
    char temp[350];
    int size = 0;

    layer.length = strlen(input);
    layer.type = type;
    if (type != 4)
    {
        printf("Sent Length : %d\n", layer.length);
    }

    memset(layer.data, 0x00, MAX_SIZE);
    memcpy(layer.data, (void *)input, layer.length);
    size = sizeof(struct L) - sizeof(layer.data) + layer.length + layer.type;

    memset(temp, 0x00, 350);
    memcpy(temp, (void *)&layer, size);
    data_send(temp, size);
}

char *L_receive(int *length)
{
    struct L *layer;
    layer = (struct L *)data_receive(length);
    *length = *length - sizeof(layer->length);
    if (layer->type == 0) {
    } else if (layer->type == 1) {
        for (int i = 0; i < layer->length; i++) {
            if (layer->data[i] >= 97 && layer->data[i] <= 122) {
                layer->data[i] = layer->data[i] - 32;
            }
        }
    } else if (layer->type == 2) {
        for (int i = 0; i < layer->length; i++) {
            if (layer->data[i] >= 65 && layer->data[i] <= 90) {
                layer->data[i] = layer->data[i] + 32;
            }
        }
    } else if (layer->type == 3) {
        printf("Echo Sent\n");
        printf("Sent Data : %s\n", layer->data);
        printf("Sent Type : 4\n");
        printf("Sent Length : %d\n", layer->length);
        L_send(layer->data, 4);
    } else if (layer->type == 4) {
        printf("Echo Received\n");
    } else {
        printf("Wrong Type\n");
        layer->data[0] = '\0';
        printf("####################################################\n");
        return (char *)layer->data;
    }

    printf("Received Length : %d\n", layer->length);
    printf("Received Type : %d\n", layer->type);

    return (char *)layer->data;
}
///////////////////////////////////////////////////////////////////////////////////////
void data_send(char *data, int length)
{
    sendto(sndsock, data, length, 0, (struct sockaddr *)&s_addr, sizeof(s_addr));
    printf("####################################################\n");
}

char *data_receive(int *length)
{
    static char data[MAX_SIZE];
    *length = recvfrom(rcvsock, data, MAX_SIZE, 0, (struct sockaddr *)&r_addr, &clen);
    return data;
}

void *do_thread(void *arg)
{
    char output[MAX_SIZE];
    int length;
    while (1)
    {
        strcpy(output, L_receive(&length));
        output[length] = '\0';
        if (strlen(output) > 1)
        {
            printf("Received Data: %s\n", output);
            printf("####################################################\n");
        }
    }
    return NULL;
}

void check_is_server(char *const *argv)
{
    char *progname = basename(argv[0]);
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "pgrep -x %s | wc -l", progname);
    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    int process_count = 0;
    char line[256];
    if (fgets(line, sizeof(line), fp) != NULL)
    {
        process_count = atoi(line);
    }
    pclose(fp);
    is_server = process_count == 1;
}

void init_socket()
{
    int send_port;
    int receive_port;
    if (is_server == 0)
    {
        printf("Session 2 Start\n");
        send_port = 8811;
        receive_port = 8810;
    }
    else
    {
        printf("Session 1 Start\n");
        send_port = 8810;
        receive_port = 8811;
    }
    if ((sndsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("socket error : ");
        exit(1);
    }
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    s_addr.sin_port = htons(send_port);
    if (connect(sndsock, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
    {
        perror("connect error : ");
        exit(1);
    }
    if ((rcvsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("socket error : ");
        exit(1);
    }

    clen = sizeof(r_addr);
    memset(&r_addr, 0, sizeof(r_addr));
    r_addr.sin_family = AF_INET;
    r_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    r_addr.sin_port = htons(receive_port);
    if (bind(rcvsock, (struct sockaddr *)&r_addr, sizeof(r_addr)) < 0)
    {
        perror("bind error : ");
        exit(1);
    }
    int optvalue = 1;
    int optlen = sizeof(optvalue);
    setsockopt(sndsock, SOL_SOCKET, SO_REUSEADDR, &optvalue, optlen);
    setsockopt(rcvsock, SOL_SOCKET, SO_REUSEADDR, &optvalue, optlen);
    printf("####################################################\n");
}
