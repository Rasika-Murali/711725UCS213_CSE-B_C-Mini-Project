// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData
struct transaction
{
    unsigned int acctNum;
    double amount;
    char type[10];
};
// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void displayRecord(FILE *fptr);
void SearchAccount(FILE *fPtr);
void viewTransactions(void);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; exits if file cannot be opened
    cfPtr = fopen("credit.dat", "wb+"); // overwrite and reset file

if (cfPtr == NULL)
{
    printf("%s: File could not be opened.\n", argv[0]);
    exit(-1);
}

// initialize 100 blank records
struct clientData blank = {0, "", "", 0.0};

for (int i = 0; i < 100; i++)
{
    fwrite(&blank, sizeof(struct clientData), 1, cfPtr);
}

// check if file is empty
fseek(cfPtr, 0, SEEK_END);
long size = ftell(cfPtr);

if(size == 0)
{
    rewind(cfPtr);
    for(int i = 0; i < 100; i++)
    {
        fwrite(&blank, sizeof(struct clientData), 1, cfPtr);
    }
}
    // enable user to specify action
    while ((choice = enterChoice()) != 8)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        // display all records
        case 5:
            displayRecord(cfPtr);
            break;
        // search for a specific account
        case 6:
            SearchAccount(cfPtr);
            break;
        case 7:
        viewTransactions();
        break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
            } // end if
        }     // end while

        fclose(writePtr); // fclose closes the file
    }                     // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    // create clientData with no information
    struct clientData client = {0, "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);
    //adding validity
    if (account < 1 || account > 100)
{
    printf("Invalid account number.\n");
    return;
}
    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);
        printf("Current Status : %s\n\n", getStatus(client.balance));
        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction; // update record balance

        struct transaction t;

        t.acctNum = client.acctNum;
        t.amount = transaction;

        if(transaction >= 0)
            strcpy(t.type, "DEPOSIT");
        else
            strcpy(t.type, "WITHDRAW");

        FILE *tPtr = fopen("transactions.dat", "ab");

        if(tPtr != NULL)
        {
            fwrite(&t, sizeof(struct transaction), 1, tPtr);
            fclose(tPtr);
        }

        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
        printf("Status         : %s\n", getStatus(client.balance));

if(client.balance < 0)
{
    printf("⚠ Warning: Account is overdrawn!\n");
}
        // move file pointer to correct record in file
        // move back by 1 record length
        fseek(fPtr, - (long) sizeof(struct clientData), SEEK_CUR);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0}; // blank client
    unsigned int accountNum;                        // account number

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);
    //adding validity
    if (accountNum < 1 || accountNum > 100)
{
    printf("Invalid account number.\n");
    return;
}
    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        // move file pointer to correct record in file
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; // account number

    // obtain number of account to create
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);
    //adding validity
    if (accountNum < 1 || accountNum > 100)
{
    printf("Invalid account number.\n");
    return;
}
    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);

        client.acctNum = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function newRecord
// display all records
void displayRecord(FILE *fPtr)
{
    struct clientData client = {0,"","",0.0};

    rewind(fPtr); // move to beginning of file

   printf("%-6s %-15s %-12s %10s\n","Acct","Last Name","First Name","Balance");
    while(fread(&client,sizeof(struct clientData),1,fPtr))
    {
        if(client.acctNum != 0)
        {
            printf("%-6d %-15s %-12s %10.2f\n",
                   client.acctNum,
                   client.lastName,
                   client.firstName,
                   client.balance);
        }
    }
}
// search for a specific account
void SearchAccount(FILE *fPtr)
{
    struct clientData client = {0,"","",0.0};
    unsigned int account;

    printf("Enter account number to search (1-100): ");
    scanf("%u",&account);

    if(account < 1 || account > 100)
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr,(account-1)*sizeof(struct clientData),SEEK_SET);
    fread(&client,sizeof(struct clientData),1,fPtr);

    if(client.acctNum == 0)
    {
        printf("Account not found.\n");
    }
    else
    {
        printf("\nAccount Details\n");
        printf("Account Number : %d\n",client.acctNum);
        printf("Last Name      : %s\n",client.lastName);
        printf("First Name     : %s\n",client.firstName);
        printf("Balance        : %.2f\n",client.balance);
        printf("Status         : %s\n", getStatus(client.balance));
    }
}
const char* getStatus(double balance)
{
    if(balance < 0)
        return "OVERDRAWN";
    else if(balance < 1000)
        return "LOW";
    else if(balance <= 10000)
        return "NORMAL";
    else
        return "PREMIUM";
}
void viewTransactions(void)
{
    FILE *tPtr = fopen("transactions.dat", "rb");
    struct transaction t;

    if(tPtr == NULL)
    {
        printf("No transaction history found.\n");
        return;
    }

    printf("\nTransaction History:\n");

    while(fread(&t, sizeof(struct transaction), 1, tPtr))
    {
        printf("Account: %d | Type: %s | Amount: %.2f\n",
               t.acctNum, t.type, t.amount);
    }

    fclose(tPtr);
}
// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - display all accounts\n"
                 "6 - search for an account\n"
                 "7 - view transaction history\n"
                 "8 - end program\n? ");
     const char* getStatus(double balance)
{
    scanf("%u", &menuChoice); // receive choice from user
    return menuChoice;
} // end function enterChoice