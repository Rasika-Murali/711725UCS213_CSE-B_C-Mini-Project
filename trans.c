// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    char password[20];
    int pin;
    int frozen;
    double balance;
};

struct transaction
{
    unsigned int acctNum;
    double amount;
    char type[10];
    char timestamp[30];
};

unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void displayRecord(FILE *fPtr);
void SearchAccount(FILE *fPtr);
void viewTransactions(void);
void transferMoney(FILE *fPtr);
void freezeAccount(FILE *fPtr);
const char* getStatus(double balance);
int login();
int authenticateAccount(FILE *fPtr, unsigned int account);

int login()
{
    char adminUser[20];
    char adminPass[20];

    printf("Admin Username: ");
    scanf("%19s", adminUser);

    printf("Admin Password: ");
    scanf("%19s", adminPass);

    if(strcmp(adminUser, "admin") == 0 &&
       strcmp(adminPass, "1234") == 0)
    {
        printf("Admin login successful!\n");
        return 1;
    }
    return 0;
}

int authenticateAccount(FILE *fPtr, unsigned int account)
{
    struct clientData client = {0};

    char password[20];
    int pin;

    fseek(fPtr,
          (account - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum == 0)
    {
        printf("Account not found.\n");
        return 0;
    }

    printf("Enter password: ");
    scanf("%19s", password);

    printf("Enter PIN: ");
    scanf("%d", &pin);

    if(strcmp(client.password, password) == 0 &&
       client.pin == pin)
    {
        return 1;
    }

    printf("Incorrect password or PIN.\n");
    return 0;
}

int main()
{
    FILE *cfPtr;
    unsigned int choice;

    cfPtr = fopen("credit.dat", "rb+");

    if(cfPtr == NULL)
    {
        cfPtr = fopen("credit.dat", "wb+");

        if(cfPtr == NULL)
        {
            printf("File could not be opened.\n");
            exit(1);
        }

        struct clientData blank = {0};

        for(int i = 0; i < 100; i++)
        {
            fwrite(&blank, sizeof(struct clientData), 1, cfPtr);
        }
    }

    rewind(cfPtr);

    printf("\n=== BANK MANAGEMENT SYSTEM ===\n");

    if(!login())
    {
        printf("Access denied.\n");
        fclose(cfPtr);
        return 1;
    }

    while((choice = enterChoice()) != 10)
    {
        switch(choice)
        {
            case 1:
                textFile(cfPtr);
                break;

            case 2:
                updateRecord(cfPtr);
                break;

            case 3:
                newRecord(cfPtr);
                break;

            case 4:
                deleteRecord(cfPtr);
                break;

            case 5:
                displayRecord(cfPtr);
                break;

            case 6:
                SearchAccount(cfPtr);
                break;

            case 7:
                viewTransactions();
                break;

            case 8:
                transferMoney(cfPtr);
                break;

            case 9:
                freezeAccount(cfPtr);
                break;

            default:
                printf("Invalid choice.\n");
        }
    }

    fclose(cfPtr);

    return 0;
}

void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client = {0};

    writePtr = fopen("accounts.txt", "w");

    if(writePtr == NULL)
    {
        printf("File could not be opened.\n");
        return;
    }

    rewind(readPtr);

    fprintf(writePtr,
            "%-6s %-15s %-12s %-10s\n",
            "Acct",
            "Last Name",
            "First Name",
            "Balance");

    while(fread(&client,
                sizeof(struct clientData),
                1,
                readPtr) == 1)
    {
        if(client.acctNum != 0)
        {
            fprintf(writePtr,
                    "%-6d %-15s %-12s %-10.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr);

    printf("accounts.txt created successfully.\n");
}

void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;

    struct clientData client = {0};
    struct transaction t;

    printf("Enter account to update (1-100): ");
    scanf("%u", &account);

    if(account < 1 || account > 100)
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr,
          (account - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum == 0)
    {
        printf("Account not found.\n");
        return;
    }

    if(!authenticateAccount(fPtr, account))
    {
        printf("Access denied.\n");
        return;
    }

    if(client.frozen)
    {
        printf("Account is frozen.\n");
        return;
    }

    printf("Current Balance: %.2f\n", client.balance);

    printf("Enter amount (+ deposit / - withdraw): ");
    scanf("%lf", &transaction);

    client.balance += transaction;

    t.acctNum = client.acctNum;
    t.amount = transaction;

    if(transaction >= 0)
        strcpy(t.type, "DEPOSIT");
    else
        strcpy(t.type, "WITHDRAW");

    time_t now = time(NULL);

    strcpy(t.timestamp, ctime(&now));

    FILE *tPtr = fopen("transactions.dat", "ab");

    if(tPtr != NULL)
    {
        fwrite(&t, sizeof(struct transaction), 1, tPtr);
        fclose(tPtr);
    }

    fseek(fPtr,
          -(long)sizeof(struct clientData),
          SEEK_CUR);

    fwrite(&client,
           sizeof(struct clientData),
           1,
           fPtr);

    printf("Updated Balance: %.2f\n", client.balance);
    printf("Status: %s\n", getStatus(client.balance));
}

void newRecord(FILE *fPtr)
{
    struct clientData client = {0};

    unsigned int accountNum;

    printf("Enter new account number (1-100): ");
    scanf("%u", &accountNum);

    if(accountNum < 1 || accountNum > 100)
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum != 0)
    {
        printf("Account already exists.\n");
        return;
    }

    client.acctNum = accountNum;

    printf("Enter last name: ");
    scanf("%14s", client.lastName);

    printf("Enter first name: ");
    scanf("%9s", client.firstName);

    printf("Enter balance: ");
    scanf("%lf", &client.balance);

    printf("Set password: ");
    scanf("%19s", client.password);

    printf("Set 4-digit PIN: ");
    scanf("%d", &client.pin);

    client.frozen = 0;

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&client,
           sizeof(struct clientData),
           1,
           fPtr);

    printf("Account created successfully.\n");
}

void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0};

    unsigned int accountNum;

    printf("Enter account number to delete: ");
    scanf("%u", &accountNum);

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum == 0)
    {
        printf("Account does not exist.\n");
        return;
    }
    if(!authenticateAccount(fPtr, accountNum))
    {
        printf("Access denied.\n");
        return;
    }

    fseek(fPtr,
          (accountNum - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&blankClient,
           sizeof(struct clientData),
           1,
           fPtr);

    printf("Account deleted.\n");
}

void displayRecord(FILE *fPtr)
{
    struct clientData client = {0};

    rewind(fPtr);

    printf("\n%-6s %-15s %-12s %-10s %-10s\n",
           "Acct",
           "Last Name",
           "First Name",
           "Balance",
           "Status");

    while(fread(&client,
                sizeof(struct clientData),
                1,
                fPtr))
    {
        if(client.acctNum != 0)
        {
            printf("%-6d %-15s %-12s %-10.2f %-10s\n",
                   client.acctNum,
                   client.lastName,
                   client.firstName,
                   client.balance,
                   getStatus(client.balance));
        }
    }
}

void SearchAccount(FILE *fPtr)
{
    struct clientData client = {0};

    unsigned int account;

    printf("Enter account number: ");
    scanf("%u", &account);

    fseek(fPtr,
          (account - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum == 0)
    {
        printf("Account not found.\n");
        return;
    }

    printf("\nAccount Details\n");

    printf("Account Number : %d\n", client.acctNum);
    printf("Last Name      : %s\n", client.lastName);
    printf("First Name     : %s\n", client.firstName);
    printf("Balance        : %.2f\n", client.balance);
    printf("Status         : %s\n", getStatus(client.balance));

    if(client.frozen)
        printf("Frozen         : YES\n");
    else
        printf("Frozen         : NO\n");
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
        printf("No transactions found.\n");
        return;
    }

    printf("\nTransaction History\n");

    while(fread(&t,
                sizeof(struct transaction),
                1,
                tPtr))
    {
        printf("Account: %d | Type: %s | Amount: %.2f | Time: %s",
               t.acctNum,
               t.type,
               t.amount,
               t.timestamp);
    }

    fclose(tPtr);
}

void transferMoney(FILE *fPtr)
{
    unsigned int fromAcc, toAcc;
    double amount;

    struct clientData sender = {0};
    struct clientData receiver = {0};

    printf("Enter sender account: ");
    scanf("%u", &fromAcc);

    printf("Enter receiver account: ");
    scanf("%u", &toAcc);

    printf("Enter amount: ");
    scanf("%lf", &amount);

    if(fromAcc == toAcc)
    {
        printf("Cannot transfer to same account.\n");
        return;
    }

    if(amount <= 0)
    {
        printf("Invalid amount.\n");
        return;
    }

    fseek(fPtr,
          (fromAcc - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&sender,
          sizeof(struct clientData),
          1,
          fPtr);

    fseek(fPtr,
          (toAcc - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&receiver,
          sizeof(struct clientData),
          1,
          fPtr);

    if(sender.acctNum == 0 || receiver.acctNum == 0)
    {
        printf("Account not found.\n");
        return;
    }
    if(!authenticateAccount(fPtr, fromAcc))
    {
        printf("Transfer denied.\n");
        return;
    }

    if(sender.frozen || receiver.frozen)
    {
        printf("One account is frozen.\n");
        return;
    }

    if(sender.balance < amount)
    {
        printf("Insufficient balance.\n");
        return;
    }

    sender.balance -= amount;
    receiver.balance += amount;

    fseek(fPtr,
          (fromAcc - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&sender,
           sizeof(struct clientData),
           1,
           fPtr);

    fseek(fPtr,
          (toAcc - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&receiver,
           sizeof(struct clientData),
           1,
           fPtr);

    printf("Transfer successful.\n");
}

void freezeAccount(FILE *fPtr)
{
    unsigned int account;

    struct clientData client = {0};

    printf("Enter account number: ");
    scanf("%u", &account);

    fseek(fPtr,
          (account - 1) * sizeof(struct clientData),
          SEEK_SET);

    fread(&client,
          sizeof(struct clientData),
          1,
          fPtr);

    if(client.acctNum == 0)
    {
        printf("Account not found.\n");
        return;
    }
    if(!authenticateAccount(fPtr, account))
    {
        printf("Access denied.\n");
        return;
    }

    client.frozen = !client.frozen;

    fseek(fPtr,
          -(long)sizeof(struct clientData),
          SEEK_CUR);

    fwrite(&client,
           sizeof(struct clientData),
           1,
           fPtr);

    if(client.frozen)
        printf("Account frozen.\n");
    else
        printf("Account unfrozen.\n");
}

unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    printf("\nEnter your choice\n");
    printf("1  - Create accounts.txt\n");
    printf("2  - Update account\n");
    printf("3  - Add new account\n");
    printf("4  - Delete account\n");
    printf("5  - Display all accounts\n");
    printf("6  - Search account\n");
    printf("7  - View transactions\n");
    printf("8  - Transfer money\n");
    printf("9  - Freeze/unfreeze account\n");
    printf("10 - Exit\n");
    printf("? ");

    scanf("%u", &menuChoice);

    return menuChoice;
}