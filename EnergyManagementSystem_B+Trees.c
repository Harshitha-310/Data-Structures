#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#define ORDER 4 
#define TRANSACTION_FILE "transactions_B+Trees.txt"
#define SELLER_PRICES_FILE "sellers_prices_B+Trees.txt"
#define MAX_DATE_LENGTH 11  

/* ============== TABLE FORMATTING CODE ============== */
#define MAX_TABLE_COLS 10
#define MAX_TABLE_ROWS 1000
#define MAX_COL_WIDTH 30

typedef struct {
    char* columns[MAX_TABLE_COLS];
    int col_widths[MAX_TABLE_COLS];
    int num_cols;
    char* rows[MAX_TABLE_ROWS][MAX_TABLE_COLS];
    int num_rows;
} Table;

void init_table(Table* table) {
    table->num_cols = 0;
    table->num_rows = 0;
    for (int i = 0; i < MAX_TABLE_COLS; i++) {
        table->col_widths[i] = 0;
        table->columns[i] = NULL;
    }
}

void free_table(Table* table) {
    for (int i = 0; i < table->num_cols; i++) {
        if (table->columns[i]) free(table->columns[i]);
    }
    for (int i = 0; i < table->num_rows; i++) {
        for (int j = 0; j < table->num_cols; j++) {
            if (table->rows[i][j]) free(table->rows[i][j]);
        }
    }
}

void add_table_column(Table* table, const char* col_name) {
    if (table->num_cols >= MAX_TABLE_COLS) return;
    table->columns[table->num_cols] = strdup(col_name);
    table->col_widths[table->num_cols] = strlen(col_name);
    table->num_cols++;
}

void add_table_row(Table* table, ...) {
    if (table->num_rows >= MAX_TABLE_ROWS) return;
    
    va_list args;
    va_start(args, table);
    
    for (int i = 0; i < table->num_cols; i++) {
        char* val = va_arg(args, char*);
        table->rows[table->num_rows][i] = strdup(val ? val : "");
        int len = strlen(val);
        if (len > table->col_widths[i]) {
            table->col_widths[i] = len > MAX_COL_WIDTH ? MAX_COL_WIDTH : len;
        }
    }
    
    va_end(args);
    table->num_rows++;
}

void print_horizontal_border(Table* table) {
    putchar('+');
    for (int i = 0; i < table->num_cols; i++) {
        for (int j = 0; j < table->col_widths[i] + 2; j++) {
            putchar('-');
        }
        putchar('+');
    }
    putchar('\n');
}

void print_table(Table* table) {
    if (table->num_cols == 0 || table->num_rows == 0) return;
    
    // Print header
    print_horizontal_border(table);
    printf("|");
    for (int i = 0; i < table->num_cols; i++) {
        printf(" %-*s |", table->col_widths[i], table->columns[i]);
    }
    printf("\n");
    print_horizontal_border(table);
    
    // Print rows
    for (int i = 0; i < table->num_rows; i++) {
        printf("|");
        for (int j = 0; j < table->num_cols; j++) {
            printf(" %-*s |", table->col_widths[j], table->rows[i][j]);
        }
        printf("\n");
    }
    print_horizontal_border(table);
}

typedef struct Transaction {
    int transactionID;
    int buyerID;
    int sellerID;
    double energyAmount;
    double pricePerKwh;
    double totalPrice;
    char timestamp[30];
    struct Transaction* next;
} Transaction;

typedef struct BPTreeNode {
    int isLeaf;  
    int numKeys;  
    int keys[ORDER - 1];  
    struct BPTreeNode* children[ORDER]; 
    struct BPTreeNode* next;  
    Transaction* records[ORDER - 1];  
} BPTreeNode;

typedef struct RegularBuyer {
    int buyerID;
    struct RegularBuyer* next;
} RegularBuyer;

typedef struct Seller {
    int sellerID;
    double rateBelow300; 
    double rateAbove300;  
    int numTransactions;
    double totalRevenue;  
    RegularBuyer* regularBuyers;
    BPTreeNode* transactionTree; 
    struct Seller* next;
} Seller;

typedef struct Buyer {
    int buyerID;
    double totalEnergyPurchased;
    int numTransactions;
    BPTreeNode* transactionTree; 
    struct Buyer* next;
} Buyer;

typedef struct {
    int sellerID;
    int buyerID;
    int transactionCount;
} SellerBuyerPair;

typedef struct {
    Transaction** transactions;
    int count;
    int capacity;
} TransactionArray;

BPTreeNode* globalTransactionTree = NULL;
Seller* seller_head = NULL;
Buyer* buyer_head = NULL;
int nextTransactionID = 1;

Transaction* createTransaction(int transactionID, int buyerID, int sellerID, double energyAmount, double pricePerKwh, char* timestamp);
BPTreeNode* createBPTreeNode(int isLeaf);
Seller* findOrCreateSeller(int sellerID);
Buyer* findOrCreateBuyer(int buyerID);
void insertTransaction(Transaction* t);
void insertTransactionIntoBPTree(BPTreeNode** root, Transaction* t);
void insertInternalNode(BPTreeNode** root, int key, BPTreeNode* rightChild, BPTreeNode* parent);
void splitLeafNode(BPTreeNode** root, BPTreeNode* node, BPTreeNode* parent);
void splitInternalNode(BPTreeNode** root, BPTreeNode* node, BPTreeNode* parent);
int findTransactionInBPTree(BPTreeNode* root, int transactionID);
void displayTransactionsFromTree(BPTreeNode* leaf);
void traverseAndFilterTransactions(BPTreeNode* node, int id, int isSeller, int* found);
void freeTransactions();
void loadDataFromFile();
void loadSellerPrices();
void saveSellerPrices();
void findTransactionsByTimeRange(char* startDate, char* endDate);
void calculateTotalRevenueBySellerID(int sellerID);
void calculateTotalRevenueForAllSellers();
void findTransactionsByEnergyRange(double minEnergy, double maxEnergy);
int compareTransactionsByEnergy(const void* a, const void* b);
int isDateInRange(const char* date, const char* startDate, const char* endDate);
int countTransactionsInTree(BPTreeNode *root);
void sortBuyersByEnergyBought();
void sortSellerBuyerPairsByTransactions();
void addRegularBuyer(Seller* seller, Buyer* buyer);
void deleteTransaction(int transactionID);
void deleteTransactionFromBPTree(BPTreeNode** root, int transactionID);
void borrowFromNext(BPTreeNode* node, int idx);
void borrowFromPrev(BPTreeNode* node, int idx);
void mergeNodes(BPTreeNode** root, BPTreeNode* node, int idx);
BPTreeNode* findParent(BPTreeNode* root, BPTreeNode* child);
void removeFromLeaf(BPTreeNode* node, int idx);
void removeFromNonLeaf(BPTreeNode** root, BPTreeNode* node, int idx);
void deleteTransactionFile(int transactionID);
void insertTransactionIntoEntityTree(BPTreeNode** entityTree, Transaction* t);

Transaction* createTransaction(int transactionID, int buyerID, int sellerID, double energyAmount, double pricePerKwh, char* timestamp) {
    Transaction* t = (Transaction*)malloc(sizeof(Transaction));
    if (!t) {
        printf("Memory allocation failed for transaction.\n");
        exit(1);
    }
    t->transactionID = transactionID;
    t->buyerID = buyerID;
    t->sellerID = sellerID;
    t->energyAmount = energyAmount;
    t->pricePerKwh = pricePerKwh;
    t->totalPrice = energyAmount * pricePerKwh;
    strncpy(t->timestamp, timestamp, sizeof(t->timestamp) - 1);
    t->timestamp[sizeof(t->timestamp) - 1] = '\0';
    t->next = NULL;
    if (transactionID >= nextTransactionID) {
        nextTransactionID = transactionID + 1;
    }
    return t;
}

BPTreeNode* createBPTreeNode(int isLeaf) {
    BPTreeNode* newNode = (BPTreeNode*)malloc(sizeof(BPTreeNode));
    if (!newNode) {
        printf("Memory allocation failed for B+ tree node.\n");
        exit(1);
    }
    newNode->isLeaf = isLeaf;
    newNode->numKeys = 0;
    newNode->next = NULL;
    for (int i = 0; i < ORDER - 1; i++) {
        newNode->keys[i] = 0;
        newNode->records[i] = NULL;
    }
    for (int i = 0; i < ORDER; i++) {
        newNode->children[i] = NULL;
    }
    return newNode;
}

int findTransactionInBPTree(BPTreeNode* root, int transactionID) {
    if (!root) return 0;
    BPTreeNode* cursor = root;
    while (!cursor->isLeaf) {
        int i;
        for (i = 0; i < cursor->numKeys; i++) {
            if (transactionID < cursor->keys[i]) {
                break;
            }
        }
        cursor = cursor->children[i];
    }
    for (int i = 0; i < cursor->numKeys; i++) {
        if (cursor->keys[i] == transactionID) {
            return 1;  
        }
    }
    return 0; 
}

int loading_mode = 0;

Seller* findOrCreateSeller(int sellerID) {
    Seller* current = seller_head;
    while (current) {
        if (current->sellerID == sellerID) {
            return current;
        }
        current = current->next;
    }
    
    double rateBelow300 = 0.0, rateAbove300 = 0.0;
    if (!loading_mode) {
        printf("New Seller detected (ID: %d). Please enter the price for energy:\n", sellerID);
        printf("Price per kWh for energy below 300 kWh: ");
        scanf("%lf", &rateBelow300);
        printf("Price per kWh for energy above 300 kWh: ");
        scanf("%lf", &rateAbove300);
    }
    
    Seller* newSeller = (Seller*)malloc(sizeof(Seller));
    if (!newSeller) {
        printf("Memory allocation failed for seller.\n");
        exit(1);
    }
    
    newSeller->sellerID = sellerID;
    newSeller->rateBelow300 = rateBelow300;
    newSeller->rateAbove300 = rateAbove300;
    newSeller->numTransactions = 0;
    newSeller->totalRevenue = 0.0;
    newSeller->regularBuyers = NULL;
    newSeller->transactionTree = NULL;
    newSeller->next = seller_head;
    seller_head = newSeller;
    
    if (!loading_mode) {
        saveSellerPrices();
    }
    
    return newSeller;
}

Buyer* findOrCreateBuyer(int buyerID) {
    Buyer* current = buyer_head;
    while (current) {
        if (current->buyerID == buyerID) {
            return current;
        }
        current = current->next;
    }
    
    Buyer* newBuyer = (Buyer*)malloc(sizeof(Buyer));
    if (!newBuyer) {
        printf("Memory allocation failed for buyer.\n");
        exit(1);
    }
    
    newBuyer->buyerID = buyerID;
    newBuyer->totalEnergyPurchased = 0;
    newBuyer->numTransactions = 0;
    
    // Initialize the buyer's B+ tree for transactions
    newBuyer->transactionTree = NULL;
    
    newBuyer->next = buyer_head;
    buyer_head = newBuyer;
    
    return newBuyer;
}

void addRegularBuyer(Seller* seller, Buyer* buyer) {
    if (buyer->numTransactions > 5) {
        RegularBuyer* current = seller->regularBuyers;
        while (current) {
            if (current->buyerID == buyer->buyerID) {
                return; 
            }
            current = current->next;
        }
        RegularBuyer* newRegularBuyer = (RegularBuyer*)malloc(sizeof(RegularBuyer));
        if (!newRegularBuyer) {
            printf("Memory allocation failed for regular buyer.\n");
            exit(1);
        }
        newRegularBuyer->buyerID = buyer->buyerID;
        newRegularBuyer->next = seller->regularBuyers;
        seller->regularBuyers = newRegularBuyer;
    }
}

void loadSellerPrices() {
    FILE *file = fopen(SELLER_PRICES_FILE, "r");
    if (!file) {
        printf("No price data found, starting fresh.\n");
        return;
    }
    int sellerID;
    double rateBelow300, rateAbove300;
    while (fscanf(file, "%d %lf %lf", &sellerID, &rateBelow300, &rateAbove300) == 3) {
        Seller* current = seller_head;
        int found = 0;
        while (current) {
            if (current->sellerID == sellerID) {
                current->rateBelow300 = rateBelow300;
                current->rateAbove300 = rateAbove300;
                found = 1;
                break;
            }
            current = current->next;
        }
        if (!found) {
            Seller* newSeller = (Seller*)malloc(sizeof(Seller));
            if (!newSeller) {
                printf("Memory allocation failed for seller.\n");
                exit(1);
            }
            newSeller->sellerID = sellerID;
            newSeller->rateBelow300 = rateBelow300;
            newSeller->rateAbove300 = rateAbove300;
            newSeller->numTransactions = 0;
            newSeller->totalRevenue = 0.0;  
            newSeller->regularBuyers = NULL; 
            newSeller->transactionTree = NULL; 
            newSeller->next = seller_head;
            seller_head = newSeller;
            printf("Loaded seller ID: %d with rates %.2f/%.2f\n", sellerID, rateBelow300, rateAbove300);
        }
    }
    fclose(file);
}

void saveSellerPrices() {
    FILE *file = fopen(SELLER_PRICES_FILE, "w");
    if (!file) {
        printf("Error opening file for saving prices.\n");
        return;
    }
    Seller* current = seller_head;
    while (current) {
        fprintf(file, "%d %.2lf %.2lf\n", current->sellerID, current->rateBelow300, current->rateAbove300);
        current = current->next;
    }
    fclose(file);
}

void splitLeafNode(BPTreeNode** root, BPTreeNode* node, BPTreeNode* parent) {
    int mid = (ORDER - 1) / 2;
    BPTreeNode* newNode = createBPTreeNode(1); 
    for (int i = mid; i < ORDER - 1; i++) {
        newNode->keys[i - mid] = node->keys[i];
        newNode->records[i - mid] = node->records[i];
        newNode->numKeys++;
        node->keys[i] = 0;
        node->records[i] = NULL;
    }
    node->numKeys = mid;
    newNode->next = node->next;
    node->next = newNode;
    int promoteKey = newNode->keys[0];
    if (parent == NULL) {
        BPTreeNode* newRoot = createBPTreeNode(0);
        newRoot->keys[0] = promoteKey;
        newRoot->children[0] = node;
        newRoot->children[1] = newNode;
        newRoot->numKeys = 1;
        *root = newRoot;
    } else {
        insertInternalNode(root, promoteKey, newNode, parent);
    }
}

void splitInternalNode(BPTreeNode** root, BPTreeNode* node, BPTreeNode* parent) {
    int mid = (ORDER - 1) / 2;
    BPTreeNode* newNode = createBPTreeNode(0);
    int promoteKey = node->keys[mid];
    for (int i = mid + 1; i < ORDER - 1; i++) {
        newNode->keys[i - (mid + 1)] = node->keys[i];
        newNode->numKeys++;
        node->keys[i] = 0;
    }
    for (int i = mid + 1; i < ORDER; i++) {
        newNode->children[i - (mid + 1)] = node->children[i];
        node->children[i] = NULL;
    }
    node->numKeys = mid;
    if (parent == NULL) {
        BPTreeNode* newRoot = createBPTreeNode(0);
        newRoot->keys[0] = promoteKey;
        newRoot->children[0] = node;
        newRoot->children[1] = newNode;
        newRoot->numKeys = 1;
        *root = newRoot;
    } else {
        insertInternalNode(root, promoteKey, newNode, parent);
    }
}

void insertInternalNode(BPTreeNode** root, int key, BPTreeNode* rightChild, BPTreeNode* node) {
    int pos;
    for (pos = 0; pos < node->numKeys; pos++) {
        if (key < node->keys[pos]) {
            break;
        }
    }
    for (int i = node->numKeys; i > pos; i--) {
        node->keys[i] = node->keys[i-1];
    }
    for (int i = node->numKeys + 1; i > pos + 1; i--) {
        node->children[i] = node->children[i-1];
    }
    node->keys[pos] = key;
    node->children[pos+1] = rightChild;
    node->numKeys++;
    if (node->numKeys == ORDER - 1) {
        if (node == *root) {
            splitInternalNode(root, node, NULL);
            return;
        }
        BPTreeNode* parent = NULL;
        BPTreeNode* cursor = *root;
        BPTreeNode* stack[100];
        int stackTop = -1;
        stackTop++;
        stack[stackTop] = cursor;
        while (stackTop >= 0) {
            cursor = stack[stackTop];
            stackTop--;
            for (int i = 0; i <= cursor->numKeys; i++) {
                if (cursor->children[i] == node) {
                    parent = cursor;
                    stackTop = -1;
                    break;
                }
                if (!cursor->isLeaf && cursor->children[i] != NULL) {
                    stackTop++;
                    stack[stackTop] = cursor->children[i];
                }
            }
        }
        if (parent != NULL) {
            splitInternalNode(root, node, parent);
        } else {
            printf("Warning: Parent not found for internal node during split.\n");
            splitInternalNode(root, node, NULL);
        }
    }
}

void insertTransactionIntoBPTree(BPTreeNode** root, Transaction* t) {
    if (!*root) {
        *root = createBPTreeNode(1);
        (*root)->keys[0] = t->transactionID;
        (*root)->records[0] = t;
        (*root)->numKeys = 1;
        return;
    }
    BPTreeNode* cursor = *root;
    BPTreeNode* parents[100]; 
    int parentIndex = -1;
    while (!cursor->isLeaf) {
        parentIndex++;
        parents[parentIndex] = cursor;
        
        int i;
        for (i = 0; i < cursor->numKeys; i++) {
            if (t->transactionID < cursor->keys[i]) {
                break;
            }
        }
        cursor = cursor->children[i];
    }
    int pos;
    for (pos = 0; pos < cursor->numKeys; pos++) {
        if (t->transactionID < cursor->keys[pos]) {
            break;
        }
    }
    for (int i = cursor->numKeys; i > pos; i--) {
        cursor->keys[i] = cursor->keys[i-1];
        cursor->records[i] = cursor->records[i-1];
    }
    cursor->keys[pos] = t->transactionID;
    cursor->records[pos] = t;
    cursor->numKeys++;
    if (cursor->numKeys == ORDER - 1) {
        if (parentIndex == -1) {
            splitLeafNode(root, cursor, NULL);
        } else {
            splitLeafNode(root, cursor, parents[parentIndex]);
        }
    }
}

void insertTransaction(Transaction* t) {
    if (findTransactionInBPTree(globalTransactionTree, t->transactionID)) {
        printf("Error: Transaction with ID %d already exists. Cannot create duplicate transactions.\n", t->transactionID);
        free(t);
        return;
    }
    
    Seller* seller = findOrCreateSeller(t->sellerID);
    Buyer* buyer = findOrCreateBuyer(t->buyerID);
    
    t->pricePerKwh = (t->energyAmount <= 300) ? seller->rateBelow300 : seller->rateAbove300;
    t->totalPrice = t->energyAmount * t->pricePerKwh;
    // Insert into global transaction tree
    insertTransactionIntoBPTree(&globalTransactionTree, t);
    // Also insert references to the same transaction into seller's and buyer's trees
    // Note: We're not creating new transaction objects, just pointing to the same one
    insertTransactionIntoEntityTree(&seller->transactionTree, t);
    insertTransactionIntoEntityTree(&buyer->transactionTree, t);
    
    seller->numTransactions++;
    seller->totalRevenue += t->totalPrice;
    buyer->numTransactions++;
    buyer->totalEnergyPurchased += t->energyAmount;
    
    addRegularBuyer(seller, buyer);
    
    FILE *file = fopen(TRANSACTION_FILE, "a");
    if (!file) {
        printf("Error opening transaction file for appending.\n");
        return;
    }
    
    fprintf(file, "%d,%d,%d,%.2f,%.2f,%.2f,%s\n", 
            t->transactionID, t->buyerID, t->sellerID, 
            t->energyAmount, t->pricePerKwh, t->totalPrice, 
            t->timestamp);
    
    fclose(file);
    printf("Transaction added successfully! ID: %d\n", t->transactionID);
}

void insertTransactionIntoEntityTree(BPTreeNode** entityTree, Transaction* t) {
    insertTransactionIntoBPTree(entityTree, t);
}

int isDateInRange(const char* date, const char* startDate, const char* endDate) {
    struct tm tm_date = {0}, tm_start = {0}, tm_end = {0};
    
    if (sscanf(date, "%d-%d-%d", &tm_date.tm_year, &tm_date.tm_mon, &tm_date.tm_mday) != 3) {
        return 0;
    }
    if (sscanf(startDate, "%d-%d-%d", &tm_start.tm_year, &tm_start.tm_mon, &tm_start.tm_mday) != 3) {
        return 0;
    }
    if (sscanf(endDate, "%d-%d-%d", &tm_end.tm_year, &tm_end.tm_mon, &tm_end.tm_mday) != 3) {
        return 0;
    }
    tm_date.tm_year -= 1900;
    tm_start.tm_year -= 1900;
    tm_end.tm_year -= 1900;
    tm_date.tm_mon -= 1;
    tm_start.tm_mon -= 1;
    tm_end.tm_mon -= 1;
    time_t t_date = mktime(&tm_date);
    time_t t_start = mktime(&tm_start);
    time_t t_end = mktime(&tm_end);
    return (t_date >= t_start && t_date <= t_end);
}

void findTransactionsByTimeRange(char* startDate, char* endDate) {
    if (!globalTransactionTree) {
        printf("No transactions available.\n");
        return;
    }

    printf("\n===== Transactions from %s to %s =====\n", startDate, endDate);

    Table table;
    init_table(&table);
    add_table_column(&table, "Transaction ID");
    add_table_column(&table, "Buyer ID");
    add_table_column(&table, "Seller ID");
    add_table_column(&table, "Energy (kWh)");
    add_table_column(&table, "Price/kWh");
    add_table_column(&table, "Total Price");
    add_table_column(&table, "Timestamp");

    BPTreeNode* cursor = globalTransactionTree;
    while (!cursor->isLeaf) {
        cursor = cursor->children[0];
    }

    int found = 0;
    while (cursor) {
        for (int i = 0; i < cursor->numKeys; i++) {
            Transaction* t = cursor->records[i];
            
            if (isDateInRange(t->timestamp, startDate, endDate)) {
                char id[20], buyer[20], seller[20], energy[20], price[20], total[20];
                snprintf(id, sizeof(id), "%d", t->transactionID);
                snprintf(buyer, sizeof(buyer), "%d", t->buyerID);
                snprintf(seller, sizeof(seller), "%d", t->sellerID);
                snprintf(energy, sizeof(energy), "%.2f", t->energyAmount);
                snprintf(price, sizeof(price), "%.2f", t->pricePerKwh);
                snprintf(total, sizeof(total), "%.2f", t->totalPrice);
                
                add_table_row(&table, id, buyer, seller, energy, price, total, t->timestamp);
                found++;
            }
        }
        cursor = cursor->next;
    }

    if (found) {
        print_table(&table);
    } else {
        printf("No transactions found in the specified time period.\n");
    }

    free_table(&table);
}

void calculateTotalRevenueBySellerID(int sellerID) {
    Seller* seller = seller_head;
    int found = 0;
    
    while (seller) {
        if (seller->sellerID == sellerID) {
            // Initialize table
            Table table;
            init_table(&table);
            add_table_column(&table, "Metric");
            add_table_column(&table, "Value");
            
            // Convert all values to strings before adding to table
            char sid[20], totalRev[50], totalTrans[50], avgRev[50];
            snprintf(sid, sizeof(sid), "%d", sellerID);
            snprintf(totalRev, sizeof(totalRev), "$%.2f", seller->totalRevenue);
            snprintf(totalTrans, sizeof(totalTrans), "%d", seller->numTransactions);
            
            // Add rows to table
            add_table_row(&table, "Seller ID", sid);
            add_table_row(&table, "Total Revenue", totalRev);
            add_table_row(&table, "Total Transactions", totalTrans);
            
            if (seller->numTransactions > 0) {
                snprintf(avgRev, sizeof(avgRev), "$%.2f", 
                        seller->totalRevenue / seller->numTransactions);
                add_table_row(&table, "Avg Revenue/Transaction", avgRev);
            }
            
            // Print table
            printf("\n===== Revenue Summary for Seller ID %d =====\n", sellerID);
            print_table(&table);
            free_table(&table);
            
            found = 1;
            break;
        }
        seller = seller->next;
    }
    
    if (!found) {
        printf("Seller ID %d not found.\n", sellerID);
    }
}

void calculateTotalRevenueForAllSellers() {
    Seller* seller = seller_head;
    if (!seller) {
        printf("No sellers found in the system.\n");
        return;
    }

    Table table;
    init_table(&table);
    add_table_column(&table, "Seller ID");
    add_table_column(&table, "Total Revenue");
    add_table_column(&table, "Transactions");
    add_table_column(&table, "Avg Revenue");

    double grandTotal = 0.0;
    int totalTransactions = 0;
    
    while (seller) {
        char id[20], revenue[20], trans[20], avg[20];
        snprintf(id, sizeof(id), "%d", seller->sellerID);
        snprintf(revenue, sizeof(revenue), "$%.2f", seller->totalRevenue);
        snprintf(trans, sizeof(trans), "%d", seller->numTransactions);
        snprintf(avg, sizeof(avg), "$%.2f", 
            seller->numTransactions > 0 ? seller->totalRevenue / seller->numTransactions : 0.0);
        
        add_table_row(&table, id, revenue, trans, avg);
        
        grandTotal += seller->totalRevenue;
        totalTransactions += seller->numTransactions;
        seller = seller->next;
    }

    // Add summary row
    char grand[20], totalTrans[20], grandAvg[20];
    snprintf(grand, sizeof(grand), "$%.2f", grandTotal);
    snprintf(totalTrans, sizeof(totalTrans), "%d", totalTransactions);
    snprintf(grandAvg, sizeof(grandAvg), "$%.2f",
        totalTransactions > 0 ? grandTotal / totalTransactions : 0.0);
    
    add_table_row(&table, "TOTAL", grand, totalTrans, grandAvg);

    printf("\n===== Revenue Summary for All Sellers =====\n");
    print_table(&table);
    free_table(&table);
}

void merge(Transaction** arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    Transaction** L = malloc(n1 * sizeof(Transaction*));
    Transaction** R = malloc(n2 * sizeof(Transaction*));
    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int i = 0; i < n2; i++) R[i] = arr[m + 1 + i];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i]->energyAmount <= R[j]->energyAmount)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    free(L);
    free(R);
}

void mergeSort(Transaction** arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

void findTransactionsByEnergyRange(double minEnergy, double maxEnergy) {
    if (!globalTransactionTree) {
        printf("No transactions available.\n");
        return;
    }

    // Initialize table
    Table table;
    init_table(&table);
    add_table_column(&table, "Transaction ID");
    add_table_column(&table, "Buyer ID");
    add_table_column(&table, "Seller ID");
    add_table_column(&table, "Energy (kWh)");
    add_table_column(&table, "Price/kWh");
    add_table_column(&table, "Total Price");
    add_table_column(&table, "Timestamp");

    printf("\n===== Transactions with Energy Amount between %.2f kWh and %.2f kWh (Ascending Order) =====\n", 
           minEnergy, maxEnergy);

    TransactionArray transArray;
    transArray.capacity = 100;  
    transArray.count = 0;
    transArray.transactions = (Transaction**)malloc(transArray.capacity * sizeof(Transaction*));
    if (!transArray.transactions) {
        printf("Memory allocation failed.\n");
        return;
    }

    // Collect matching transactions
    BPTreeNode* cursor = globalTransactionTree;
    while (!cursor->isLeaf) {
        cursor = cursor->children[0];
    }

    while (cursor) {
        for (int i = 0; i < cursor->numKeys; i++) {
            Transaction* t = cursor->records[i];
            if (t->energyAmount >= minEnergy && t->energyAmount <= maxEnergy) {
                if (transArray.count >= transArray.capacity) {
                    transArray.capacity *= 2;
                    Transaction** newArray = (Transaction**)realloc(transArray.transactions, 
                                                                   transArray.capacity * sizeof(Transaction*));
                    if (!newArray) {
                        printf("Memory reallocation failed.\n");
                        free(transArray.transactions);
                        free_table(&table);
                        return;
                    }
                    transArray.transactions = newArray;
                }
                transArray.transactions[transArray.count++] = t;
            }
        }
        cursor = cursor->next;
    }

    // Sort transactions by energy amount
    mergeSort(transArray.transactions, 0, transArray.count - 1);

    // Add transactions to table
    for (int i = 0; i < transArray.count; i++) {
        Transaction* t = transArray.transactions[i];
        char id[20], buyer[20], seller[20], energy[20], price[20], total[20];
        snprintf(id, sizeof(id), "%d", t->transactionID);
        snprintf(buyer, sizeof(buyer), "%d", t->buyerID);
        snprintf(seller, sizeof(seller), "%d", t->sellerID);
        snprintf(energy, sizeof(energy), "%.2f", t->energyAmount);
        snprintf(price, sizeof(price), "%.2f", t->pricePerKwh);
        snprintf(total, sizeof(total), "%.2f", t->totalPrice);
        
        add_table_row(&table, id, buyer, seller, energy, price, total, t->timestamp);
    }

    // Display results
    if (transArray.count > 0) {
        print_table(&table);
    } else {
        printf("No transactions found in the specified energy range.\n");
    }

    // Clean up
    free(transArray.transactions);
    free_table(&table);
}

int countTransactionsInTree(BPTreeNode* root) {
    if (!root) return 0;
    if (root->isLeaf) {
        return root->numKeys;
    }
    BPTreeNode* cursor = root;
    while (!cursor->isLeaf) {
        cursor = cursor->children[0];
    }
    int count = 0;
    while (cursor) {
        count += cursor->numKeys;
        cursor = cursor->next;
    }
    return count;
}

int compareBuyersByEnergyDesc(Buyer* a, Buyer* b) {
    if (a->totalEnergyPurchased < b->totalEnergyPurchased) return 1;
    if (a->totalEnergyPurchased > b->totalEnergyPurchased) return -1;
    return 0;
}

void merge_buyers(Buyer** arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    Buyer** L = (Buyer**)malloc(n1 * sizeof(Buyer*));
    Buyer** R = (Buyer**)malloc(n2 * sizeof(Buyer*));
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (compareBuyersByEnergyDesc(L[i], R[j]) <= 0)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    free(L);
    free(R);
}

void mergeSortBuyers(Buyer** arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortBuyers(arr, left, mid);
        mergeSortBuyers(arr, mid + 1, right);
        merge_buyers(arr, left, mid, right);
    }
}

void sortBuyersByEnergyBought() {
    int buyerCount = 0;
    Buyer* current = buyer_head;
    while (current) {
        buyerCount++;
        current = current->next;
    }

    if (buyerCount == 0) {
        printf("No buyers found in the system.\n");
        return;
    }

    Table table;
    init_table(&table);
    add_table_column(&table, "Buyer ID");
    add_table_column(&table, "Energy Purchased");
    add_table_column(&table, "Transactions");

    Buyer** buyerArray = (Buyer**)malloc(buyerCount * sizeof(Buyer*));
    current = buyer_head;
    for (int i = 0; current; i++) {
        buyerArray[i] = current;
        current = current->next;
    }

    // Sort the array (using your existing merge sort functions)
    mergeSortBuyers(buyerArray, 0, buyerCount - 1);

    double totalEnergy = 0.0;
    int totalTransactions = 0;
    for (int i = 0; i < buyerCount; i++) {
        char id[20], energy[20], trans[20];
        snprintf(id, sizeof(id), "%d", buyerArray[i]->buyerID);
        snprintf(energy, sizeof(energy), "%.2f kWh", buyerArray[i]->totalEnergyPurchased);
        snprintf(trans, sizeof(trans), "%d", buyerArray[i]->numTransactions);
        
        add_table_row(&table, id, energy, trans);
        
        totalEnergy += buyerArray[i]->totalEnergyPurchased;
        totalTransactions += buyerArray[i]->numTransactions;
    }

    // Add summary row
    char totalE[20], totalT[20];
    snprintf(totalE, sizeof(totalE), "%.2f kWh", totalEnergy);
    snprintf(totalT, sizeof(totalT), "%d", totalTransactions);
    add_table_row(&table, "TOTAL", totalE, totalT);

    printf("\n===== Buyers Sorted by Energy Purchased =====\n");
    print_table(&table);
    free_table(&table);
    free(buyerArray);
}

int compareSellerBuyerPairs(const SellerBuyerPair* a, const SellerBuyerPair* b) {
    return b->transactionCount - a->transactionCount;  
}

void mergeSellerBuyerPairs(SellerBuyerPair* arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    SellerBuyerPair* L = (SellerBuyerPair*)malloc(n1 * sizeof(SellerBuyerPair));
    SellerBuyerPair* R = (SellerBuyerPair*)malloc(n2 * sizeof(SellerBuyerPair));
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (compareSellerBuyerPairs(&L[i], &R[j]) <= 0)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    free(L);
    free(R);
}

void mergeSortSellerBuyerPairs(SellerBuyerPair* arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortSellerBuyerPairs(arr, left, mid);
        mergeSortSellerBuyerPairs(arr, mid + 1, right);
        mergeSellerBuyerPairs(arr, left, mid, right);
    }
}

void sortSellerBuyerPairsByTransactions() {
    if (!globalTransactionTree) {
        printf("No transactions available.\n");
        return;
    }

    const int MAX_PAIRS = 1000;
    SellerBuyerPair* pairs = (SellerBuyerPair*)malloc(MAX_PAIRS * sizeof(SellerBuyerPair));
    if (!pairs) {
        printf("Memory allocation failed.\n");
        return;
    }

    // Initialize pairs array
    for (int i = 0; i < MAX_PAIRS; i++) {
        pairs[i].sellerID = -1;
        pairs[i].buyerID = -1;
        pairs[i].transactionCount = 0;
    }

    // Count transactions for each seller-buyer pair
    int pairCount = 0;
    BPTreeNode* cursor = globalTransactionTree;
    while (!cursor->isLeaf) {
        cursor = cursor->children[0];
    }

    while (cursor) {
        for (int i = 0; i < cursor->numKeys; i++) {
            Transaction* t = cursor->records[i];
            int pairIndex = -1;
            
            // Check if pair already exists
            for (int j = 0; j < pairCount; j++) {
                if (pairs[j].sellerID == t->sellerID && pairs[j].buyerID == t->buyerID) {
                    pairIndex = j;
                    break;
                }
            }

            if (pairIndex >= 0) {
                pairs[pairIndex].transactionCount++;
            } else if (pairCount < MAX_PAIRS) {
                pairs[pairCount].sellerID = t->sellerID;
                pairs[pairCount].buyerID = t->buyerID;
                pairs[pairCount].transactionCount = 1;
                pairCount++;
            } else {
                printf("Warning: Too many seller-buyer pairs, some may not be counted.\n");
            }
        }
        cursor = cursor->next;
    }

    // Sort pairs by transaction count
    mergeSortSellerBuyerPairs(pairs, 0, pairCount - 1);

    // Initialize table
    Table table;
    init_table(&table);
    add_table_column(&table, "Seller ID");
    add_table_column(&table, "Buyer ID");
    add_table_column(&table, "Transaction Count");

    // Add pairs to table
    int totalTransactions = 0;
    for (int i = 0; i < pairCount; i++) {
        char seller[20], buyer[20], count[20];
        snprintf(seller, sizeof(seller), "%d", pairs[i].sellerID);
        snprintf(buyer, sizeof(buyer), "%d", pairs[i].buyerID);
        snprintf(count, sizeof(count), "%d", pairs[i].transactionCount);
        
        add_table_row(&table, seller, buyer, count);
        totalTransactions += pairs[i].transactionCount;
    }

    // Print results
    printf("\n===== Seller-Buyer Pairs Sorted by Transaction Count =====\n");
    if (pairCount > 0) {
        print_table(&table);
        printf("\nSummary:\n");
        printf("Total Pairs: %d\n", pairCount);
        printf("Total Transactions: %d\n", totalTransactions);
    } else {
        printf("No seller-buyer pairs found.\n");
    }

    // Clean up
    free(pairs);
    free_table(&table);
}

void loadDataFromFile() {
    FILE *file = fopen(TRANSACTION_FILE, "r");
    if (!file) {
        printf("No existing transactions found. Starting fresh.\n");
        return;
    }
    
    char line[256];
    loading_mode = 1;
    int totalLoaded = 0;
    int duplicates = 0;
    
    while (fgets(line, sizeof(line), file)) {
        int transactionID, buyerID, sellerID;
        double energyAmount, pricePerKwh, totalPrice;
        char timestamp[30];
        
        if (sscanf(line, "%d,%d,%d,%lf,%lf,%lf,%[^\n]", 
                  &transactionID, &buyerID, &sellerID, 
                  &energyAmount, &pricePerKwh, &totalPrice, 
                  timestamp) == 7) {
            
            if (findTransactionInBPTree(globalTransactionTree, transactionID)) {
                printf("Warning: Duplicate transaction ID %d found in file. Skipping.\n", transactionID);
                duplicates++;
                continue;
            }
            
            Transaction* t = createTransaction(transactionID, buyerID, sellerID, 
                                             energyAmount, pricePerKwh, timestamp);
            t->totalPrice = totalPrice;
            
            Seller* seller = findOrCreateSeller(t->sellerID);
            Buyer* buyer = findOrCreateBuyer(t->buyerID);
            
            // Insert into global transaction tree
            insertTransactionIntoBPTree(&globalTransactionTree, t);
            
            // Also insert into seller's and buyer's trees
            insertTransactionIntoEntityTree(&seller->transactionTree, t);
            insertTransactionIntoEntityTree(&buyer->transactionTree, t);
            
            seller->numTransactions++;
            seller->totalRevenue += t->totalPrice;
            buyer->numTransactions++;
            buyer->totalEnergyPurchased += t->energyAmount;
            
            addRegularBuyer(seller, buyer);
            printf("Loaded transaction: ID %d\n", transactionID);
            totalLoaded++;
        } else {
            printf("Warning: Malformed transaction data in file: %s", line);
        }
    }
    
    loading_mode = 0;
    fclose(file);
    printf("Successfully loaded %d transactions. Skipped %d duplicates.\n", totalLoaded, duplicates);
    printf("Verifying B+ tree structure...\n");
    
    int treeCount = countTransactionsInTree(globalTransactionTree);
    printf("Count of transactions in B+ tree: %d\n", treeCount);
    
    if (treeCount != totalLoaded) {
        printf("WARNING: Mismatch between loaded transactions (%d) and tree count (%d)!\n", 
               totalLoaded, treeCount);
    }
}

Transaction* findTransactionById(BPTreeNode* root, int id) {
    if (!root) return NULL;
    BPTreeNode* cursor = root;
    while (!cursor->isLeaf) {
        int i;
        for (i = 0; i < cursor->numKeys; i++) {
            if (id < cursor->keys[i]) {
                break;
            }
        }
        cursor = cursor->children[i];
    }
    for (int i = 0; i < cursor->numKeys; i++) {
        if (cursor->keys[i] == id) {
            return cursor->records[i];
        }
    }
    return NULL;
}

void displayTransactionsFromTree(BPTreeNode* root) {
    if (!root) {
        printf("No transactions to display.\n");
        return;
    }
    
    Table table;
    init_table(&table);
    add_table_column(&table, "Transaction ID");
    add_table_column(&table, "Buyer ID");
    add_table_column(&table, "Seller ID");
    add_table_column(&table, "Energy (kWh)");
    add_table_column(&table, "Price/kWh");
    add_table_column(&table, "Total Price");
    add_table_column(&table, "Timestamp");

    BPTreeNode* cursor = root;
    while (!cursor->isLeaf) {
        cursor = cursor->children[0];
    }
    
    int count = 0;
    while (cursor) {
        for (int i = 0; i < cursor->numKeys; i++) {
            Transaction* t = cursor->records[i];
            if (t) {
                char id[20], buyer[20], seller[20], energy[20], price[20], total[20];
                snprintf(id, sizeof(id), "%d", t->transactionID);
                snprintf(buyer, sizeof(buyer), "%d", t->buyerID);
                snprintf(seller, sizeof(seller), "%d", t->sellerID);
                snprintf(energy, sizeof(energy), "%.2f", t->energyAmount);
                snprintf(price, sizeof(price), "%.2f", t->pricePerKwh);
                snprintf(total, sizeof(total), "%.2f", t->totalPrice);
                
                add_table_row(&table, id, buyer, seller, energy, price, total, t->timestamp);
                count++;
            }
        }
        cursor = cursor->next;
    }

    printf("\n===== All Transactions (%d) =====\n", count);
    print_table(&table);
    free_table(&table);
}

void freeBPTree(BPTreeNode* node) {
    if (!node) return;
    if (node->isLeaf) {
        for (int i = 0; i < node->numKeys; i++) {
            free(node->records[i]);
        }
    } else {
        for (int i = 0; i <= node->numKeys; i++) {
            freeBPTree(node->children[i]);
        }
    }
    free(node);
}

void freeBPTreeNodesOnly(BPTreeNode* node) {
    if (!node) return;
    if (!node->isLeaf) {
        for (int i = 0; i <= node->numKeys; i++) {
            freeBPTreeNodesOnly(node->children[i]);
        }
    }
    free(node);
}

void freeTransactions() {
    // Free global transaction tree
    freeBPTree(globalTransactionTree);
    // Free seller data
    Seller* s = seller_head;
    while (s) {
        Seller* temp = s;
        // Free seller's transaction tree if it exists
        if (s->transactionTree) {
            freeBPTreeNodesOnly(s->transactionTree);
        }
        // Free regular buyers list
        RegularBuyer* rb = s->regularBuyers;
        while (rb) {
            RegularBuyer* tempRb = rb;
            rb = rb->next;
            free(tempRb);
        }
        s = s->next;
        free(temp);
    }
    
    // Free buyer data
    Buyer* b = buyer_head;
    while (b) {
        Buyer* temp = b;
        // Free buyer's transaction tree if it exists
        if (b->transactionTree) {
            freeBPTreeNodesOnly(b->transactionTree);
        }
        b = b->next;
        free(temp);
    }
}

void createSetOfTransactionsForSeller(int sellerID) {
    printf("\n===== Transactions for Seller ID %d =====\n", sellerID);
    
    Seller* seller = NULL;
    Seller* current = seller_head;
    
    while (current) {
        if (current->sellerID == sellerID) {
            seller = current;
            break;
        }
        current = current->next;
    }
    
    if (!seller) {
        printf("Seller ID %d not found.\n", sellerID);
        return;
    }
    
    if (!seller->transactionTree) {
        printf("No transactions found for Seller ID %d.\n", sellerID);
        return;
    }
    
    Table table;
    init_table(&table);
    add_table_column(&table, "Transaction ID");
    add_table_column(&table, "Buyer ID");
    add_table_column(&table, "Seller ID");
    add_table_column(&table, "Energy (kWh)");
    add_table_column(&table, "Price/kWh");
    add_table_column(&table, "Total Price");
    add_table_column(&table, "Timestamp");
    
    BPTreeNode* cursor = seller->transactionTree;
    
    while (!cursor->isLeaf) {
        cursor = cursor->children[0];
    }
    
    int found = 0;
    while (cursor) {
        for (int i = 0; i < cursor->numKeys; i++) {
            Transaction* t = cursor->records[i];
            if (t) {
                char id[20], buyer[20], seller[20], energy[20], price[20], total[20];
                snprintf(id, sizeof(id), "%d", t->transactionID);
                snprintf(buyer, sizeof(buyer), "%d", t->buyerID);
                snprintf(seller, sizeof(seller), "%d", t->sellerID);
                snprintf(energy, sizeof(energy), "%.2f", t->energyAmount);
                snprintf(price, sizeof(price), "%.2f", t->pricePerKwh);
                snprintf(total, sizeof(total), "%.2f", t->totalPrice);
                
                add_table_row(&table, id, buyer, seller, energy, price, total, t->timestamp);
                found++;
            }
        }
        cursor = cursor->next;
    }
    
    if (found) {
        print_table(&table);
    } else {
        printf("No transactions found for Seller ID %d.\n", sellerID);
    }
    
    free_table(&table);
}

void createSetOfTransactionsForBuyer(int buyerID) {
    printf("\n===== Transactions for Buyer ID %d =====\n", buyerID);
    
    Buyer* buyer = NULL;
    Buyer* current = buyer_head;
    
    while (current) {
        if (current->buyerID == buyerID) {
            buyer = current;
            break;
        }
        current = current->next;
    }
    
    if (!buyer) {
        printf("Buyer ID %d not found.\n", buyerID);
        return;
    }
    
    if (!buyer->transactionTree) {
        printf("No transactions found for Buyer ID %d.\n", buyerID);
        return;
    }
    
    Table table;
    init_table(&table);
    add_table_column(&table, "Transaction ID");
    add_table_column(&table, "Buyer ID");
    add_table_column(&table, "Seller ID");
    add_table_column(&table, "Energy (kWh)");
    add_table_column(&table, "Price/kWh");
    add_table_column(&table, "Total Price");
    add_table_column(&table, "Timestamp");
    
    BPTreeNode* cursor = buyer->transactionTree;
    
    while (!cursor->isLeaf) {
        cursor = cursor->children[0];
    }
    
    int found = 0;
    while (cursor) {
        for (int i = 0; i < cursor->numKeys; i++) {
            Transaction* t = cursor->records[i];
            if (t) {
                char id[20], buyer[20], seller[20], energy[20], price[20], total[20];
                snprintf(id, sizeof(id), "%d", t->transactionID);
                snprintf(buyer, sizeof(buyer), "%d", t->buyerID);
                snprintf(seller, sizeof(seller), "%d", t->sellerID);
                snprintf(energy, sizeof(energy), "%.2f", t->energyAmount);
                snprintf(price, sizeof(price), "%.2f", t->pricePerKwh);
                snprintf(total, sizeof(total), "%.2f", t->totalPrice);
                
                add_table_row(&table, id, buyer, seller, energy, price, total, t->timestamp);
                found++;
            }
        }
        cursor = cursor->next;
    }
    
    if (found) {
        print_table(&table);
    } else {
        printf("No transactions found for Buyer ID %d.\n", buyerID);
    }
    
    free_table(&table);
}

int isValidDateTimeFormat(const char* dateTime) {
    int year, month, day, hour, minute, second;
    // Check basic format using sscanf
    if (sscanf(dateTime, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6) {
        return 0;
    }
    // Check reasonable ranges for date
    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
        return 0;
    }
    // Check reasonable ranges for time
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        return 0;
    }
    // Additional checks for specific months
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
        return 0;
    }
    if (month == 2) {
        // Check for leap year
        int isLeapYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        if (day > (isLeapYear ? 29 : 28)) {
            return 0;
        }
    }
    return 1;
}

void deleteTransaction(int transactionID) {
    Transaction* t = findTransactionById(globalTransactionTree, transactionID);
    if (!t) {
        printf("Error: Transaction with ID %d does not exist.\n", transactionID);
        return;
    }
    
    int buyerID = t->buyerID;
    int sellerID = t->sellerID;
    double energyAmount = t->energyAmount;
    double totalPrice = t->totalPrice;
    
    // Find the seller and buyer
    Seller* seller = seller_head;
    while (seller && seller->sellerID != sellerID) {
        seller = seller->next;
    }
    
    Buyer* buyer = buyer_head;
    while (buyer && buyer->buyerID != buyerID) {
        buyer = buyer->next;
    }
    
    // Delete from global transaction tree
    deleteTransactionFromBPTree(&globalTransactionTree, transactionID);
    
    // Delete from seller's transaction tree if it exists
    if (seller && seller->transactionTree) {
        deleteTransactionFromBPTree(&seller->transactionTree, transactionID);
        seller->numTransactions--;
        seller->totalRevenue -= totalPrice;
    }
    
    // Delete from buyer's transaction tree if it exists
    if (buyer && buyer->transactionTree) {
        deleteTransactionFromBPTree(&buyer->transactionTree, transactionID);
        buyer->numTransactions--;
        buyer->totalEnergyPurchased -= energyAmount;
    }
    
    // Update the transaction file
    deleteTransactionFile(transactionID);
    printf("Transaction with ID %d successfully deleted.\n", transactionID);
}


BPTreeNode* findParent(BPTreeNode* root, BPTreeNode* child) {
    if (!root || root == child)
        return NULL;
    for (int i = 0; i <= root->numKeys; i++) {
        if (root->children[i] == child)
            return root;
    }
    for (int i = 0; i <= root->numKeys; i++) {
        if (!root->isLeaf) {
            BPTreeNode* parent = findParent(root->children[i], child);
            if (parent)
                return parent;
        }
    }
    return NULL;
}

void removeFromLeaf(BPTreeNode* node, int idx) {
    for (int i = idx; i < node->numKeys - 1; i++) {
        node->keys[i] = node->keys[i + 1];
        node->records[i] = node->records[i + 1];
    }
    node->numKeys--;
}

void removeFromNonLeaf(BPTreeNode** root, BPTreeNode* node, int idx) {
    int key = node->keys[idx];
    if (node->children[idx]->numKeys >= ORDER/2) {
        BPTreeNode* cursor = node->children[idx];
        while (!cursor->isLeaf)
            cursor = cursor->children[cursor->numKeys];
        int predKey = cursor->keys[cursor->numKeys - 1];
        node->keys[idx] = predKey;
        deleteTransactionFromBPTree(&node->children[idx], predKey);
    }
    else if (node->children[idx + 1]->numKeys >= ORDER/2) {
        BPTreeNode* cursor = node->children[idx + 1];
        while (!cursor->isLeaf)
            cursor = cursor->children[0];
        int succKey = cursor->keys[0];
        node->keys[idx] = succKey;
        deleteTransactionFromBPTree(&node->children[idx + 1], succKey);
    }
    else {
        mergeNodes(root, node, idx);
        deleteTransactionFromBPTree(&node->children[idx], key);
    }
}

void borrowFromNext(BPTreeNode* node, int idx) {
    BPTreeNode* child = node->children[idx];
    BPTreeNode* sibling = node->children[idx + 1];
    child->keys[child->numKeys] = node->keys[idx];
    if (!child->isLeaf)
        child->children[child->numKeys + 1] = sibling->children[0];
    else
        child->records[child->numKeys] = sibling->records[0];
    node->keys[idx] = sibling->keys[0];
    for (int i = 0; i < sibling->numKeys - 1; i++) {
        sibling->keys[i] = sibling->keys[i + 1];
        if (!sibling->isLeaf)
            sibling->children[i] = sibling->children[i + 1];
        else
            sibling->records[i] = sibling->records[i + 1];
    }
    if (!sibling->isLeaf)
        sibling->children[sibling->numKeys - 1] = sibling->children[sibling->numKeys];
    child->numKeys++;
    sibling->numKeys--;
}

void borrowFromPrev(BPTreeNode* node, int idx) {
    BPTreeNode* child = node->children[idx];
    BPTreeNode* sibling = node->children[idx - 1];
    for (int i = child->numKeys - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
        if (!child->isLeaf)
            child->children[i + 2] = child->children[i + 1];
        else
            child->records[i + 1] = child->records[i];
    }
    if (!child->isLeaf)
        child->children[1] = child->children[0];
    child->keys[0] = node->keys[idx - 1];
    if (!child->isLeaf)
        child->children[0] = sibling->children[sibling->numKeys];
    else
        child->records[0] = sibling->records[sibling->numKeys - 1];
    node->keys[idx - 1] = sibling->keys[sibling->numKeys - 1];
    child->numKeys++;
    sibling->numKeys--;
}

void mergeNodes(BPTreeNode** root, BPTreeNode* node, int idx) {
    BPTreeNode* leftChild = node->children[idx];
    BPTreeNode* rightChild = node->children[idx + 1];
    leftChild->keys[leftChild->numKeys] = node->keys[idx];
    for (int i = 0; i < rightChild->numKeys; i++) {
        leftChild->keys[leftChild->numKeys + 1 + i] = rightChild->keys[i];
        if (!leftChild->isLeaf)
            leftChild->children[leftChild->numKeys + 1 + i] = rightChild->children[i];
        else
            leftChild->records[leftChild->numKeys + 1 + i] = rightChild->records[i];
    }
    if (!leftChild->isLeaf)
        leftChild->children[leftChild->numKeys + 1 + rightChild->numKeys] = rightChild->children[rightChild->numKeys];
    if (leftChild->isLeaf)
        leftChild->next = rightChild->next;
    leftChild->numKeys += 1 + rightChild->numKeys;
    for (int i = idx; i < node->numKeys - 1; i++) {
        node->keys[i] = node->keys[i + 1];
        node->children[i + 1] = node->children[i + 2];
    }
    node->numKeys--;
    free(rightChild);
    if (node->numKeys == 0 && *root == node) {
        *root = leftChild;
        free(node);
    }
}

void deleteTransactionFromBPTree(BPTreeNode** root, int transactionID) {
    if (!*root) {
        printf("Tree is empty. Nothing to delete.\n");
        return;
    }
    BPTreeNode* cursor = *root;
    BPTreeNode* parent = NULL;
    int idx = 0;
    while (!cursor->isLeaf) {
        parent = cursor;
        for (idx = 0; idx < cursor->numKeys; idx++) {
            if (transactionID < cursor->keys[idx])
                break;
        }
        cursor = cursor->children[idx];
    }
    int keyIdx = -1;
    for (int i = 0; i < cursor->numKeys; i++) {
        if (cursor->keys[i] == transactionID) {
            keyIdx = i;
            break;
        }
    }
    if (keyIdx == -1) {
        printf("Transaction with ID %d not found in the tree.\n", transactionID);
        return;
    }
    free(cursor->records[keyIdx]);
    removeFromLeaf(cursor, keyIdx);
    if (cursor->numKeys == 0 && cursor == *root) {
        free(cursor);
        *root = NULL;
        return;
    }
    if (cursor->numKeys >= (ORDER - 1) / 2 || cursor == *root)
        return;
    int parentIdx = 0;
    while (parentIdx <= parent->numKeys && parent->children[parentIdx] != cursor)
        parentIdx++;
    if (parentIdx < parent->numKeys && parent->children[parentIdx + 1]->numKeys > (ORDER - 1) / 2) {
        borrowFromNext(parent, parentIdx);
    }
    else if (parentIdx > 0 && parent->children[parentIdx - 1]->numKeys > (ORDER - 1) / 2) {
        borrowFromPrev(parent, parentIdx);
    }
    else {
        if (parentIdx < parent->numKeys)
            mergeNodes(root, parent, parentIdx);
        else
            mergeNodes(root, parent, parentIdx - 1);
    }
}

void deleteTransactionFile(int transactionID) {
    FILE *originalFile = fopen(TRANSACTION_FILE, "r");
    if (!originalFile) {
        printf("Error opening transaction file for reading.\n");
        return;
    }
    FILE *tempFile = fopen("temp_transactions.txt", "w");
    if (!tempFile) {
        printf("Error creating temporary file.\n");
        fclose(originalFile);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), originalFile)) {
        int currentID;
        if (sscanf(line, "%d,", &currentID) == 1) {
            if (currentID != transactionID) {
                fputs(line, tempFile);
            }
        } else {
            fputs(line, tempFile);
        }
    }
    fclose(originalFile);
    fclose(tempFile);
    remove(TRANSACTION_FILE);
    rename("temp_transactions.txt", TRANSACTION_FILE);
}

void displayMenu() {
    printf("\n===== Energy Marketplace System =====\n");
    printf("1. Add a new transaction\n");
    printf("2. Display all transactions\n");
    printf("3. Find transactions by seller ID\n");
    printf("4. Find transactions by buyer ID\n");
    printf("5. Find transactions in a time period\n");
    printf("6. Calculate total revenue by seller ID\n");
    printf("7. Calculate total revenue for all sellers\n");
    printf("8. Find transactions by energy amount range\n");
    printf("9. Sort buyers by energy bought\n");           
    printf("10. Sort seller-buyer pairs by transactions\n");
    printf("11. Delete a transaction\n"); 
    printf("12. Debug\n");
    printf("13. Exit\n");
    printf("Enter your choice (1-13): ");
}

int main() {

    loadSellerPrices();
    loadDataFromFile();
    int choice;
    int running = 1;
    
    while (running) {
        displayMenu();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                int transactionID, buyerID, sellerID;
                double energyAmount;
                char timestamp[30]; 
                printf("\nEnter transaction details:\n");
                printf("Transaction ID: ");
                scanf("%d", &transactionID);
                if (findTransactionInBPTree(globalTransactionTree, transactionID)) {
                    printf("Error: Transaction with ID %d already exists. Cannot create duplicate transactions.\n", transactionID);
                    break;
                }
                printf("Buyer ID: ");
                scanf("%d", &buyerID);
                printf("Seller ID: ");
                scanf("%d", &sellerID);
                printf("Energy amount (kWh): ");
                scanf("%lf", &energyAmount);
                do {
                    printf("Date and time (YYYY-MM-DD HH:MM:SS): ");
                    scanf(" %[^\n]", timestamp); // Using space before %[ to consume any whitespace and reading until newline
                    
                    if (!isValidDateTimeFormat(timestamp)) {
                        printf("Invalid format. Please use YYYY-MM-DD HH:MM:SS format.\n");
                    }
                } while (!isValidDateTimeFormat(timestamp));
                Transaction* t = createTransaction(transactionID, buyerID, sellerID, energyAmount, 0.0, timestamp);
                insertTransaction(t);
                break;
            }
            case 2:
                displayTransactionsFromTree(globalTransactionTree);
                break;
            case 3: {
                Seller* seller = seller_head;
                if (!seller) {
                     printf("No sellers found.\n");
                } else {
                    while (seller) {
                        createSetOfTransactionsForSeller(seller->sellerID);
                         seller = seller->next;
                    }
                }
                break;
            }
            case 4: {
                Buyer* buyer = buyer_head;
                if (!buyer) {
                    printf("No buyers found.\n");
                } else {
                    while (buyer) {
                        createSetOfTransactionsForBuyer(buyer->buyerID);
                        buyer = buyer->next;
                    }
                }
                break;
            }
            case 5: {
                char startDateTime[30], endDateTime[30];
                do {
                    printf("\nEnter start date and time (YYYY-MM-DD HH:MM:SS): ");
                    scanf(" %[^\n]", startDateTime);
                    if (!isValidDateTimeFormat(startDateTime)) {
                        printf("Invalid format. Please use YYYY-MM-DD HH:MM:SS format.\n");
                    }
                } while (!isValidDateTimeFormat(startDateTime));
                do {
                    printf("Enter end date and time (YYYY-MM-DD HH:MM:SS): ");
                    scanf(" %[^\n]", endDateTime);
                    
                    if (!isValidDateTimeFormat(endDateTime)) {
                        printf("Invalid format. Please use YYYY-MM-DD HH:MM:SS format.\n");
                    }
                } while (!isValidDateTimeFormat(endDateTime));
                findTransactionsByTimeRange(startDateTime, endDateTime);
                break;
            }
            case 6: {
                int sellerID;
                printf("\nEnter seller ID: ");
                scanf("%d", &sellerID);
                calculateTotalRevenueBySellerID(sellerID);
                break;
            }
            case 7:
                calculateTotalRevenueForAllSellers();
                break;
            case 8: {
                double minEnergy, maxEnergy;
                printf("\nEnter minimum energy amount (kWh): ");
                scanf("%lf", &minEnergy);
                printf("Enter maximum energy amount (kWh): ");
                scanf("%lf", &maxEnergy);
                findTransactionsByEnergyRange(minEnergy, maxEnergy);
                break;
            }
            case 9:{
                sortBuyersByEnergyBought();
                break;
            }
            case 10:{
                sortSellerBuyerPairsByTransactions();
                break;
            }
            case 11: {
                int transactionID;
                printf("\nEnter transaction ID to delete: ");
                scanf("%d", &transactionID);
                deleteTransaction(transactionID);
                break;
            }
            case 12: {
                printf("\n===== Debug Menu =====\n");
                printf("1. Verify B+ Tree Structure\n");
                printf("2. Search for Transaction by ID\n");
                printf("3. List all Transaction IDs in order\n");
                printf("Enter debug option: ");
                int debugOption;
                scanf("%d", &debugOption);
                
                switch (debugOption) {
                    case 1: {
                        int count = countTransactionsInTree(globalTransactionTree);
                        printf("Total transactions in B+ tree: %d\n", count);
                        break;
                    }
                    case 2: {
                        int searchID;
                        printf("Enter transaction ID to search for: ");
                        scanf("%d", &searchID);
                        Transaction* t = findTransactionById(globalTransactionTree, searchID);
                        if (t) {
                            printf("Found Transaction ID: %d | Buyer ID: %d | Seller ID: %d | Energy: %.2f kWh | Price: %.2f/kWh | Total: %.2f | Time: %s\n",
                                    t->transactionID, t->buyerID, t->sellerID, 
                                    t->energyAmount, t->pricePerKwh, t->totalPrice, 
                                    t->timestamp);
                        } else {
                            printf("Transaction with ID %d not found in the tree.\n", searchID);
                        }
                        break;
                    }
                    case 3: {
                        BPTreeNode* cursor = globalTransactionTree;
                        if (!cursor) {
                            printf("Tree is empty.\n");
                            break;
                        }
                        while (!cursor->isLeaf) {
                            cursor = cursor->children[0];
                        }
                        printf("Transaction IDs in order: ");
                        int count = 0;
                        while (cursor) {
                            for (int i = 0; i < cursor->numKeys; i++) {
                                printf("%d ", cursor->keys[i]);
                                count++;
                            }
                            cursor = cursor->next;
                        }
                        printf("\nTotal: %d IDs\n", count);
                        break;
                    }
                    default:
                        printf("Invalid debug option.\n");
                }
                break;
            }            
            case 13:
                printf("\nExiting program. Goodbye!\n");
                running = 0;
                break;
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    }
    freeTransactions();
    return 0;
}