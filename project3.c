// shop.h
#ifndef SHOP_H
#define SHOP_H

#define MAX_PRODUCTS 20
#define MAX_CLIENTS 5
#define MAX_DESC_LENGTH 50

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    char description[MAX_DESC_LENGTH];
    float price;
    int item_count;
} product;

// Function declarations
void initialize_catalog(product catalog[]);
void process_order(int client_socket, product catalog[], sem_t *catalog_lock);
void generate_report(product catalog[], int total_orders, int successful_orders, int failed_orders, float total_revenue);

#endif // SHOP_H

// server.c
#include "shop.h"

#define PORT 8080

// Global variables for tracking orders
int total_orders = 0;
int successful_orders = 0;
int failed_orders = 0;
float total_revenue = 0;

void initialize_catalog(product catalog[]) {
    for (int i = 0; i < MAX_PRODUCTS; i++) {
        snprintf(catalog[i].description, MAX_DESC_LENGTH, "Product %d", i + 1);
        catalog[i].price = (i + 1) * 10.0;
        catalog[i].item_count = 2;
    }
}

void process_order(int client_socket, product catalog[], sem_t *catalog_lock) {
    int product_id;
    int quantity;

    // Read product ID and quantity from client
    read(client_socket, &product_id, sizeof(int));
    read(client_socket, &quantity, sizeof(int));

    sem_wait(catalog_lock);
    total_orders++;

    if (product_id >= 0 && product_id < MAX_PRODUCTS && quantity > 0) {
        if (catalog[product_id].item_count >= quantity) {
            catalog[product_id].item_count -= quantity;
            float cost = catalog[product_id].price * quantity;
            total_revenue += cost;
            successful_orders++;

            // Send success message to client
            char response[256];
            snprintf(response, sizeof(response), "Order successful! Total cost: %.2f\n", cost);
            write(client_socket, response, strlen(response) + 1);
        } else {
            failed_orders++;

            // Send failure message to client
            char response[] = "Order failed! Not enough stock.\n";
            write(client_socket, response, strlen(response) + 1);
        }
    } else {
        failed_orders++;

        // Send invalid request message to client
        char response[] = "Order failed! Invalid product ID or quantity.\n";
        write(client_socket, response, strlen(response) + 1);
    }

    sem_post(catalog_lock);
}

void generate_report(product catalog[], int total_orders, int successful_orders, int failed_orders, float total_revenue) {
    printf("\n=== Sales Report ===\n");
    for (int i = 0; i < MAX_PRODUCTS; i++) {
        printf("%s: %d left\n", catalog[i].description, catalog[i].item_count);
    }
    printf("\nTotal Orders: %d\n", total_orders);
    printf("Successful Orders: %d\n", successful_orders);
    printf("Failed Orders: %d\n", failed_orders);
    printf("Total Revenue: %.2f\n", total_revenue);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    product catalog[MAX_PRODUCTS];
    sem_t catalog_lock;

    // Initialize catalog and semaphore
    initialize_catalog(catalog);
    sem_init(&catalog_lock, 0, 1);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        if (!fork()) {
            close(server_socket);
            process_order(client_socket, catalog, &catalog_lock);
            close(client_socket);
            exit(0);
        }
        close(client_socket);
    }

    // Cleanup
    sem_destroy(&catalog_lock);
    close(server_socket);

    generate_report(catalog, total_orders, successful_orders, failed_orders, total_revenue);

    return 0;
}

// client.c
#include "shop.h"

void client_behavior(int client_id) {
    int client_socket;
    struct sockaddr_in server_addr;

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection to server failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Client %d connected to the server.\n", client_id);

    for (int i = 0; i < 10; i++) {
        int product_id = rand() % MAX_PRODUCTS;
        int quantity = 1 + rand() % 2; // Request 1 or 2 items

        // Send product ID and quantity to server
        write(client_socket, &product_id, sizeof(int));
        write(client_socket, &quantity, sizeof(int));

        // Read server response
        char response[256];
        read(client_socket, response, sizeof(response));
        printf("Client %d: %s", client_id, response);

        sleep(1); // Wait 1 second before next order
    }

    close(client_socket);
    printf("Client %d disconnected.\n", client_id);
}

int main() {
    pid_t pids[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        pids[i] = fork();

        if (pids[i] == 0) {
            srand(getpid()); // Seed random number generator
            client_behavior(i + 1);
            exit(0);
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        wait(NULL); // Wait for all child processes to finish
    }

    return 0;
}
