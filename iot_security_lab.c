// Bibliotecas necessárias
#include <string.h>                 // Para funções de string como strlen()
#include "pico/stdlib.h"            // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"        // Driver WiFi para Pico W
#include "include/wifi_conn.h"      // Funções personalizadas de conexão WiFi
#include "include/mqtt_comm.h"      // Funções personalizadas para MQTT
#include "include/xor_cipher.h"     // Funções de cifra XOR
#include <time.h>                   // Funções para tempo

#define WIFI_SSID "Holy"
#define WIFI_PASSWORD "CowCowCow"
#define BROKER_IP "192.168.137.1"
#define BROKER_PASSWORD "senha123"
#define TOPIC "escola/sala1/temperatura"

#define INIT_AS_PUBLISHER 1
#define USING_CRYPTOGRAPHY 0
#define NO_REPLAY 1

int main() 
{
    stdio_init_all();
    
    connect_to_wifi( WIFI_SSID , WIFI_PASSWORD );

    mqtt_setup( "bitdog1", BROKER_IP , "aluno", BROKER_PASSWORD );



#if INIT_AS_PUBLISHER
    char mensagem[50] = "26.5";
#if USING_CRYPTOGRAPHY
    uint8_t criptografada[50];
    xor_encrypt((uint8_t *)mensagem, criptografada, strlen(mensagem), 23);
    mensagem = criptografada;
#endif
#endif

#if !INIT_AS_PUBLISHER
    mqtt_comm_subscribe( TOPIC );
#endif

    while (true) 
    {
#if INIT_AS_PUBLISHER
#if NO_REPLAY
    sprintf(mensagem, "{\"valor\":26.5,\"ts\":%lu}", time(NULL));
#endif
    mqtt_comm_publish( TOPIC , mensagem, strlen(mensagem));
#endif
        sleep_ms(5000);
    }
    return 0;
}

/* 
 * Comandos de terminal para testar o MQTT:
 * 
 * Inicia o broker MQTT com logs detalhados:
 * mosquitto -c mosquitto.conf -v
 * 
 * Assina o tópico de temperatura (recebe mensagens):
 * mosquitto_sub -h localhost -p 1883 -t "escola/sala1/temperatura" -u "aluno" -P "senha123"
 * 
 * Publica mensagem de teste no tópico de temperatura:
 * mosquitto_pub -h localhost -p 1883 -t "escola/sala1/temperatura" -u "aluno" -P "senha123" -m "26.6"
 */