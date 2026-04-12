#include "StochasticProcesses.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <atomic>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utility>

using namespace std;
using namespace std::chrono;

#define PORT 12345
#define BUFFER_SIZE 1024

struct ClientInfo {
    int socket;
    string name;
    thread clientThread;
};

vector<unique_ptr<ClientInfo>> clients;
mutex clientsMutex;

unordered_map<int, steady_clock::time_point> priceTimestamps;
unordered_set<int> priceAlreadyHit;
mutex priceMutex;

atomic<int> priceId{0};

/*
 * Evolve a single instrument using Heston stochastic volatility,
 * Ornstein Uhlenbeck drift, and Kou double exponential jumps.
 * Each broadcast advances the model by SUBSTEPS micro steps so
 * that the 5 second broadcast interval produces visible price
 * movement despite the small per step dt.
 */
void broadcastPrices() {
    constexpr double S0 = 150.0;
    constexpr double dt = 1.0 / (252.0 * 23400.0);
    constexpr int SUBSTEPS = 2000;
    constexpr double rho = -0.7;
    constexpr double lambda = 2.0;
    constexpr double p_up = 0.4;
    constexpr double eta1 = 20.0;
    constexpr double eta2 = 14.0;

    double ln_S = std::log(S0);
    HestonProcess heston(2.0, 0.04, 0.3, 0.04);
    DriftProcess drift(0.3, 0.05, 1.0, 0.05);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::normal_distribution<> normal(0.0, 1.0);

    while (true) {
        for (int s = 0; s < SUBSTEPS; ++s) {
            double v_old = heston.variance();
            double Z_S = normal(rng);

            drift.step(normal(rng), dt);

            double Z_v = rho * Z_S + std::sqrt(1.0 - rho * rho) * normal(rng);
            heston.step(Z_v, dt);

            double J = 0.0;
            if (KouJumpProcess::sampleArrival(lambda, dt, rng))
                J += KouJumpProcess::sampleSize(p_up, eta1, eta2, rng);

            ln_S += (drift.drift() - v_old / 2.0) * dt
                    + std::sqrt(std::max(v_old, 0.0) * dt) * Z_S + J;
        }

        int id = priceId++;
        double price = std::exp(ln_S);
        string message = to_string(id) + "," + to_string(price);

        {
            lock_guard<mutex> lock(priceMutex);
            priceTimestamps[id] = steady_clock::now();
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            for (auto& client : clients) {
                send(client->socket, message.c_str(), message.size(), 0);
            }
        }

        cout << "[PRICE] Sent price ID " << id << " with value " << price << endl;
        this_thread::sleep_for(chrono::seconds(5));
    }
}

// Handle a client connection
void handleClient(ClientInfo* client) {
    char buffer[BUFFER_SIZE];

    // Receive client name
    memset(buffer, 0, BUFFER_SIZE);
    int bytesReceived = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived <= 0) {
        cerr << "[ERROR] Failed to receive client name." << endl;
        close(client->socket);
        return;
    }

    client->name = string(buffer);
    cout << "[JOIN] Registered client: " << client->name << endl;

    // Receive orders
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        bytesReceived = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            cerr << "[ERROR] Client " << client->name << " disconnected." << endl;
            break;
        }

        int receivedPriceId = atoi(buffer);
        steady_clock::time_point now = steady_clock::now();

        {
            lock_guard<mutex> lock(priceMutex);
            if (priceAlreadyHit.count(receivedPriceId)) {
                // Already hit by another client
                continue;
            }

            if (priceTimestamps.find(receivedPriceId) == priceTimestamps.end()) {
                cerr << "[WARN] Unknown price ID: " << receivedPriceId << endl;
                continue;
            }

            priceAlreadyHit.insert(receivedPriceId);
            auto latency = duration_cast<milliseconds>(now - priceTimestamps[receivedPriceId]).count();
            cout << "[HIT] " << client->name << " hit price ID " << receivedPriceId
                 << " after " << latency << " ms" << endl;
        }
    }

    close(client->socket);
}

// Start the server
void startServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);  // Localhost

    if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    cout << "[START] Server is listening on 127.0.0.1:" << PORT << endl;

    thread priceThread(broadcastPrices);
    priceThread.detach();

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }

        cout << "[CONN] Client connected: " << inet_ntoa(clientAddr.sin_addr) << endl;

        // Create a new client object on the heap
        auto client = make_unique<ClientInfo>();
        client->socket = clientSocket;

        // Store a pointer before moving ownership
        ClientInfo* clientPtr = client.get();

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(std::move(client));
        }

        // Launch the thread with raw pointer (safe because we hold it in the vector)
        thread t(handleClient, clientPtr);
        t.detach();
        clientPtr->clientThread = std::move(t);  // Optional
    }

    close(serverSocket);
}

int main() {
    startServer();
    return 0;
}
