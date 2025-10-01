#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/errno.h>

#include "debug.h"

#include "msg.h"

/*
 * Run like: ./app <name> <time_s>
 */
int main(int argc, char *argv[]) {
    /*Garante que o utilizador passou os 2 argumentos obrigatórios.
    Se não passou, mostra mensagem de uso e termina o programa.*/
    if (argc != 3) {
        printf("Usage: %s <name> <time_s>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse arguments
    //Guarda o nome da aplicação (string).
    const char *app_name = argv[1];

    /*Converte argv[2] (string do tempo em segundos) para número (long).

    endptr é usado para verificar se a conversão correu bem.
    Base 10 (decimal).
    errno = 0 para detectar se a função strtol gerou algum erro.*/

    char *endptr;
    errno = 0;
    long val = strtol(argv[2], &endptr, 10);
    if (errno != 0) {
        perror("strtol");  // conversion error (overflow, etc.)
        return 1;
    }
    /*Garante que o argumento era mesmo só um número.
    Se houvesse caracteres extra (ex: "10abc"), dá erro.*/
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid number: %s\n", argv[2]);
        return 1;
    }
    /*Verifica que o valor está dentro de um intervalo válido (não negativo, nem maior que INT_MAX).*/
    if (val < 0 || val > INT_MAX) {  // optional range check
        fprintf(stderr, "Value out of range: %ld\n", val);
        return 1;
    }
    /*Converte o valor para inteiro de 32 bits → este é o tempo em segundos que a aplicação quer de CPU.*/
    int32_t time_s = (int32_t) val;

    // Setup socket for communication
    //Cria um socket do tipo UNIX (não usa TCP/IP, mas sim comunicação local entre processos).Se falhar, dá erro e sai.
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    /*Prepara o endereço do socket:
    AF_UNIX → comunicação local.
    SOCKET_PATH → caminho do ficheiro especial do socket (deve estar definido noutro .h).*/
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    /*Liga-se ao simulador (servidor).
    Se não conseguir ligar, mostra erro e termina.*/
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // All in place to start simulating the app
    //Mensagem para o utilizador: esta aplicação começou e precisa de X segundos de CPU.
    printf("Application %s started, will need the CPU for %d seconds\n", app_name, time_s);

    // Send RUN request
    /*Prepara uma mensagem para o simulador:
    pid → ID do processo da aplicação.
    request → tipo de pedido (aqui: “RUN”, ou seja, quero CPU).
    time_ms → tempo em milissegundos.*/
    pid_t pid = getpid();
    msg_t msg = {
        .pid = pid,
        .request = PROCESS_REQUEST_RUN,
        .time_ms = time_s * 1000
    };

    /*Envia a mensagem pelo socket.
    Se não conseguir enviar tudo, dá erro.*/
    if (write(sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
        perror("write");
        close(sockfd);
        return EXIT_FAILURE;
    }
    //Macro de debug (provavelmente só imprime se DEBUG estiver ativo).
    DBG("Application %s (PID %d) sent RUN request for %d ms",
           app_name, pid, msg.time_ms);
    // Wait for ACK and the internal simulation time
    /*Lê uma mensagem do simulador.
    Espera um ACK (acknowledge) a confirmar que o pedido foi aceite.
    Se não for ACK, dá erro.*/
    if (read(sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
        perror("read");
        close(sockfd);
        return EXIT_FAILURE;
    }
    if (msg.request != PROCESS_REQUEST_ACK) {
        printf("Received invalid request. Expected ACK\n");
        return EXIT_FAILURE;
    }

    // Received ACK
    //Guarda o tempo em que o simulador disse que a app começou a correr.
    uint32_t start_time_ms = msg.time_ms;
//    printf("Application %s (PID %d) started running at time %d ms\n", app_name, pid, start_time_ms);

    // Wait for the EXIT message
    /*Espera outra mensagem do simulador.
    Desta vez, tem de ser um DONE/EXIT → significa que o processo terminou no simulador.*/
    if (read(sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
        perror("read");
        close(sockfd);
        return EXIT_FAILURE;
    }
    if (msg.request != PROCESS_REQUEST_DONE) {
        printf("Received invalid request. Expected EXIT\n");
    }

    // Received EXIT, print stats

    /*Estatísticas finais:
        real → tempo real decorrido (fim - início).
        user → tempo que pediu de CPU (tempo_s).
        sys → tempo de espera (diferença entre real e CPU).*/
    double real = (msg.time_ms - start_time_ms) / 1000.0;
    double user = (double) time_s;
    double sys = real - time_s;

    //Mostra os resultados finais: nome da aplicação, PID, tempo de fim, tempo total (Elapsed), tempo de CPU usado.
    printf("Application %s (PID %d) finished at time %d ms, Elapsed: %.03f seconds, CPU: %.03f seconds\n",
           app_name, pid, msg.time_ms, real, user);

    //Fecha o socket e termina o programa com sucesso.
    close(sockfd);
    return EXIT_SUCCESS;
}