#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "include/mqtt_comm.h"
#include "lwipopts.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/***************************************************************
* PRIVATE VARIABLES
****************************************************************/
static uint32_t ultima_timestamp_recebida = 0;
static mqtt_client_t *client = NULL;

/***************************************************************
* PRIVATE FUNCTIONS
****************************************************************/
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) 
{
    if (status == MQTT_CONNECT_ACCEPTED) 
    {
        printf("Conectado ao broker MQTT com sucesso!\n");
    } 
    else 
    {
        printf("Falha ao conectar ao broker, código: %d\n", status);
    }
}

static void mqtt_pub_request_cb(void *arg, err_t result) 
{
    if (result == ERR_OK) 
    {
        printf("Publicação MQTT enviada com sucesso!\n");
    } 
    else 
    {
        printf("Erro ao publicar via MQTT: %d\n", result);
    }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) 
{
    printf("Mensagem recebida no tópico: %s (tam: %u)\n", topic, tot_len);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) 
{
    char msg[128] = {0};
    if (len >= sizeof(msg)) len = sizeof(msg) - 1;

    memcpy(msg, data, len);
    msg[len] = '\0';

    uint32_t nova_timestamp;
    float valor;

    if (sscanf(msg, "{\"valor\":%f,\"ts\":%lu}", &valor, &nova_timestamp) != 2) 
    {
        printf("Erro no parse da mensagem: %s\n", msg);
        return;
    }

    if (nova_timestamp > ultima_timestamp_recebida) 
    {
        ultima_timestamp_recebida = nova_timestamp;
        printf("Nova leitura: %.2f (ts: %lu)\n", valor, nova_timestamp);       
    } 
    else 
    {
        printf("Replay detectado (ts: %lu <= %lu)\n", nova_timestamp, ultima_timestamp_recebida);
    }
}

/***************************************************************
* PUBLIC FUNCTIONS
****************************************************************/
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass) 
{
    ip_addr_t broker_addr;
    
    if (!ip4addr_aton(broker_ip, &broker_addr)) 
    {
        printf("Erro no IP: %s\n", broker_ip);
        return;
    }

    client = mqtt_client_new();
    if (client == NULL) 
    {
        printf("Falha ao criar o cliente MQTT\n");
        return;
    }

    struct mqtt_connect_client_info_t ci = {
        .client_id = client_id,
        .client_user = user,
        .client_pass = pass,
    };

    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

    mqtt_client_connect(client, &broker_addr, 1884, mqtt_connection_cb, NULL, &ci);
}

void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len) 
{
    err_t status = mqtt_publish(
        client,              // Instância do cliente
        topic,               // Tópico de publicação
        data,                // Dados a serem enviados
        len,                 // Tamanho dos dados
        0,                   // QoS 0
        0,                   // Não reter
        mqtt_pub_request_cb, // Callback
        NULL
    );

    if (status != ERR_OK) 
    {
        printf("mqtt_publish falhou: %d\n", status);
    }
}

void mqtt_comm_subscribe(const char *topic) 
{
    err_t status = mqtt_subscribe(
        client,
        topic,
        0,          // QoS
        NULL,       // Callback opcional de confirmação
        NULL
    );

    if (status != ERR_OK) 
    {
        printf("mqtt_subscribe falhou: %d\n", status);
    }
}