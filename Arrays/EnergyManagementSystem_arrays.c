#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define TIME_LEN 30
#define DB_SIZE 10000
#define MAX_SELLERS 1000
#define MAX_BUYERS 1000

struct Transaction_record
{
    int Transaction_ID;
    int Buyer_ID;
    int Seller_ID;
    float Energy_amount;
    float Price;
    char Timestamp[TIME_LEN];
    int Transaction_count_flag;//used as two different variables in two different functions
    float revenue_flag;//used to store revenue in a function
};

struct Seller_revenue
{
	float Total_revenue;
	int Seller_ID;
};

struct Buyer_energy
{
	float Total_energy;
	int Buyer_ID;
};

//to add new transactions from the operator
int new_transactions(struct Transaction_record records[], int count)
{
	int no_transactions;
	printf("Number of new transactions you need to enter: \n");
	scanf("%d",&no_transactions);
    if (count >= DB_SIZE)
    {
        printf("Transaction list is overloaded. \n");
        return count; // No update to count since DataBase is full
    }
    else
    {
    	struct Transaction_record new_transactions[no_transactions];
    	for(int i=0;i<no_transactions;i++)
    	{
        	printf("Enter transaction id: \n");
        	scanf("%d", &new_transactions[i].Transaction_ID);
        	printf("Enter buyer id: \n");
        	scanf("%d", &new_transactions[i].Buyer_ID);
        	printf("Enter Seller id: \n");
        	scanf("%d", &new_transactions[i].Seller_ID);
        	printf("Enter energy amount in kwh: \n");
        	scanf("%f", &new_transactions[i].Energy_amount);
        	printf("Enter price per kwh: \n");
        	scanf("%f", &new_transactions[i].Price);
        	printf("Enter time and date of transaction in YYYYMMDDHHMMSS: \n");
        	scanf("%s", new_transactions[i].Timestamp);
        	records[count] = new_transactions[i];
        	count++;
        	printf("Transaction added successfully\n");
        }
    }
    return count;
}

//To display all the transactions occured.
void display_transactions(struct Transaction_record records[], int count)
{
    if (count==0)
    {
        printf("No transactions available. \n");
        return;
    }
    printf("All Transactions are: \n");
    for (int i = 0; i < count; i++)
    {
        printf("Transaction id: %d, Buyer id: %d, Seller id: %d, Amount of energy in kwh: %f, Price per kwh: %f, Timestamp: %s \n",
               records[i].Transaction_ID,
               records[i].Buyer_ID,
               records[i].Seller_ID,
               records[i].Energy_amount,
               records[i].Price,
               records[i].Timestamp);
    }
}

//To display all transactions performed by a seller.
void seller_transactions(struct Transaction_record records[], int count, int seller_id)
{
    int found = 0;
    printf("Transactions for seller id %d are: \n", seller_id);
    for (int i = 0; i < count; i++)
    {
        if ((records[i].Seller_ID) == seller_id)
        {
            printf("Transaction id: %d, Buyer id: %d, Seller id: %d, Amount of energy in kwh: %f, Price per kwh: %f, Timestamp: %s \n",
                   records[i].Transaction_ID,
                   records[i].Buyer_ID,
                   records[i].Seller_ID,
                   records[i].Energy_amount,
                   records[i].Price,
                   records[i].Timestamp);
            found = 1;
        }
    }
    if (!found)
    {
        printf("No data for the given seller id %d.\n", seller_id);
    }
}

//To display all transactions performed by a buyer. 
void buyer_transactions(struct Transaction_record records[], int count, int buyer_id)
{
    int found = 0;
    printf("Transactions for buyer id %d are: \n", buyer_id);
    for (int i = 0; i < count; i++)
    {
        if ((records[i].Buyer_ID) == buyer_id)
        {
            printf("Transaction id: %d,Buyer id: %d, Seller id: %d, Amount of energy in kwh: %f, Price per kwh: %f, Timestamp: %s \n",
                   records[i].Transaction_ID,
                   records[i].Buyer_ID,
                   records[i].Seller_ID,
                   records[i].Energy_amount,
                   records[i].Price,
                   records[i].Timestamp);
            found = 1;
        }
    }
    if (!found)
    {
        printf("No data for the given buyer id %d.\n", buyer_id);
    }
}

//To check whether the transaction is in the mentioned time period or not
int within_time(char Timestamp[],char start[],char end[])
{
	return(strcmp(Timestamp,start)>=0 && strcmp(Timestamp,end)<=0);
}

//To display all transaction in the mentioned time period
void in_transaction_period(struct Transaction_record records[],int count,char start[],char end[])
{
	printf("Transactions between %s and %s: \n",start,end);
	int found=0;
	for(int i=0;i<count;i++)
	{
		if(within_time(records[i].Timestamp,start,end))
		{
			printf("Transaction id: %d, Buyer id: %d, Seller id: %d, Amount of energy in kwh: %.2f, Price per kwh: %.2f, Timestamp: %s \n",
                   		records[i].Transaction_ID,
                   		records[i].Buyer_ID,
                   		records[i].Seller_ID,
                   		records[i].Energy_amount,
                   		records[i].Price,
                   		records[i].Timestamp);
            found = 1;
        }
    }
    if (!found)
    {
    	printf("No transactions found in given time period. \n");
    }
}

//To extract month from the Timestamp string
int extract_month(char timestamp[])
{
	char month_str[3];
	strncpy(month_str,&timestamp[4],2);
	month_str[2]='\0';
	return atoi(month_str);
}

//To display the month with maximum number of transactions
void max_transactions_month(struct Transaction_record records[],int count)
{
	int month_count[12]={0};
	for (int i=0;i<count;i++)
	{
		int month=extract_month(records[i].Timestamp);
		if(month>=1 && month<=12)
		{
			month_count[month-1]++;
		}
	}
	int max_transactions=0,max_month=0;
	for(int i=0;i<12;i++)
	{
		if(month_count[i]>max_transactions)
		{
			max_transactions=month_count[i];
			max_month=i+1;// months range from 1 to 12,not 0 to 11.
		}
	}
	printf("Maximum transactions happened in %02d month with %d transactions. \n",max_month,max_transactions);
}

//To calculate the total revenue of a seller
void total_revenue(struct Transaction_record records[],int count,int seller_id)
{
	float total_revenue=0;
	int found=0;
	for(int i=0;i<count;i++)
	{
		if(records[i].Seller_ID==seller_id)
		{
			total_revenue=(total_revenue+((records[i].Energy_amount)*(records[i].Price)));
			found=1;
		}
	}
	if(found)
	{
		printf("Total revenue for seller id %d is %f \n",seller_id,total_revenue);
	}
	else
	{
		printf("No transactions found for seller id %d \n",seller_id);
	}
}

//To calculate revenues of each seller
void calculate_revenues(struct Transaction_record records[],int count,struct Seller_revenue sellers[],int *num_sellers)
{
	int seller_found,i,j;
	*num_sellers=0;
	for(i=0;i<count;i++)
	{
		seller_found=0;
		for(j=0;j<*num_sellers;j++)
		{
			if(sellers[j].Seller_ID==records[i].Seller_ID)
			{
				sellers[j].Total_revenue+=((records[i].Energy_amount)*(records[i].Price));
				seller_found=1;
			}
		}
		if(!seller_found)
		{
			sellers[*num_sellers].Seller_ID=records[i].Seller_ID;	sellers[*num_sellers].Total_revenue=((records[i].Energy_amount)*(records[i].Price));
			printf("Adding new seller id:%d with initial revenue: %f \n",sellers[*num_sellers].Seller_ID,sellers[*num_sellers].Total_revenue);
			(*num_sellers)++;
		}
	}
}
//partition function for sellers revenue
int partition(struct Seller_revenue sellers[],int low,int high)
{
	float pivot=sellers[high].Total_revenue;
	int i=low-1;
	for(int j=low;j<high;j++)
	{
		if((sellers[j].Total_revenue)>=pivot)//sorting in descending order..
		{
			i++;
			struct Seller_revenue temp=sellers[i];
			sellers[i]=sellers[j];
			sellers[j]=temp;
		}
	}
	struct Seller_revenue temp=sellers[i+1];
	sellers[i+1]=sellers[high];
	sellers[high]=temp;
	return i+1;
}

//Quicksort function to sort according to revenue
void quicksort(struct Seller_revenue sellers[],int low,int high)
{
	if(low<high)
	{
		int p=partition(sellers,low,high);
		quicksort(sellers,low,p-1);
		quicksort(sellers,p+1,high);
	}
}
//To display the sorted order of sellers according to their revenue
void display_sorted_sellers(struct Seller_revenue sellers[],int *num_sellers)
{
	printf("Sellers sorted by total revenue generated: \n");
	for(int i=0;i<*num_sellers;i++)
	{
		printf("Seller id is %d and total revenue is %f \n",
				sellers[i].Seller_ID,
				sellers[i].Total_revenue);
	}
}

//calculate energies brought by every buyer
void calculate_energies(struct Transaction_record records[],int count,struct Buyer_energy buyers[],int *num_buyers)
{
	int buyer_found,i,j;
	*num_buyers=0;
	for(i=0;i<count;i++)
	{
		buyer_found=0;
		for(j=0;j<*num_buyers;j++)
		{
			if(buyers[j].Buyer_ID==records[i].Buyer_ID)
			{
				buyers[j].Total_energy+=records[i].Energy_amount;
				buyer_found=1;
			}
		}
		if(!buyer_found)
		{
			buyers[*num_buyers].Buyer_ID=records[i].Buyer_ID;
			buyers[*num_buyers].Total_energy=records[i].Energy_amount;						   
			(*num_buyers)++;
		}
	}
}

//partition function for energy brought by the buyer
int partition_buyer(struct Buyer_energy buyers[],int low,int high)
{
	float pivot=buyers[high].Total_energy;
	int i=low-1;
	for(int j=low;j<high;j++)
	{
		if((buyers[j].Total_energy)>=pivot)//sorting in descending order..
		{
			i++;
			struct Buyer_energy temp=buyers[i];
			buyers[i]=buyers[j];
			buyers[j]=temp;
		}
	}
	struct Buyer_energy temp=buyers[i+1];
	buyers[i+1]=buyers[high];
	buyers[high]=temp;
	return i+1;
}

//Quicksort function to sort according to energy brought by buyer
void quicksort_buyer(struct Buyer_energy buyers[],int low,int high)
{
	if(low<high)
	{
		int p=partition_buyer(buyers,low,high);
		quicksort_buyer(buyers,low,p-1);
		quicksort_buyer(buyers,p+1,high);
	}
}

//To display the sorted list of buyers according to the energy brought
void display_sorted_buyers_energy(struct Buyer_energy buyers[],int *num_buyers)
{
	printf("Buyers sorted by total energy brought: \n");
	for(int i=0;i<*num_buyers;i++)
	{
		printf("Buyer id is %d and total energy is %f \n",
				buyers[i].Buyer_ID,
				buyers[i].Total_energy);
	}
}

//Find and display the transaction with the highest energy amount
//using Modified Bubble sort
void bubble_sort_and_find_max(struct Transaction_record records[],int count) 
{
    int i, j;
    int swapped=0;
    for (i = 0;(i<count-1); i++) 
    {
        swapped = 0;
        for (j = 0;j<count-i-1;j++)
        {
            if (records[j].Energy_amount > records[j + 1].Energy_amount) 
            {
                struct Transaction_record temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
                swapped = 1;
            }
        }
    }
    printf("Transaction with the highest energy amount: \n");
    printf("Transaction ID: %d, Buyer ID: %d, Seller ID: %d, Energy in KWh: %f, Price: %f, Timestamp: %s\n",
           records[count-1].Transaction_ID,
           records[count-1].Buyer_ID,
           records[count-1].Seller_ID,
           records[count-1].Energy_amount,
           records[count-1].Price,
           records[count-1].Timestamp);
    printf("All Transactions sorted by energy amount: \n");
    display_transactions(records,count);
}

//seller-buyer pair involved in maximum number of transactions
void max_transaction_pair(struct Transaction_record records[],int count)
{
	int max_transactions=0;
	int max_seller_id=0;
	int max_buyer_id=0;
	for(int i=0;i<count;i++)
	{
		if(records[i].Transaction_count_flag != -1)//skip if seller buyer pair is already done
		{
			int buyer_id=records[i].Buyer_ID;
			int seller_id=records[i].Seller_ID;
			int total_transactions=1;
			//count occurances of a seller buyer pair
			for(int j=i+1;j<count;j++)
			{
				if((records[j].Buyer_ID==buyer_id)&&(records[j].Seller_ID==seller_id))
				{
					total_transactions++;
					records[j].Transaction_count_flag= -1;
				}
			}
			//store the count in given transaction
			records[i].Transaction_count_flag=total_transactions;
			if(total_transactions>max_transactions)
			{
				max_transactions=total_transactions;
				max_seller_id=seller_id;
				max_buyer_id=buyer_id;
			}
		}
	}
	if(max_seller_id != -1 && max_buyer_id != -1)
	{
		//displaying seller-buyer pair with maximum transactions
		printf("Seller-Buyer pair with maximum number of transactions: \n");
		printf("Seller id: %d,Buyer id: %d,transactions: %d \n",
				records[0].Seller_ID,
				records[0].Buyer_ID,
				records[0].Transaction_count_flag);
	}
	else
	{
		printf("No valid transactions. \n");
	}
}

//sort all sellers-buyer pairs in decreasing order of the number of transactions
void count_sort_pairs_transaction(struct Transaction_record records[],int count)
{
	for(int i=0;i<count;i++)
	{
		if(records[i].Transaction_count_flag != -1)//skip if seller buyer pair is already done
		{
			int buyer_id=records[i].Buyer_ID;
			int seller_id=records[i].Seller_ID;
			int transaction_count=1;
			//count occurances of a seller buyer pair
			for(int j=i+1;j<count;j++)
			{
				if((records[j].Buyer_ID==buyer_id)&&(records[j].Seller_ID==seller_id))
				{
					transaction_count++;
					records[j].Transaction_count_flag= -1;
				}
			}
			//store the count in given transaction
			records[i].Transaction_count_flag=transaction_count;
		}
	}
	// Modified bubble sort for sorting in descending order
	int flag=0;
	for(int i=0;i<count-1;i++)
	{
		flag=0;
		for(int j=0;j<count-i-1;j++)
		{
			if(records[j].Transaction_count_flag < records[j+1].Transaction_count_flag)
			{
				//swap
				struct Transaction_record temp=records[j];
				records[j]=records[j+1];
				records[j+1]=temp;
			}
		}
	}
	//displaying sorted seller-buyer pairs
	printf("Sorted descending order of Seller-Buyer pairs by number of transactions: \n");
	for(int i=0;i<count;i++)
	{
		if(records[i].Transaction_count_flag != -1)
		{
			printf("Seller id: %d,Buyer id: %d,transactions: %d \n",
				records[i].Seller_ID,
				records[i].Buyer_ID,
				records[i].Transaction_count_flag);
		}
	}
}

//sort all buyer-seller pairs in decreasing order of total revenue exchanged between them 
void count_sort_pairs_revenue(struct Transaction_record records[],int count)
{
	//calculate total revenue of each seller-buyer pair
	for(int i=0;i<count;i++)
	{
		if(records[i].revenue_flag !=-1)//skip if seller buyer pair is already done
		{
			int buyer_id=records[i].Buyer_ID;
			int seller_id=records[i].Seller_ID;
			float revenue=((records[i].Energy_amount)*(records[i].Price));
			for(int j=i+1;j<count;j++)
			{
				if((records[j].Buyer_ID==buyer_id)&&(records[j].Seller_ID==seller_id))
				{
					revenue+=((records[i].Energy_amount)*(records[i].Price));
					records[j].revenue_flag= -1;
				}
			}
			//store the revenue in revenue_flag
			records[i].revenue_flag=revenue;
		}
	}
	//Modified bubble sort for sorting in descending order
	int flag=0;
	for(int i=0;i<count-1;i++)
	{
		flag=0;
		for(int j=0;j<count-i-1;j++)
		{
			if(records[j].revenue_flag < records[j+1].revenue_flag)
			{
				//swap
				struct Transaction_record temp=records[j];
				records[j]=records[j+1];
				records[j+1]=temp;
			}
		}
	}
	//displaying sorted seller-buyer pairs by revenue
	printf("Sorted descending order of Seller-Buyer pairs by revenue: \n");
	for(int i=0;i<count;i++)
	{
		if(records[i].revenue_flag != -1)
		{
			printf("Seller id: %d,Buyer id: %d,revenue: %f \n",
				records[i].Seller_ID,
				records[i].Buyer_ID,
				records[i].revenue_flag);
		}
	}
}

//main function to execute each and every function
int main()
{
	//All variables required for execution
	int num_records;
	int size=0,s=0;
	int *num_sellers=&size;
	int *num_buyers=&s;
	int choice,sellerid,buyerid;
	char start[TIME_LEN],end[TIME_LEN];
	//structures written 
	struct Transaction_record records[DB_SIZE];
	struct Seller_revenue sellers[MAX_SELLERS];
	struct Buyer_energy buyers[MAX_BUYERS];
	printf("Total number of Records: \n");
	scanf("%d",&num_records); 
	//To read total number of transactions from user...
	for (int i=0;i<num_records;i++)
	{
		//To read transaction id,seller id,buyer id,amount of energy,cost,date and time of transaction occured from user for every transactions
		printf("Transaction id is : \n");
		scanf("%d",&records[i].Transaction_ID);
		printf("Buyer id is : \n");
		scanf("%d",&records[i].Buyer_ID);
		printf("Seller id is : \n");
		scanf("%d",&records[i].Seller_ID);
		printf("Enter amount of energy in kwh: \n");
		scanf("%f",&records[i].Energy_amount);
		printf("Enter price per kwh: \n");
		scanf("%f",&records[i].Price);
		printf("Enter time and date in format YYYYMMDDHHMMSS: \n");
		scanf("%s",records[i].Timestamp);
	}
	int transaction_count=num_records;
	while(choice!=14)
	{
		printf("Energy transaction management system \n");
		printf("1.Add new Transactions \n");
		printf("2.Display all transactions \n");
		printf("3.Display all transactions of a seller \n");
		printf("4.Display all transactions of a buyer \n");
		printf("5.Display all transactions within timeperiod \n");
		printf("6.Display month with maximum transactions \n");
		printf("7.Display total revenue of a seller \n");
		printf("8.Sort sellers by total revenue \n");
		printf("9.Sort buyers by total energy brought \n");
		printf("10.Sort and display transaction with highest amount of energy \n");
		printf("11.Display seller-buyer pair with maximum number of transactions \n");
		printf("12.Sorted buyer-seller pairs by transactions \n");//in descending order.. 
		printf("13.Sorted buyer-seller pairs by revenue \n");//in descending order.. 
		printf("14.Exit \n");
		printf("Select any one option: \n");//To execute any one from above
		scanf("%d",&choice);
		//Using if,else to make your choice work...
		if(choice==1)
		{
		 
			transaction_count=new_transactions(records,transaction_count);
		}
		else if(choice==2)
		{
			display_transactions(records,transaction_count);
		}
		else if(choice==3)
		{
			printf("Enter the seller id: \n");
			scanf("%d",&sellerid);
			seller_transactions(records,transaction_count,sellerid);
		}
		else if(choice==4)
		{
			printf("Enter the buyer id: \n");
			scanf("%d",&buyerid);
			buyer_transactions(records,transaction_count,buyerid);
		}
		else if(choice==5)
		{
			printf("Enter the start time in YYYYMMDDHHMMSS: \n");
			scanf("%s",start);
			printf("Enter the end time in YYYYMMDDHHMMSS: \n");
			scanf("%s",end);
			in_transaction_period(records,transaction_count,start,end);
		}
		else if(choice==6)
		{
			max_transactions_month(records,transaction_count);
		}
		else if(choice==7)
		{
			printf("Enter the seller id: \n");
			scanf("%d",&sellerid);
			total_revenue(records,transaction_count,sellerid);
		}
		else if(choice==8)
		{
			//calculate total revenue for each seller
			calculate_revenues(records,transaction_count,sellers,num_sellers);
			//Sort sellers by revenue using quicksort
			quicksort(sellers,0,(*num_sellers)-1);
			//Display sorted sellers
			display_sorted_sellers(sellers,num_sellers);
		}
		else if(choice==9)
		{
			//calculate total energy for each buyer
			calculate_energies(records,transaction_count,buyers,num_buyers);
			//sort buyers by energy brought using quick sort
			quicksort_buyer(buyers,0,(*num_buyers)-1);
			//Display sorted buyers according to energy
			display_sorted_buyers_energy(buyers,num_buyers);
		}
		else if(choice==10)
	    {
			bubble_sort_and_find_max(records,transaction_count);
		}
		else if(choice==11)
		{
			max_transaction_pair(records,transaction_count);
		}
		else if(choice==12)
		{
			count_sort_pairs_transaction(records,transaction_count);
		}
		else if(choice==13)
		{
			count_sort_pairs_revenue(records,transaction_count);
		}
		else if(choice==14)
		{
			printf("Exiting the system. \n");
			return 0;
		}
		else
		{
			//if select any integer other than integers from 0 to 14
			printf("Invalid choice \n");
		}
	}
	return 0;						
}

