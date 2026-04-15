#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <string>
#include <algorithm>

  // ZIP TREE CLASS

class ZipTree {
private:
    struct Node {
        int key;
        int rank;
        Node *left, *right;

        Node(int k, int r) : key(k), rank(r), left(nullptr), right(nullptr) {}
    };

    Node* root;
    long long comparison_count;
    long long change_count;

    // Modern C++ Randomness
    std::mt19937 rng;
    
    int generate_random_rank() {
        int rank = 0;
        std::uniform_int_distribution<int> dist(0, 1);
        while (dist(rng) == 1) {
            rank++;
        }
        return rank;
    }

    /* Core Operations */

    Node* zip(Node* x, Node* y) {
        if (!x) return y;
        if (!y) return x;

        comparison_count++;
        if (x->rank < y->rank) {
            change_count++;
            y->left = zip(x, y->left);
            return y;
        } else {
            change_count++;
            x->right = zip(x->right, y);
            return x;
        }
    }

    void unzip(Node* node, int key, Node** x, Node** y) {
        if (!node) {
            *x = nullptr;
            *y = nullptr;
            return;
        }

        if (node->key < key) {
            *x = node;
            unzip(node->right, key, &(node->right), y);
        } else {
            *y = node;
            unzip(node->left, key, x, &(node->left));
        }
    }

    Node* insert_helper(Node* curr, Node* newNode) {
        if (!curr) return newNode;

        if (newNode->rank < curr->rank) {
            comparison_count++;
            if (newNode->key < curr->key) {
                curr->left = insert_helper(curr->left, newNode);
            } else {
                curr->right = insert_helper(curr->right, newNode);
            }
            return curr;
        } else {
            Node *left_part = nullptr, *right_part = nullptr;
            unzip(curr, newNode->key, &left_part, &right_part);
            newNode->left = left_part;
            newNode->right = right_part;
            return newNode;
        }
    }

    Node* delete_helper(Node* curr, int key, bool& found) {
        if (!curr) {
            found = false;
            return nullptr;
        }

        if (key < curr->key) {
            curr->left = delete_helper(curr->left, key, found);
            return curr;
        } else if (key > curr->key) {
            curr->right = delete_helper(curr->right, key, found);
            return curr;
        } else {
            found = true;
            Node* left = curr->left;
            Node* right = curr->right;
            delete curr;
            return zip(left, right);
        }
    }

    /* Recursive Traversal/Utility Helpers */

    void clear(Node* node) {
        if (node) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

    void preorder(Node* node) const {
        if (node) {
            std::cout << node->key << "(r:" << node->rank << ") ";
            preorder(node->left);
            preorder(node->right);
        }
    }

    void print_structure(Node* node, std::string prefix, bool isLeft) const {
        if (!node) return;

        std::cout << prefix << (isLeft ? "|-- " : "`-- ") << node->key << "(r:" << node->rank << ")\n";

        std::string newPrefix = prefix + (isLeft ? "|   " : "    ");
        if (node->right) print_structure(node->right, newPrefix, true);
        if (node->left) print_structure(node->left, newPrefix, false);
    }

    int get_height(Node* node) const {
        if (!node) return 0;
        return 1 + std::max(get_height(node->left), get_height(node->right));
    }

    int count_nodes(Node* node) const {
        if (!node) return 0;
        return 1 + count_nodes(node->left) + count_nodes(node->right);
    }

    void path_lengths(Node* node, int depth, int& total, int& count) const {
        if (!node) return;
        count++;
        total += depth;
        path_lengths(node->left, depth + 1, total, count);
        path_lengths(node->right, depth + 1, total, count);
    }

public:
    ZipTree() : root(nullptr), comparison_count(0), change_count(0) {
        rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    ~ZipTree() { clear(root); }

    void insert(int key) {
        Node* newNode = new Node(key, generate_random_rank());
        root = insert_helper(root, newNode);
    }

    bool remove(int key) {
        bool found = false;
        root = delete_helper(root, key, found);
        return found;
    }

    bool search(int key) {
        Node* curr = root;
        while (curr) {
            comparison_count++;
            if (key == curr->key) return true;
            curr = (key < curr->key) ? curr->left : curr->right;
        }
        return false;
    }

    /* Stats and Display */

    void reset_counters() { comparison_count = 0; change_count = 0; }
    long long get_comparisons() const { return comparison_count; }

    void display_preorder() const { preorder(root); std::cout << std::endl; }

    void display_structure() const {
        if (!root) { std::cout << "(empty)\n"; return; }
        std::cout << root->key << "(r:" << root->rank << ")\n";
        if (root->right) print_structure(root->right, "", true);
        if (root->left) print_structure(root->left, "", false);
    }

    void print_stats() const {
        int n = count_nodes(root);
        int h = get_height(root);
        int total_depth = 0, node_count = 0;
        path_lengths(root, 0, total_depth, node_count);
        double avg_path = node_count > 0 ? (double)total_depth / node_count : 0;
        double theoretical = (n > 0) ? std::log2(n) : 0;

        std::cout << "  Total nodes: " << n << "\n";
        std::cout << "  Tree height: " << h << "\n";
        std::cout << "  Avg path length: " << std::fixed << std::setprecision(2) << avg_path << "\n";
        std::cout << "  Balance factor: " << (theoretical > 0 ? h / theoretical : 0) << "\n";
    }

    int size() const { return count_nodes(root); }
    int height() const { return get_height(root); }
};

  // BENCHMARKING & MAIN

void run_comprehensive_benchmark() {
    std::cout << "\n--- COMPREHENSIVE BENCHMARK ---\n";
    int sizes[] = {1000, 5000, 10000, 20000};
    
    std::cout << std::left << std::setw(10) << "Size" << " | " 
              << std::setw(12) << "Insert(ms)" << " | " 
              << std::setw(12) << "Search(ms)" << " | " 
              << std::setw(10) << "Height" << "\n";
    std::cout << std::string(55, '-') << "\n";

    for (int n : sizes) {
        ZipTree tree;
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 1; i <= n; i++) tree.insert(i);
        auto end = std::chrono::high_resolution_clock::now();
        double insert_time = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        for (int i = 1; i <= n; i++) tree.search(i);
        end = std::chrono::high_resolution_clock::now();
        double search_time = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << std::left << std::setw(10) << n << " | "
                  << std::setw(12) << insert_time << " | "
                  << std::setw(12) << search_time << " | "
                  << std::setw(10) << tree.height() << "\n";
    }
}

int main() {
    ZipTree tree;
    int choice, key;

    while (true) {
        std::cout << "\n=== ZIP TREE (C++ VERSION) ===\n"
                  << "1. Insert  2. Delete  3. Search  4. Display  5. Stats  6. Benchmark  7. Exit\n"
                  << "Choice: ";
        if (!(std::cin >> choice)) break;

        switch (choice) {
            case 1:
                std::cout << "Key: "; std::cin >> key;
                tree.insert(key);
                break;
            case 2:
                std::cout << "Key: "; std::cin >> key;
                if (tree.remove(key)) std::cout << "Deleted.\n";
                else std::cout << "Not found.\n";
                break;
            case 3:
                std::cout << "Key: "; std::cin >> key;
                tree.reset_counters();
                if (tree.search(key)) std::cout << "Found! Comparisons: " << tree.get_comparisons() << "\n";
                else std::cout << "Not found.\n";
                break;
            case 4:
                tree.display_structure();
                break;
            case 5:
                tree.print_stats();
                break;
            case 6:
                run_comprehensive_benchmark();
                break;
            case 7:
                return 0;
            default:
                std::cout << "Invalid.\n";
        }
    }
    return 0;
}