#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
using namespace std;

const int MAX_N = 50;
const int INF = 1000000000;
int n;
string nodeName[MAX_N];
int people[MAX_N], capacity[MAX_N];
int corridor[MAX_N][MAX_N], shortest[MAX_N][MAX_N];
bool isExit[MAX_N], blocked[MAX_N][MAX_N];

int readInt(string prompt, int low, int high) {
    string line;
    int value;
    char extra;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) exit(0);
        stringstream input(line);
        if ((input >> value) && !(input >> extra) && value >= low && value <= high)
            return value;
        cout << "Enter a whole number from " << low << " to " << high << ".\n";
    }
}

void initialize(int count) {
    n = count;
    for (int i = 0; i < n; i++) {
        people[i] = capacity[i] = 0;
        isExit[i] = false;
        nodeName[i] = "";
        for (int j = 0; j < n; j++) {
            corridor[i][j] = (i == j ? 0 : INF);
            blocked[i][j] = false;
        }
    }
}

void addCorridor(int u, int v, int length) {
    corridor[u][v] = corridor[v][u] = min(corridor[u][v], length);
}

int edgeWeight(int u, int v) {
    return blocked[u][v] ? INF : corridor[u][v];
}

void loadDemo() {
    initialize(8);
    string names[] = {"Ward_A", "ICU", "Ward_B", "Junction_A",
                      "Junction_B", "Exit_East", "Exit_West", "Isolation"};
    for (int i = 0; i < n; i++) nodeName[i] = names[i];
    people[0] = 4; people[1] = 3; people[2] = 5; people[7] = 2;
    isExit[5] = isExit[6] = true;
    capacity[5] = 5; capacity[6] = 6;
    addCorridor(0, 3, 4); addCorridor(1, 3, 2); addCorridor(2, 4, 3);
    addCorridor(3, 4, 2); addCorridor(3, 6, 6); addCorridor(4, 5, 4);
}

void loadCustom() {
    initialize(readInt("Number of locations (2-50): ", 2, MAX_N));
    for (int i = 0; i < n; i++) {
        do {
            cout << "Name for location " << i << " (1-24 characters): ";
            if (!getline(cin, nodeName[i])) exit(0);
        } while (nodeName[i].empty() || nodeName[i].size() > 24);
    }
    int exits = readInt("Number of exits: ", 1, n - 1);
    for (int i = 0; i < exits; i++) {
        int id = readInt("Exit location ID: ", 0, n - 1);
        if (isExit[id]) {
            cout << "Choose an exit ID not already selected.\n";
            i--;
            continue;
        }
        isExit[id] = true;
        capacity[id] = readInt("Exit capacity in people (0 means closed): ", 0, 10000);
    }
    for (int i = 0; i < n; i++) if (!isExit[i]) {
        cout << "Occupancy for " << i << " (" << nodeName[i] << ")\n";
        people[i] = readInt("People here (0 for a junction): ", 0, 10000);
    }
    int edges = readInt("Number of corridors: ", 0, n * (n - 1) / 2);
    for (int i = 0; i < edges; i++) {
        cout << "Corridor " << i + 1 << "\n";
        int u = readInt("First location ID: ", 0, n - 1);
        int v = readInt("Second location ID: ", 0, n - 1);
        if (u == v || corridor[u][v] != INF) {
            cout << "Choose distinct locations without an existing corridor.\n";
            i--;
            continue;
        }
        addCorridor(u, v, readInt("Length in metres: ", 1, 1000000));
    }
}

void showGraph() {
    cout << "\nLOCATIONS\nID  Name                      People  Exit capacity\n";
    for (int i = 0; i < n; i++) {
        cout << left << setw(4) << i << setw(26) << nodeName[i]
             << setw(8) << people[i];
        if (isExit[i]) cout << capacity[i];
        else cout << "-";
        cout << "\n";
    }
    cout << "\nCORRIDORS (two-way, metres)\n";
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (corridor[i][j] != INF)
                cout << i << " <-> " << j << " : " << corridor[i][j]
                     << (blocked[i][j] ? " [BLOCKED]\n" : "\n");
}

void floydWarshall() {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) shortest[i][j] = edgeWeight(i, j);
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (shortest[i][k] != INF && shortest[k][j] != INF)
                    shortest[i][j] = min(shortest[i][j], shortest[i][k] + shortest[k][j]);
}

void showDistances() {
    floydWarshall();
    cout << "\nFLOYD-WARSHALL: LOCATION-TO-EXIT DISTANCES (metres)\n";
    cout << left << setw(28) << "Location";
    for (int e = 0; e < n; e++) if (isExit[e]) cout << setw(12) << e;
    cout << "\n";
    for (int i = 0; i < n; i++) {
        if (isExit[i]) continue;
        cout << setw(28) << nodeName[i];
        for (int e = 0; e < n; e++) if (isExit[e]) {
            if (shortest[i][e] == INF) cout << setw(12) << "INF";
            else cout << setw(12) << shortest[i][e];
        }
        cout << "\n";
    }
    cout << "INF = no route. Exit IDs match the location list.\n";
}

void dijkstra(int source, int dist[], int parent[]) {
    bool visited[MAX_N] = {};
    for (int i = 0; i < n; i++) { dist[i] = INF; parent[i] = -1; }
    dist[source] = 0;
    for (int step = 0; step < n; step++) {
        int u = -1;
        for (int i = 0; i < n; i++)
            if (!visited[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] == INF) break;
        visited[u] = true;
        for (int v = 0; v < n; v++) {
            int w = edgeWeight(u, v);
            if (!visited[v] && w != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
            }
        }
    }
}

void printPath(int destination, int parent[]) {
    vector<int> path;
    for (int v = destination; v != -1; v = parent[v]) path.push_back(v);
    for (int i = (int)path.size() - 1; i >= 0; i--) {
        cout << nodeName[path[i]];
        if (i > 0) cout << " -> ";
    }
}

void planEvacuation() {
    floydWarshall();
    int remaining[MAX_N], assigned = 0, total = 0;
    int unreachable = 0, capacityShortfall = 0, longest = 0;
    long long totalDistance = 0;
    for (int i = 0; i < n; i++) { remaining[i] = capacity[i]; total += people[i]; }
    cout << "\nEVACUATION PLAN\nRooms: increasing ID. Distance ties: lower exit ID.\n";
    for (int room = 0; room < n; room++) {
        if (isExit[room] || people[room] == 0) continue;
        int dist[MAX_N], parent[MAX_N], waiting = people[room];
        dijkstra(room, dist, parent);
        bool reachable = false;
        for (int e = 0; e < n; e++)
            if (isExit[e] && shortest[room][e] != INF) reachable = true;
        while (waiting > 0) {
            int best = -1;
            for (int e = 0; e < n; e++)
                if (isExit[e] && remaining[e] > 0 && shortest[room][e] != INF &&
                    (best == -1 || shortest[room][e] < shortest[room][best])) best = e;
            if (best == -1) break;
            int count = min(waiting, remaining[best]);
            cout << count << " people: ";
            printPath(best, parent);
            cout << " (" << dist[best] << " m per person)\n";
            remaining[best] -= count; waiting -= count; assigned += count;
            totalDistance += (long long)count * dist[best];
            longest = max(longest, dist[best]);
        }
        if (waiting > 0) {
            cout << nodeName[room] << ": " << waiting << " UNASSIGNED ("
                 << (reachable ? "reachable exits full/closed" : "no reachable exit") << ")\n";
            if (reachable) capacityShortfall += waiting;
            else unreachable += waiting;
        }
    }
    cout << "\nEXIT USAGE\n";
    for (int e = 0; e < n; e++) if (isExit[e])
        cout << nodeName[e] << ": " << capacity[e] - remaining[e] << "/" << capacity[e]
             << " people; spare " << remaining[e] << "\n";
    cout << "Assigned: " << assigned << "/" << total << "\nUnassigned: " << total - assigned
         << " (unreachable " << unreachable << ", capacity shortfall " << capacityShortfall << ")\n";
    cout << "Total assigned travel: " << totalDistance << " person-metres\n";
    if (assigned > 0)
        cout << fixed << setprecision(2) << "Mean assigned route: "
             << (double)totalDistance / assigned << " m\nLongest assigned route: " << longest << " m\n";
    cout << "Static planning estimate; corridor congestion and evacuation time are not modelled.\n";
}

void toggleCorridor() {
    int u = readInt("First location ID: ", 0, n - 1);
    int v = readInt("Second location ID: ", 0, n - 1);
    if (u == v || corridor[u][v] == INF) { cout << "No corridor between these locations.\n"; return; }
    blocked[u][v] = blocked[v][u] = !blocked[u][v];
    cout << (blocked[u][v] ? "Corridor blocked.\n" : "Corridor restored.\n");
    cout << "Run a new plan to recompute routes and assignments.\n";
}

int main(int argc, char* argv[]) {
    loadDemo();
    cout << "HOSPITAL EVACUATION PLANNER | GROUP 13 | ROHIT AND SHUBHAM\n";
    if (argc > 1 && string(argv[1]) == "--demo") {
        showGraph(); showDistances(); planEvacuation(); return 0;
    }
    cout << "Demo floor plan loaded.\n";
    while (true) {
        cout << "\n1. Show floor plan\n2. Room-to-exit distance table\n3. Generate evacuation plan\n"
             << "4. Block/restore corridor\n5. Enter custom floor plan\n6. Reset demo\n0. Exit\n";
        int choice = readInt("Choice: ", 0, 6);
        if (choice == 0) break;
        if (choice == 1) showGraph();
        if (choice == 2) showDistances();
        if (choice == 3) planEvacuation();
        if (choice == 4) toggleCorridor();
        if (choice == 5) loadCustom();
        if (choice == 6) { loadDemo(); cout << "Demo reset.\n"; }
    }
    return 0;
}
