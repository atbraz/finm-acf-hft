#include <iostream>
#include <iomanip>
#include <deque>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

static volatile sig_atomic_t running = 1;

void handleSignal(int) { running = 0; }

void receiveAndRespond(int socketFd, const string& name) {
    char buffer[BUFFER_SIZE];
    deque<float> priceHistory;

    int ticksReceived = 0;
    int ordersUp = 0;
    int ordersDown = 0;
    int skipped = 0;
    int wins = 0;
    int losses = 0;
    double totalPnl = 0.0;

    FILE* logFile = fopen("build/prices.log", "w");
    if (logFile)
        fprintf(logFile, "price_id,price,action\n");
    else
        cerr << "[WARN] Could not open build/prices.log for logging." << endl;

    bool pendingTrade = false;
    float entryPrice = 0.0f;
    bool entryWasUp = false;

    send(socketFd, name.c_str(), name.size(), 0);

    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(socketFd, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            if (!running)
                break;
            cerr << "Server closed connection or error occurred." << endl;
            break;
        }

        string data(buffer);
        size_t commaPos = data.find(',');
        if (commaPos == string::npos) {
            cerr << "Invalid price format received: " << data << endl;
            continue;
        }

        int priceId = stoi(data.substr(0, commaPos));
        float price = stof(data.substr(commaPos + 1));
        ticksReceived++;

        cout << "[RECV] Price ID: " << priceId << ", Value: " << price << endl;

        /*
         * Settle the previous trade against the current price.
         * PnL is computed as the price change in the direction
         * of the momentum bet: (current - entry) for long,
         * (entry - current) for short.
         */
        if (pendingTrade) {
            float pnl = entryWasUp ? (price - entryPrice) : (entryPrice - price);
            totalPnl += pnl;
            if (pnl > 0) wins++;
            else losses++;
            pendingTrade = false;
        }

        if (priceHistory.size() >= 3)
            priceHistory.pop_front();
        priceHistory.push_back(price);

        if (priceHistory.size() == 3) {
            float a = priceHistory[0];
            float b = priceHistory[1];
            float c = priceHistory[2];

            bool up = (a < b) && (b < c);
            bool down = (a > b) && (b > c);

            if (up || down) {
                this_thread::sleep_for(chrono::milliseconds(10 + rand() % 41));
                string order = to_string(priceId);
                send(socketFd, order.c_str(), order.length(), 0);
                cout << "[ORDER] Momentum " << (up ? "up" : "down")
                     << ". Sent order for price ID: " << priceId << endl;

                pendingTrade = true;
                entryPrice = price;
                entryWasUp = up;
                if (up) ordersUp++;
                else ordersDown++;
                if (logFile) {
                    fprintf(logFile, "%d,%f,%s\n", priceId, price, up ? "order_up" : "order_down");
                    fflush(logFile);
                }
            } else {
                skipped++;
                cout << "[SKIP] No momentum. Ignoring price ID: " << priceId << endl;
                if (logFile) {
                    fprintf(logFile, "%d,%f,skip\n", priceId, price);
                    fflush(logFile);
                }
            }
        } else {
            if (logFile) {
                fprintf(logFile, "%d,%f,skip\n", priceId, price);
                fflush(logFile);
            }
        }
    }

    if (logFile) fclose(logFile);
    close(socketFd);

    int totalOrders = ordersUp + ordersDown;
    int settled = wins + losses;

    cout << "\n--- Session Stats ---" << endl;
    cout << "Ticks received:    " << ticksReceived << endl;
    cout << "Orders sent:       " << totalOrders
         << " (up: " << ordersUp << ", down: " << ordersDown << ")" << endl;
    cout << "Skipped:           " << skipped << endl;
    if (ticksReceived > 0) {
        cout << "Hit rate:          " << fixed << setprecision(1)
             << (100.0 * totalOrders / ticksReceived) << "%" << endl;
    }
    if (settled > 0) {
        cout << "Settled trades:    " << settled
             << " (wins: " << wins << ", losses: " << losses << ")" << endl;
        cout << "Win rate:          " << fixed << setprecision(1)
             << (100.0 * wins / settled) << "%" << endl;
        cout << "Total PnL:         " << fixed << setprecision(4) << totalPnl << endl;
        cout << "Avg PnL/trade:     " << fixed << setprecision(4)
             << (totalPnl / settled) << endl;
    }
}

int main() {
    signal(SIGINT, handleSignal);
    srand(time(nullptr));

    string name;
    cout << "Enter your client name: ";
    getline(cin, name);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Socket creation failed!" << endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Connection to server failed!" << endl;
        return 1;
    }

    cout << "[CONN] Connected to server at " << SERVER_IP << ":" << SERVER_PORT << endl;
    receiveAndRespond(sock, name);
    return 0;
}
