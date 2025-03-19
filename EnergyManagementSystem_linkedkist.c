#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TRANSACTIONS 10000
#define MAX_SELLERS 1000
#define MAX_BUYERS 1000
#define MAX_TRANSACTION_HISTORY 1000
#define MAX_REGULAR_BUYERS 1000
#define MAX_LINE_LENGTH 256
#define TRANSACTION_FILE "transactions.txt"

typedef struct time_stamp_tag{
    int year;
    int month;
    int day;
    int hour;
    int minutes;
    int seconds;
} time_stamp;

typedef struct transaction_tag {
    int transaction_id;
    int buyer_id;
    int seller_id;
    float energy_kwh;
    float price_per_kwh;
    time_stamp time_stamp;
    struct transaction_tag *prev;
    struct transaction_tag *next;
} transaction;

typedef struct seller_tag {
    int seller_id;
    int transaction_count;
    float revenue;
    float rate_below_300;  
    float rate_above_300;  
    int regular_buyers[MAX_REGULAR_BUYERS];  
    transaction transaction_history[MAX_TRANSACTION_HISTORY]; 
    struct seller_tag *prev;
    struct seller_tag *next;
} seller;

typedef struct buyer_tag {
    int buyer_id;
    int transaction_count;
    float total_energy;
    transaction transaction_history[MAX_TRANSACTION_HISTORY]; 
    struct buyer_tag *prev;
    struct buyer_tag *next;
} buyer;

typedef struct seller_buyer_pair_tag {
    int seller_id;
    int buyer_id;
    int transaction_count;
    float total_revenue;
    struct seller_buyer_pair_tag *prev;
    struct seller_buyer_pair_tag *next;
} seller_buyer_pair;

seller* find_Seller(int sellerID);
buyer* find_Buyer(int buyerID);

transaction* transaction_head = NULL;
buyer* buyer_head = NULL;
seller* seller_head = NULL;
seller_buyer_pair* pair_head = NULL;

transaction* createTransactionNode(int transactionID, int buyerID, int sellerID, float energy_kwh, float price_per_kwh, time_stamp ts, seller* sellerNode) {
    transaction* newTransaction = (transaction*)malloc(sizeof(transaction));
    if (!newTransaction) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    newTransaction->transaction_id = transactionID; 
    newTransaction->buyer_id = buyerID; 
    newTransaction->seller_id = sellerID;  
    newTransaction->energy_kwh = energy_kwh;
    newTransaction->price_per_kwh = price_per_kwh;
    newTransaction->time_stamp = ts;
    newTransaction->next = NULL;
    newTransaction->prev = NULL;

    // Adjust price based on energy threshold
    newTransaction->price_per_kwh = (energy_kwh > 300) ? sellerNode->rate_above_300 : sellerNode->rate_below_300;

    return newTransaction;
}

void add_Transaction(int transactionID, int buyerID, int sellerID, float energyAmount, float pricePerKWh, time_stamp ts);


seller* createSellerNode(int seller_id, float rate_below_300, float rate_above_300) {
    seller* newNode = (seller*) malloc(sizeof(seller));
    if (newNode != NULL) {
        newNode->seller_id = seller_id;
        newNode->transaction_count = 0;
        newNode->revenue = 0.0;
        newNode->rate_below_300 = rate_below_300;
        newNode->rate_above_300 = rate_above_300;
        newNode->prev = NULL;
        newNode->next = NULL;
        memset(newNode->regular_buyers, 0, sizeof(newNode->regular_buyers));  // Initialize regular buyers list
    }
    return newNode;
}

buyer* createBuyerNode(int buyer_id) {
    buyer* newNode = (buyer*) malloc(sizeof(buyer));
    if (newNode != NULL) {
        newNode->buyer_id = buyer_id;
        newNode->transaction_count = 0;
        newNode->total_energy = 0.0;
        newNode->prev = NULL;
        newNode->next = NULL;
    }
    return newNode;
}

seller_buyer_pair* createSellerBuyerPairNode(int seller_id, int buyer_id) {
    seller_buyer_pair* newNode = (seller_buyer_pair*) malloc(sizeof(seller_buyer_pair));
    if (newNode != NULL) {
        newNode->seller_id = seller_id;
        newNode->buyer_id = buyer_id;
        newNode->transaction_count = 0;
        newNode->total_revenue = 0.0;
        newNode->prev = NULL;
        newNode->next = NULL;
    }
    return newNode;
}

time_stamp get_time() {
    time_stamp ts;
    printf("Enter the year (YYYY): ");
    scanf("%d", ts.year);
    printf("Enter the month (MM (from 1-12)): ");
    scanf("%d", ts.month);
    printf("Enter the day (DD): ");
    scanf("%d", ts.day);
    printf("Enter hour: ");
    scanf("%d", ts.hour);
    printf("Enter minutes: ");
    scanf("%d", ts.minutes);
    printf("Enter seconds: ");
    scanf("%d", ts.seconds);
    return ts;
}

void add_Buyer(int buyerID);
void add_Seller(int sellerID, float rate_below_300, float rate_above_300);

void save_data_to_file() {
    FILE *file = fopen(TRANSACTION_FILE, "w");
    if (file == NULL) {
        printf("Error: Unable to open file for writing: %s\n", TRANSACTION_FILE);
        return;
    }

    // First save all seller information
    seller *current_seller = seller_head;
    while (current_seller != NULL) {
        fprintf(file, "Seller,%d,%.2f,%.2f\n", 
                current_seller->seller_id,
                current_seller->rate_below_300,
                current_seller->rate_above_300);
        current_seller = current_seller->next;
    }

    // Then save all transactions
    transaction *current_trans = transaction_head;
    while (current_trans != NULL) {
        fprintf(file, "Transaction,%d,%d,%d,%.2f,%.2f,%04d-%02d-%02d %02d:%02d:%02d\n",
                current_trans->transaction_id,
                current_trans->buyer_id,
                current_trans->seller_id,
                current_trans->energy_kwh,
                current_trans->price_per_kwh,
                current_trans->time_stamp.year,
                current_trans->time_stamp.month,
                current_trans->time_stamp.day,
                current_trans->time_stamp.hour,
                current_trans->time_stamp.minutes,
                current_trans->time_stamp.seconds);
        current_trans = current_trans->next;
    }

    fclose(file);
    printf("Data successfully saved to %s\n", TRANSACTION_FILE);
}

// Helper function to parse seller line
int parse_seller_line(char *line, int *seller_id, float *rate_below, float *rate_above) {
    if (sscanf(line, "Seller,%d,%f,%f", seller_id, rate_below, rate_above) != 3) {
        printf("Error: Invalid seller data format: %s", line);
        return 0;
    }
    return 1;
}

// Helper function to parse transaction line
int parse_transaction_line(char *line, int *trans_id, int *buyer_id, int *seller_id,
                         float *energy, float *price, time_stamp *ts) {
    if (sscanf(line, "Transaction,%d,%d,%d,%f,%f,%d-%d-%d %d:%d:%d",
               trans_id, buyer_id, seller_id, energy, price,
               &ts->year, &ts->month, &ts->day,
               &ts->hour, &ts->minutes, &ts->seconds) != 11) {
        printf("Error: Invalid transaction data format: %s", line);
        return 0;
    }
    return 1;
}

void load_data_from_file() {
    FILE *file = fopen(TRANSACTION_FILE, "r");
    if (!file) {
        printf("Notice: No existing data file found (%s)\n", TRANSACTION_FILE);
        return;
    }

    // Clear existing data
    while (transaction_head != NULL) {
        transaction *temp = transaction_head;
        transaction_head = transaction_head->next;
        free(temp);
    }
    while (seller_head != NULL) {
        seller *temp = seller_head;
        seller_head = seller_head->next;
        free(temp);
    }
    while (buyer_head != NULL) {
        buyer *temp = buyer_head;
        buyer_head = buyer_head->next;
        free(temp);
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;

    // First pass: Load all sellers
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        
        // Skip empty lines
        if (strlen(line) <= 1) continue;

        if (strncmp(line, "Seller,", 7) == 0) {
            int seller_id;
            float rate_below, rate_above;

            if (!parse_seller_line(line, &seller_id, &rate_below, &rate_above)) {
                printf("Warning: Skipping invalid seller data at line %d\n", line_number);
                continue;
            }

            add_Seller(seller_id, rate_below, rate_above);
            printf("Loaded seller: ID=%d, Rates: %.2f/%.2f\n", 
                   seller_id, rate_below, rate_above);
        }
    }

    // Reset file pointer for second pass
    rewind(file);
    line_number = 0;

    // Second pass: Load all transactions
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        
        // Skip empty lines
        if (strlen(line) <= 1) continue;

        if (strncmp(line, "Transaction,", 12) == 0) {
            int trans_id, buyer_id, seller_id;
            float energy, price;
            time_stamp ts;

            if (!parse_transaction_line(line, &trans_id, &buyer_id, &seller_id,
                                     &energy, &price, &ts)) {
                printf("Warning: Skipping invalid transaction at line %d\n", line_number);
                continue;
            }

            // Verify seller exists
            seller *seller_node = find_Seller(seller_id);
            if (!seller_node) {
                printf("Error: Seller %d not found for transaction %d\n", 
                       seller_id, trans_id);
                continue;
            }

            // Create buyer if doesn't exist
            buyer *buyer_node = find_Buyer(buyer_id);
            if (!buyer_node) {
                add_Buyer(buyer_id);
            }

            // Add the transaction
            add_Transaction(trans_id, buyer_id, seller_id, energy, price, ts);
            printf("Loaded transaction: ID=%d, Buyer=%d, Seller=%d\n", 
                   trans_id, buyer_id, seller_id);
        }
    }

    fclose(file);
    printf("Data loading completed successfully\n");
}


void add_Buyer(int buyerID) {
    buyer* newBuyer = createBuyerNode(buyerID);
    if (newBuyer == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    if (buyer_head == NULL) {
        buyer_head = newBuyer;
    } else {
        buyer* temp = buyer_head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newBuyer;
        newBuyer->prev = temp;
    }
    printf("Buyer with ID %d added successfully.\n", buyerID);
}

void add_Seller(int sellerID, float rate_below_300, float rate_above_300) {
    seller* newSeller = createSellerNode(sellerID, rate_below_300, rate_above_300);
    
    if (newSeller == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    
    if (seller_head == NULL) {
        seller_head = newSeller;
    } else {
        seller* temp = seller_head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newSeller;
        newSeller->prev = temp;
    }

    printf("Seller with ID %d added successfully.\n", sellerID);
}


transaction* find_transaction(int transactionID) {
    transaction* current = transaction_head;
    while (current) {
        if (current->transaction_id == transactionID) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

buyer* find_Buyer(int buyerID) {
    buyer* current = buyer_head;
    while (current) {
        if (current->buyer_id == buyerID) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

seller* find_Seller(int sellerID) {
    seller* current = seller_head;
    while (current) {
        if (current->seller_id == sellerID) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

seller_buyer_pair* find_or_add_pair(int seller_id, int buyer_id) {
    seller_buyer_pair* current = pair_head;

    // Check if pair already exists
    while (current) {
        if (current->seller_id == seller_id && current->buyer_id == buyer_id) {
            return current;
        }
        current = current->next;
    }

    // If pair does not exist, create a new one
    seller_buyer_pair* new_pair = createSellerBuyerPairNode(seller_id, buyer_id);
    if (!new_pair) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    // Insert at the head of the doubly linked list
    new_pair->next = pair_head;
    new_pair->prev = NULL;

    if (pair_head) {
        pair_head->prev = new_pair;
    }

    pair_head = new_pair;
    return new_pair;
}

int is_duplicate_transaction(int transactionID, int buyerID) {
    transaction* current = transaction_head;
    while (current != NULL) {
        if (current->transaction_id == transactionID && current->buyer_id == buyerID) {
            return 1; 
        }
        current = current->next;
    }
    return 0; 
}

void add_Transaction(int transactionID, int buyerID, int sellerID, float energyAmount, float pricePerKWh, time_stamp ts) {
    // Find or create a seller-buyer pair
    seller_buyer_pair* pair = find_or_add_pair(sellerID, buyerID);
    seller* sellerNode = find_Seller(sellerID);

    if (is_duplicate_transaction(transactionID, buyerID)) {
        printf("Error: Duplicate transaction detected! Transaction ID %d with Buyer ID %d already exists.\n", 
               transactionID, buyerID);
        return;
    }
    
    transaction* newTransaction = createTransactionNode(transactionID, buyerID, sellerID, energyAmount, pricePerKWh, ts, sellerNode);
    
    newTransaction->time_stamp = ts;
    newTransaction->prev = NULL;  // Ensure proper initialization

    // Add transaction to the linked list of transactions
    if (!transaction_head) {
        transaction_head = newTransaction;  
    } else {
        transaction* temp = transaction_head;
        while (temp->next) {
            temp = temp->next;  
        }
        temp->next = newTransaction;  
        newTransaction->prev = temp;  
    }

    // Find or create buyer node
    buyer* buyerNode = find_Buyer(buyerID);
    if (!buyerNode) {
        add_Buyer(buyerID);  
        buyerNode = find_Buyer(buyerID);  
    }
    if (buyerNode != NULL) {
        buyerNode->total_energy += energyAmount;  
        printf("Buyer ID: %d, Total Energy Updated: %.2f kWh\n", buyerNode->buyer_id, buyerNode->total_energy);
    }

    // Update seller's revenue based on adjusted price
    sellerNode->revenue += (energyAmount * pricePerKWh);
    
    // Add full transaction to buyer's transaction history if within limits
    if (buyerNode->transaction_count < MAX_TRANSACTION_HISTORY) {
        buyerNode->transaction_history[buyerNode->transaction_count] = *newTransaction;  // Store entire transaction
        buyerNode->transaction_count++;
    } else {
        printf("Warning: Buyer transaction history is full.\n");
    }

    // Add full transaction to seller's transaction history if within limits
    if (sellerNode->transaction_count < MAX_TRANSACTION_HISTORY) {
        sellerNode->transaction_history[sellerNode->transaction_count] = *newTransaction;  // Store entire transaction
        sellerNode->transaction_count++;
    } else {
        printf("Warning: Seller transaction history is full.\n");
    }

    // Check if the buyer qualifies as a regular buyer
    if (buyerNode->transaction_count > 5) {
        int isRegularBuyer = 0;

        // Check if buyer is already in the regular buyers list
        for (int i = 0; i < MAX_REGULAR_BUYERS; i++) {
            if (sellerNode->regular_buyers[i] == buyerID) {
                isRegularBuyer = 1;
                break;
            }
        }

        // If buyer is not already in the list, add the buyer
        if (!isRegularBuyer) {
            for (int i = 0; i < MAX_REGULAR_BUYERS; i++) {
                if (sellerNode->regular_buyers[i] == 0) {  // Find an empty slot
                    sellerNode->regular_buyers[i] = buyerID;  // Add buyer to regular buyers list
                    break;
                }
            }
        }
    }

    // Update pair's transaction count and revenue
    pair->transaction_count++;
    pair->total_revenue += (energyAmount * pricePerKWh);

    
    printf("Transaction %d added successfully with adjusted price Rs %.2f per kWh.\n", transactionID, pricePerKWh);

   
    float rate_below_300_kWh = sellerNode->rate_below_300;  
    float rate_above_300_kWh = sellerNode->rate_above_300;  

    save_data_to_file();
}

void print_Transactions() {

    if (!transaction_head) {
        printf("No transactions available.\n");
        return;
    }

    transaction* temp = transaction_head;
    while (temp) {
        printf("Transaction ID: %d, Buyer ID: %d, Seller ID: %d, Energy: %.2f kWh, Price: %.2f Rs, Date: %04d-%02d-%02d %02d:%02d:%02d\n",
               temp->transaction_id, temp->buyer_id, temp->seller_id, temp->energy_kwh, temp->price_per_kwh,
               temp->time_stamp.year, temp->time_stamp.month, temp->time_stamp.day, temp->time_stamp.hour,
               temp->time_stamp.minutes, temp->time_stamp.seconds);
        temp = temp->next;
    }
}



void print_Transactions_for_Seller(int sellerID) {
    seller* current = find_Seller(sellerID);

    if (current == NULL) {
        printf("Seller %d not found.\n", sellerID);
        return;
    }

    
    if (current->transaction_count == 0) {
        printf("No transactions found for Seller %d.\n", sellerID);
        return;
    }

    printf("Transactions for Seller %d:\n", sellerID);

    // Traverse through the seller's transaction history
    for (int i = 0; i < current->transaction_count; i++) {
        transaction* t = &current->transaction_history[i];  

        printf("Transaction ID: %d, Buyer ID: %d, Energy: %.2f kWh, Price per kWh: Rs %.2f, Date: %04d-%02d-%02d %02d:%02d:%02d\n",
               t->transaction_id, t->buyer_id, t->energy_kwh, t->price_per_kwh,
               t->time_stamp.year, t->time_stamp.month, t->time_stamp.day, 
               t->time_stamp.hour, t->time_stamp.minutes, t->time_stamp.seconds);
    }
}




void print_Transactions_for_Buyer(int buyerID) {
    buyer* current = find_Buyer(buyerID);

    if (current == NULL) {
        printf("Buyer %d not found.\n", buyerID);
        return;
    }

    
    if (current->transaction_count == 0) {
        printf("No transactions found for Buyer %d.\n", buyerID);
        return;
    }

    printf("Transactions for Buyer %d:\n", buyerID);

    // Traverse through the buyer's transaction history
    for (int i = 0; i < current->transaction_count; i++) {
        transaction* t = &current->transaction_history[i];  

        printf("Transaction ID: %d, Seller ID: %d, Energy: %.2f kWh, Price per kWh: Rs %.2f, Date: %04d-%02d-%02d %02d:%02d:%02d\n",
               t->transaction_id, t->seller_id, t->energy_kwh, t->price_per_kwh,
               t->time_stamp.year, t->time_stamp.month, t->time_stamp.day, 
               t->time_stamp.hour, t->time_stamp.minutes, t->time_stamp.seconds);
    }
}



int date_range(time_stamp ts, int start_year, int start_month, int start_day, int start_hour, int start_minutes, int start_seconds, 
    int end_year, int end_month, int end_day, int end_hour, int end_minutes, int end_seconds) 
{
    // Check if the timestamp is before the start time
    if (ts.year < start_year || (ts.year == start_year && ts.month < start_month) ||
        (ts.year == start_year && ts.month == start_month && ts.day < start_day) ||
        (ts.year == start_year && ts.month == start_month && ts.day == start_day && ts.hour < start_hour) ||
        (ts.year == start_year && ts.month == start_month && ts.day == start_day && ts.hour == start_hour && ts.minutes < start_minutes) ||
        (ts.year == start_year && ts.month == start_month && ts.day == start_day && ts.hour == start_hour && ts.minutes == start_minutes && ts.seconds < start_seconds))
    {
        return 0;
    }

    // Check if the timestamp is after the end time
    if (ts.year > end_year || (ts.year == end_year && ts.month > end_month) ||
        (ts.year == end_year && ts.month == end_month && ts.day > end_day) ||
        (ts.year == end_year && ts.month == end_month && ts.day == end_day && ts.hour > end_hour) ||
        (ts.year == end_year && ts.month == end_month && ts.day == end_day && ts.hour == end_hour && ts.minutes > end_minutes) ||
        (ts.year == end_year && ts.month == end_month && ts.day == end_day && ts.hour == end_hour && ts.minutes == end_minutes && ts.seconds > end_seconds))
    {
        return 0;  
    }
    return 1; 
}

void transactions_in_a_given_period(int start_year, int start_month, int start_day, int start_hour, int start_minutes, int start_seconds,
    int end_year, int end_month, int end_day, int end_hour, int end_minutes, int end_seconds) 
{
    int found = 0;

    printf("Transactions between %04d-%02d-%02d %02d:%02d:%02d and %04d-%02d-%02d %02d:%02d:%02d\n", 
        start_year, start_month, start_day, start_hour, start_minutes, start_seconds,
        end_year, end_month, end_day, end_hour, end_minutes, end_seconds);

    transaction* current = transaction_head;

    while (current != NULL) 
    {
        if (date_range(current->time_stamp, start_year, start_month, start_day, start_hour, start_minutes, start_seconds, 
            end_year, end_month, end_day, end_hour, end_minutes, end_seconds)) 
        {
            printf("Transaction ID: %d, Buyer ID: %d, Seller ID: %d, Energy: %.2f kWh, Price per kWh: Rs %.2f, Date: %04d-%02d-%02d %02d:%02d:%02d\n",
                current->transaction_id, current->buyer_id, current->seller_id, current->energy_kwh, current->price_per_kwh,
                current->time_stamp.year, current->time_stamp.month, current->time_stamp.day,
                current->time_stamp.hour, current->time_stamp.minutes, current->time_stamp.seconds);
            found = 1;
        }
        current = current->next; 
    }

    if (!found) 
    {
        printf("No transactions found in the specified time period.\n");
    }
}


float total_revenue_by_seller(int sellerID) {
    seller* sellerNode = find_Seller(sellerID);  // Find the seller by ID
    if (sellerNode == NULL) {
        printf("Seller %d not found.\n", sellerID);
        return 0.0;
    }

    float totalRevenue = 0.0;
    sellerNode->revenue = 0.0;  

    // Traverse through the seller's transaction history array
    for (int i = 0; i < sellerNode->transaction_count; i++) {
        transaction* t = &sellerNode->transaction_history[i]; 
        totalRevenue += t->energy_kwh * t->price_per_kwh;  
    }

    sellerNode->revenue = totalRevenue;  
    return totalRevenue; 
}



void total_revenue() {
    seller* currentSeller = seller_head;  
    while (currentSeller != NULL) {
        float totalRevenue = 0.0;
        currentSeller->revenue = 0.0;  

        // Iterate through the seller's transaction history array
        for (int i = 0; i < currentSeller->transaction_count; i++) {
            transaction* currentTransaction = &currentSeller->transaction_history[i];  
            totalRevenue += currentTransaction->energy_kwh * currentTransaction->price_per_kwh;  
        }

        currentSeller->revenue = totalRevenue;  
        currentSeller = currentSeller->next;  
    }
}


void total_energy() {
    buyer* currentBuyer = buyer_head;  
    while (currentBuyer != NULL) {
        currentBuyer->total_energy = 0.0;  

        // Iterate through the linked list of transactions for the current buyer
        transaction* currentTransaction = transaction_head;
        while (currentTransaction != NULL) {
            if (currentTransaction->buyer_id == currentBuyer->buyer_id) {
                currentBuyer->total_energy += currentTransaction->energy_kwh;
            }
            currentTransaction = currentTransaction->next;  
        }

        currentBuyer = currentBuyer->next;  
    }
}



void Transaction_with_highest_energy() {
    if (transaction_head == NULL) {
        printf("No transactions available.\n");
        return;
    }

    transaction* currentTransaction = transaction_head;  
    transaction* maxTransaction = currentTransaction;  
    float maxEnergy = currentTransaction->energy_kwh;  
    currentTransaction = currentTransaction->next;

    while (currentTransaction != NULL) {
        if (currentTransaction->energy_kwh > maxEnergy) {
            maxTransaction = currentTransaction;  
            maxEnergy = currentTransaction->energy_kwh;  
        }
        currentTransaction = currentTransaction->next;  
    }

   
    printf("Transaction with the highest energy:\n");
    printf("Transaction ID: %d\n", maxTransaction->transaction_id);
    printf("Buyer ID: %d\n", maxTransaction->buyer_id);
    printf("Seller ID: %d\n", maxTransaction->seller_id);
    printf("Energy: %.2f kWh\n", maxTransaction->energy_kwh);
    printf("Price per kWh: Rs %.2f\n", maxTransaction->price_per_kwh);
    printf("Date: %04d-%02d-%02d %02d:%02d:%02d\n",
            maxTransaction->time_stamp.year,
            maxTransaction->time_stamp.month,
            maxTransaction->time_stamp.day,
            maxTransaction->time_stamp.hour,
            maxTransaction->time_stamp.minutes,
            maxTransaction->time_stamp.seconds);
}


// Merge two sorted transaction lists into one sorted list based on energy (descending order)
transaction* merge_Transactions(transaction* left, transaction* right) {
    if (!left) return right;
    if (!right) return left;

    if (left->energy_kwh > right->energy_kwh) {
        left->next = merge_Transactions(left->next, right);
        if (left->next) left->next->prev = left;
        left->prev = NULL;
        return left;
    } else {
        right->next = merge_Transactions(left, right->next);
        if (right->next) right->next->prev = right;
        right->prev = NULL;
        return right;
    }
}

// Split the transaction list into two halves
transaction* split_Transactions(transaction* head) {
    if (!head || !head->next) return head;

    transaction* slow = head;
    transaction* fast = head->next;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    transaction* middle = slow->next;
    slow->next = NULL;

    return middle;
}

// Merge Sort function to sort transactions by energy
transaction* mergeSort_Transactions(transaction* head) {
    if (!head || !head->next) return head;

    transaction* middle = split_Transactions(head);
    transaction* left = mergeSort_Transactions(head);
    transaction* right = mergeSort_Transactions(middle);

    return merge_Transactions(left, right);
}

// Function to sort transactions by energy (in descending order)
void sort_Transactions_By_Energy() {
    if (!transaction_head) {
        printf("No transactions to sort.\n");
        return;
    }

    // Sort the transactions by energy (descending)
    transaction_head = mergeSort_Transactions(transaction_head);

    // Optionally, print the sorted transactions to verify
    transaction* temp = transaction_head;
    while (temp) {
        printf("Transaction ID: %d, Energy: %.2f kWh, Price per kWh: Rs %.2f, Date: %d-%d-%d %d:%d:%d\n",
               temp->transaction_id, temp->energy_kwh, temp->price_per_kwh,
               temp->time_stamp.year, temp->time_stamp.month, temp->time_stamp.day,
               temp->time_stamp.hour, temp->time_stamp.minutes, temp->time_stamp.seconds);
        temp = temp->next;
    }
}

// Function to merge two sorted buyer lists based on total_energy
buyer* mergeBuyers(buyer* head1, buyer* head2) {
    buyer* result = NULL;
    buyer* temp = NULL;

    while (head1 != NULL && head2 != NULL) {
        if (head1->total_energy >= head2->total_energy) {
            if (result == NULL) {
                result = temp = head1;
            } else {
                temp->next = head1;
                head1->prev = temp;
                temp = temp->next;
            }
            head1 = head1->next;
        } else {
            if (result == NULL) {
                result = temp = head2;
            } else {
                temp->next = head2;
                head2->prev = temp;
                temp = temp->next;
            }
            head2 = head2->next;
        }
    }

    // Attach remaining nodes, if any
    if (head1 != NULL) {
        temp->next = head1;
        head1->prev = temp;
    }
    if (head2 != NULL) {
        temp->next = head2;
        head2->prev = temp;
    }

    return result;
}

// Function to split the buyer list into two halves
buyer* divideBuyerList(buyer* head) {
    if (head == NULL || head->next == NULL) return NULL;

    buyer* slow = head;
    buyer* fast = head;

    while (fast != NULL && fast->next != NULL) {
        fast = fast->next->next;
        if (fast) slow = slow->next;
    }

    buyer* middleNext = slow->next;
    slow->next = NULL;
    if (middleNext) middleNext->prev = NULL;

    return middleNext;
}

// Merge Sort function for buyer list
buyer* mergeSortBuyers(buyer* head) {
    if (head == NULL || head->next == NULL) return head;

    buyer* middle = divideBuyerList(head);
    buyer* left = mergeSortBuyers(head);
    buyer* right = mergeSortBuyers(middle);

    return mergeBuyers(left, right);
}

// Function to sort the entire buyer list
void sortBuyerList() {
    if (buyer_head != NULL) {
        buyer_head = mergeSortBuyers(buyer_head);
        buyer* current = buyer_head;
    }
}

// Function to print the buyer list
void printBuyerList() {
    buyer* current = buyer_head;
    
    while (current != NULL) {
        printf("Buyer ID: %d\n", current->buyer_id);
        printf("Total Energy: %.2f kWh\n", current->total_energy);
        printf("\n");
        current = current->next;
    }
}

seller_buyer_pair* mergePairs(seller_buyer_pair* head1, seller_buyer_pair* head2) 
{
    seller_buyer_pair* result = NULL, *temp = NULL;
    
    while (head1 && head2) {
        if (head1->transaction_count >= head2->transaction_count) {
            if (!result) result = temp = head1;
            else { temp->next = head1; head1->prev = temp; temp = temp->next; }
            head1 = head1->next;
        } else {
            if (!result) result = temp = head2;
            else { temp->next = head2; head2->prev = temp; temp = temp->next; }
            head2 = head2->next;
        }
    }
    
    if (head1) temp->next = head1, head1->prev = temp;
    if (head2) temp->next = head2, head2->prev = temp;
    
    return result;
}

seller_buyer_pair* dividePairList(seller_buyer_pair* head) {
    if (!head || !head->next) return NULL;
    seller_buyer_pair *slow = head, *fast = head;
    
    while (fast && fast->next) {
        fast = fast->next->next;
        if (fast) slow = slow->next;
    }
    
    seller_buyer_pair* middleNext = slow->next;
    slow->next = NULL;
    if (middleNext) middleNext->prev = NULL;
    
    return middleNext;
}

seller_buyer_pair* mergeSortPairs(seller_buyer_pair* head) {
    if (!head || !head->next) return head;
    seller_buyer_pair* middle = dividePairList(head);
    return mergePairs(mergeSortPairs(head), mergeSortPairs(middle));
}

void sortSellerBuyerPairs() {
    if (pair_head) {
        pair_head = mergeSortPairs(pair_head);
        printf("Pairs sorted by transaction count.\n");
    } else {
        printf("No seller-buyer pairs available to sort.\n");
    }
}

void printSortedPairs() {
    printf("\nSeller-Buyer Pairs Sorted by Transactions:\n");
    seller_buyer_pair* current = pair_head;
    while (current) {
        printf("Seller ID: %d, Buyer ID: %d, Transactions: %d, Total Revenue: Rs %.2f\n",
               current->seller_id, current->buyer_id, current->transaction_count, current->total_revenue);
        current = current->next;
    }
}


int update_transaction_details(int transactionID, float new_energy_kwh, float new_price_per_kwh) {
    // Find the transaction by its ID
    transaction* trans = find_transaction(transactionID);
    if (trans == NULL) {
        printf("Transaction not found.\n");
        return 0; 
    }

    // Find the seller related to the transaction
    seller* seller_node = find_Seller(trans->seller_id);
    if (seller_node == NULL) {
        printf("Seller not found.\n");
        return 0;
    }

    // Deduct previous revenue based on the old energy amount and price
    float previous_price = (trans->energy_kwh > 300) ? seller_node->rate_above_300 : seller_node->rate_below_300;
    seller_node->revenue -= (trans->energy_kwh * previous_price);

    // Update the transaction details with the new energy and price
    trans->energy_kwh = new_energy_kwh;
    trans->price_per_kwh = new_price_per_kwh;

    // Calculate new adjusted price based on the new energy amount
    float adjustedPrice = (new_energy_kwh > 300) ? seller_node->rate_above_300 : seller_node->rate_below_300;

    // Add new revenue based on the updated transaction details
    seller_node->revenue += (new_energy_kwh * adjustedPrice);

    // Successfully updated the transaction
    printf("Transaction %d updated successfully.\n", transactionID);
    save_data_to_file();
    return 1; 
}



int update_buyer_seller_transaction_history(transaction* updated_trans) {
    int result = 1;  // Assume success

    // Update for Buyer
    buyer* buyerNode = find_Buyer(updated_trans->buyer_id);
    if (buyerNode != NULL) {
        int found = 0;
        for (int i = 0; i < buyerNode->transaction_count; i++) {
            if (buyerNode->transaction_history[i].transaction_id == updated_trans->transaction_id) {
                buyerNode->transaction_history[i] = *updated_trans;  
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("Transaction not found in buyer's history.\n");
            result = 0;  // Mark failure if the transaction wasn't found for the buyer
        }
    }

    // Update for Seller
    seller* sellerNode = find_Seller(updated_trans->seller_id);
    if (sellerNode != NULL) {
        int found = 0;
        for (int i = 0; i < sellerNode->transaction_count; i++) {
            if (sellerNode->transaction_history[i].transaction_id == updated_trans->transaction_id) {
                sellerNode->transaction_history[i] = *updated_trans;  
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("Transaction not found in seller's history.\n");
            result = 0;  // Mark failure if the transaction wasn't found for the seller
        }
    }

    return result;  // Return 1 for success, 0 for failure
}


int main() {
    int num_transactions, choice, transactionID, buyerID, sellerID;
    float energy_kwh, price_per_kwh;
    float new_energy_kwh, new_price_per_kwh;
    time_stamp ts, start, end;
    load_data_from_file();

    while (1) {
        printf("\nMenu:\n");
        printf("1. Add Transaction\n");
        printf("2. Display All Transactions\n");
        printf("3. Display Transactions for a Seller\n");
        printf("4. Display Transactions for a Buyer\n");
        printf("5. Display transactions in a given time period\n");
        printf("6. Calculate Total Revenue by Seller\n");
        printf("7. Find Transaction with Highest Energy\n");
        printf("8. Sort Transactions by Energy\n");
        printf("9. Sort Buyers by Energy\n");
        printf("10. Sort pairs by transaction\n");
        printf("11. Update Transaction\n");
        printf("12. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter the number of transactions to add: ");
                scanf("%d", &num_transactions);

                for (int i = 0; i < num_transactions; i++) {
                    printf("\nEnter details for Transaction %d:\n", i + 1);
                    printf("Enter Transaction ID: ");
                    scanf("%d", &transactionID);
                    printf("Enter Buyer ID: ");
                    scanf("%d", &buyerID);
                    printf("Enter Seller ID: ");
                    scanf("%d", &sellerID);
                    printf("Enter Energy Amount (kWh): ");
                    scanf("%f", &energy_kwh);
                    printf("Enter Year, Month, Day, Hour, Minute, Second (timestamp): \n");
                    scanf("%d %d %d %d %d %d", &ts.year, &ts.month, &ts.day, &ts.hour, &ts.minutes, &ts.seconds);
                    // Calculate the price based on energy amount
                    seller* sellerNode = find_Seller(sellerID);
                    if (!sellerNode) {
                        printf("Seller ID %d not found. Please enter the rates for the seller.\n", sellerID);
                        // Get the rates from the user before proceeding
                        float rate_below_300, rate_above_300;
                        printf("Enter rate below 300 kWh: ");
                        scanf("%f", &rate_below_300);
                        printf("Enter rate above 300 kWh: ");
                        scanf("%f", &rate_above_300);
                        add_Seller(sellerID, rate_below_300, rate_above_300);
                        sellerNode = find_Seller(sellerID); 
                    }

                     // Create a new transaction node, passing the adjusted price based on energy amount
                    float adjustedPrice = (energy_kwh > 300) ? sellerNode->rate_above_300 : sellerNode->rate_below_300;
                    add_Transaction(transactionID, buyerID, sellerID, energy_kwh, adjustedPrice, ts);

                }
                break;

            case 2:
                print_Transactions();
                break;

            case 3:
                printf("Enter Seller ID: ");
                scanf("%d", &sellerID);
                print_Transactions_for_Seller(sellerID);
                break;

            case 4:
                printf("Enter Buyer ID: ");
                scanf("%d", &buyerID);
                print_Transactions_for_Buyer(buyerID);
                break;

            case 5:
                printf("Enter start time attributes(year month(1-12) date(1-31) hour(1-12) minutes(1-60) seconds(1-60)): \n");
                scanf("%d %d %d %d %d %d", &start.year, &start.month, &start.day, &start.hour, &start.minutes, &start.seconds);
                printf("Enter end time attributes(year month(1-12) date(1-31) hour(1-12) minutes(1-60) seconds(1-60)): \n");
                scanf("%d %d %d %d %d %d", &end.year, &end.month, &end.day, &end.hour, &end.minutes, &end.seconds);
                transactions_in_a_given_period(start.year, start.month, start.day, start.hour, start.minutes, start.seconds, end.year, end.month, end.day, end.hour, end.minutes, end.seconds);
                break;  

            case 6:
                printf("Enter Seller ID: ");
                scanf("%d", &sellerID);
                printf("Total Revenue: %.2f\n", total_revenue_by_seller(sellerID));
                break;

            case 7:
                Transaction_with_highest_energy();
                break;

            case 8:
                sort_Transactions_By_Energy();
                printf("Transactions sorted by energy traded.\n");
                break;

            case 9:
                sortBuyerList();
                printf("Buyers sorted by total energy bought.\n");
                printBuyerList();
                break;

            case 10:
                sortSellerBuyerPairs();
                printf("Seller-buyer pairs are sorted by number of transactions in descending order.\n");
                printSortedPairs();
                break;

            case 11:
                printf("Enter Transaction ID to update: ");
                scanf("%d", &transactionID);
                printf("Enter New Energy Amount and Price per kWh: ");
                scanf("%f %f", &new_energy_kwh, &new_price_per_kwh);
                update_transaction_details(transactionID, new_energy_kwh,  new_price_per_kwh);
                break;

            case 12:
                printf("Exiting the program.\n");
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }

    return 0;
}
